
#include "IntercoDataMps.hpp"
#include "xprs_driver.hpp"
#include <fstream>
#include <sstream>

std::string get_name(std::string const & path) {
	int last_sep(0);
	for (int i(0); i < path.size(); ++i) {
		if (path[i] == '\\') {
			last_sep = i;
		}
	}
	std::string name(path.substr(last_sep + 1));
	name = name.substr(0, name.size() - 4);
	return name;
}

int main(int argc, char** argv) {
	//if (argc < 2) {
	//	std::cout << "usage: <exe> <Xpansion study output>" << std::endl;
	//	std::exit(0);
	//}
	//std::string const root(argv[1]);
	//std::string const root("D:\\STAGES\\Enzo\\test_case_expansion\\output\\20190123-1546eco");
	//std::string const root("D:\\STAGES\\Enzo\\test_case_expansion\\output\\20190123-1635eco-2");
	std::string const root("D:\\boucle\\1- Base case\\output\\20190214-1427eco");

	//std::string const root("D:\\boucle\\2- Solar PV and wind only - loss\\output\\20190124-1240eco");
	//std::string const root("D:\\boucle\\test_case_expansion\\output\\20190125-1046eco");

	std::string const candidates_file_name(root + PATH_SEPARATOR + ".." + PATH_SEPARATOR + ".." + PATH_SEPARATOR + "user" + PATH_SEPARATOR + "expansion" + PATH_SEPARATOR + "candidates.ini");

	std::string const mps_file_name(root + PATH_SEPARATOR + "mps.txt");
	std::string const area_file_name(root + PATH_SEPARATOR + "area.txt");
	std::string const interco_file_name(root + PATH_SEPARATOR + "interco.txt");
	{
		Candidates::MPS_LIST.clear();
		std::ifstream file(mps_file_name.c_str());
		if (!file.good()) {
			std::cout << "unable to open " << mps_file_name << std::endl;
			std::exit(0);
		}
		std::string line;
		while (std::getline(file, line)) {
			std::stringstream buffer(line);
			if (!line.empty() && line.front() != '#') {
				std::vector<std::string> data;
				std::string str;
				while (buffer >> str) {
					data.push_back(str);
				}
				Candidates::MPS_LIST.push_back(data);
			}
		}
	} 
	{
		Candidates::intercos_map.clear();
		std::ifstream file(interco_file_name.c_str());
		if (!file.good()) {
			std::cout << "unable to open " << interco_file_name << std::endl;
			std::exit(0);
		}
		std::string line;
		while (std::getline(file, line)) {
			std::stringstream buffer(line);
			if (!line.empty() && line.front() != '#') {
				int interco;
				int pays_or;
				int pays_ex;

				buffer >> interco;
				buffer >> pays_or;
				buffer >> pays_ex;

				Candidates::intercos_map.push_back({ interco, pays_or, pays_ex });
			}
		}
	}
	{
		Candidates::area_names.clear();
		std::ifstream file(area_file_name.c_str());
		if (!file.good()) {
			std::cout << "unable to open " << area_file_name << std::endl;
			std::exit(0);
		}
		std::string line;
		while (std::getline(file, line)) {
			if (!line.empty() && line.front() != '#') {
				Candidates::area_names.push_back(line);				
			}
		}
		for (auto const & kvp : Candidates::intercos_map) {
			std::string const & pays_or(Candidates::area_names[std::get<1>(kvp)]);
			std::string const & pays_ex(Candidates::area_names[std::get<2>(kvp)]);
			Candidates::or_ex_id[{pays_or, pays_ex}] = std::get<0>(kvp);
		}
	}

	//std::vector<std::vector<std::string>> candidates_map;
	//{
	//	std::ifstream file(candidates_file_name.c_str());
	//	if (!file.good()) {
	//		std::cout << "unable to open " << candidates_file_name << std::endl;
	//		std::exit(0);
	//	}
	//	std::string line;
	//	while (std::getline(file, line)) {
	//		std::stringstream buffer(line);
	//		if (!line.empty() && line.front() != '#') {
	//			std::vector<std::string> data;
	//			std::string str;
	//			while (buffer >> str) {
	//				data.push_back(str);
	//			}
	//			candidates_map.push_back(data);
	//		}
	//	}
	//}
	Candidates candidates(candidates_file_name);
	//std::string const root = ".\\";
	//
	//std::string const area_path = root + PATH_SEPARATOR + ".." + PATH_SEPARATOR + ".." + PATH_SEPARATOR + "input" + PATH_SEPARATOR + "areas" + PATH_SEPARATOR + "list.txt";
	//std::string const study_path = root + PATH_SEPARATOR + ".." + PATH_SEPARATOR + "..";

	std::map< std::pair<std::string, std::string>, int> couplings;
	XPRSinit("");
	int n_mps(0);
	for (auto const & mps : Candidates::MPS_LIST) {
		candidates.treat(root, mps, couplings);
		n_mps += 1;
		//if (n_mps > 4) {
			//break;
		//}
	}
	// master generation
	XPRSprob master;
	XPRScreateprob(&master);
	XPRSsetcbmessage(master, optimizermsg, NULL);
	XPRSsetintcontrol(master, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_FULL_OUTPUT);
	XPRSloadlp(master, "master", 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	int status;

	int nintero = candidates.size();
	std::vector<int> mstart(nintero + 1, 0);
	std::vector<double> obj_interco(nintero, 0);
	std::vector<double> lb_interco(nintero, +XPRS_MINUSINFINITY);
	std::vector<double> ub_interco(nintero, +XPRS_PLUSINFINITY);
	std::vector<std::string> interco_names(nintero);
	int i(0);
	for (auto const & interco : candidates) {
		obj_interco[i] = interco.second.obj();
		lb_interco[i] = interco.second.lb();
		ub_interco[i] = interco.second.ub();
		int interco_id = Candidates::or_ex_id.find({ interco.second.str("linkor") , interco.second.str("linkex") })->second;
		//std::cout << i << " : " << interco.second.paysor() << " - " << interco.second.paysex() << std::endl;
		std::stringstream buffer;
		//buffer << "INVEST_INTERCO_" << interco.second.str("linkor") << "_" << interco.second.str("linkex");
		buffer << "INVEST_INTERCO_" << interco_id;
		interco_names[i] = buffer.str();
		++i;
	}
	status = XPRSaddcols(master, nintero, 0, obj_interco.data(), mstart.data(), NULL, NULL, lb_interco.data(), ub_interco.data());
	if (status) {
		std::cout << "master XPRSaddcols error" << std::endl;
		std::exit(0);
	}
	i = 0;
	for (auto const & name : interco_names) {
		status = XPRSaddnames(master, 2, interco_names[i].c_str(), i, i);
		if (status) {
			std::cout << "master XPRSaddname error" << std::endl;
			std::exit(0);
		}
		++i;
	}
	std::string const lp_name = "master";	
	XPRSwriteprob(master, (root + PATH_SEPARATOR + "lp" + PATH_SEPARATOR + lp_name + ".lp").c_str(), "l");
	XPRSwriteprob(master, (root + PATH_SEPARATOR + "lp" + PATH_SEPARATOR + lp_name + ".mps").c_str(), "");
	XPRSdestroyprob(master);
	XPRSfree();
	std::map<std::string, std::map<std::string, int> > output;
	for (auto const & coupling : couplings) {
		output[get_name(coupling.first.second)][coupling.first.first] = coupling.second;
	}
	i = 0;
	for (auto const & name : interco_names) {
		output["master"][name] = i;
		++i;
	}
	std::ofstream coupling_file((root + PATH_SEPARATOR + "lp" + PATH_SEPARATOR + "structure.txt").c_str());
	for (auto const & mps : output) {
		for (auto const & pmax : mps.second) {
			coupling_file << std::setw(50) << mps.first;
			coupling_file << std::setw(50) << pmax.first;
			coupling_file << std::setw(10) << pmax.second;
			coupling_file << std::endl;
		}	
	}
	coupling_file.close();
	//structure_file.close();
	return 0;
}

