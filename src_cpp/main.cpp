/**
 * \file main.cpp
 * \brief POC Antares Xpansion V2
 * \author {Manuel Ruiz; Luc Di Gallo}
 * \version 0.1
 * \date 07 aout 2019
 *
 * POC Antares Xpansion V2: create inputs for the solver version 2
 *
 */
#include "IntercoDataMps.hpp"
#include "xprs_driver.hpp"
#include <fstream>
#include <sstream>



 /**
  * \fn string get_name(string const & path)
  * \brief Get the correct path from a string
  *
  * \param path String corresponding to a path with mistakes
  * \return The correct path
  */
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




/**
 * \fn void initializedCandidates(string rootPath, Candidates & candidates)
 * \brief Initialize the candidates structure with input data located in the directory given in argument
 *
 * \param rootPath String corresponding to the path where are located input data
 * \param candidates Structure which is initialized
 * \return void
 */
void initializedCandidates(std::string rootPath, Candidates & candidates) {
	std::string line;

	// Get all mandatory path
	std::string const candidates_file_name(rootPath + PATH_SEPARATOR + ".." + PATH_SEPARATOR + ".." + PATH_SEPARATOR + "user" + PATH_SEPARATOR + "expansion" + PATH_SEPARATOR + "candidates.ini");
	std::string const mps_file_name(rootPath + PATH_SEPARATOR + "mps.txt");
	std::string const area_file_name(rootPath + PATH_SEPARATOR + "area.txt");
	std::string const interco_file_name(rootPath + PATH_SEPARATOR + "interco.txt");

	// Initialize the list of MPS files
	Candidates::MPS_LIST.clear();
	std::ifstream mps_filestream(mps_file_name.c_str());
	if (!mps_filestream.good()) {
		std::cout << "unable to open " << mps_file_name << std::endl;
		std::exit(0);
	}
	while (std::getline(mps_filestream, line)) {
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

	// Initialize the list of interconnexion
	Candidates::intercos_map.clear();
	std::ifstream interco_filestream(interco_file_name.c_str());
	if (!interco_filestream.good()) {
		std::cout << "unable to open " << interco_file_name << std::endl;
		std::exit(0);
	}
	while (std::getline(interco_filestream, line)) {
		std::stringstream buffer(line);
		if (!line.empty() && line.front() != '#') {
			int interco; /*!< Number of the interconnection */
			int pays_or; /*!< Number of the origin country */
			int pays_ex; /*!< Number of the destination country */

			buffer >> interco;
			buffer >> pays_or;
			buffer >> pays_ex;

			Candidates::intercos_map.push_back(std::make_tuple(interco, pays_or, pays_ex));
		}
	}

	// Initialize the list of area
	Candidates::area_names.clear();
	std::ifstream area_filestream(area_file_name.c_str());
	if (!area_filestream.good()) {
		std::cout << "unable to open " << area_file_name << std::endl;
		std::exit(0);
	}
	while (std::getline(area_filestream, line)) {
		if (!line.empty() && line.front() != '#') {
			Candidates::area_names.push_back(line);
		}
	}
	for (auto const & kvp : Candidates::intercos_map) {
		std::string const & pays_or(Candidates::area_names[std::get<1>(kvp)]);
		std::string const & pays_ex(Candidates::area_names[std::get<2>(kvp)]);
		Candidates::or_ex_id[std::make_tuple(pays_or, pays_ex)] = std::get<0>(kvp);
	}

	candidates.getCandidatesFromFile(candidates_file_name);
}




/**
 * \fn void masterGeneration()
 * \brief Generate the master ob the optimization problem
 *
 * \param rootPath String corresponding to the path where are located input data
 * \param candidates Structure which contains the list of candidates
 * \param couplings map pairs and integer which give the correspondence between optim variable and antares variable
 * \return void
 */
void masterGeneration(std::string rootPath, Candidates candidates, std::map< std::pair<std::string, std::string>, int> couplings, std::string const &master_formulation) {
	XPRSprob master;
	XPRScreateprob(&master);
	XPRSsetcbmessage(master, optimizermsg, NULL);
	XPRSsetintcontrol(master, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_FULL_OUTPUT);
	XPRSloadlp(master, "master", 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	int status;

	int ninterco = candidates.size();
	std::vector<int> mstart(ninterco + 1, 0);
	std::vector<double> obj_interco(ninterco, 0);
	std::vector<double> lb_interco(ninterco, +XPRS_MINUSINFINITY);
	std::vector<double> ub_interco(ninterco, +XPRS_PLUSINFINITY);
	std::vector<std::string> interco_names(ninterco);
	
	int i(0);
	std::vector<std::string> pallier_names;
	std::vector<int> pallier;
	std::vector<int> pallier_i;
	std::vector<double> unit_size;
	std::vector<double> max_unit;

	for (auto const & interco : candidates) {
		obj_interco[i] = interco.second.obj();
		lb_interco[i] = interco.second.lb();
		ub_interco[i] = interco.second.ub();
		int interco_id = Candidates::or_ex_id.find(std::make_tuple(interco.second.str("linkor"), interco.second.str("linkex")))->second;
		std::stringstream buffer;
		//buffer << "INVEST_INTERCO_" << interco_id;
		buffer << Candidates::id_name.find(interco_id)->second;
		interco_names[i] = buffer.str();
		
		if (interco.second.is_integer()) {
			pallier.push_back(i);
			int new_id = ninterco + pallier_i.size();
			pallier_i.push_back(new_id);
			unit_size.push_back(interco.second.unit_size());
			max_unit.push_back(interco.second.max_unit());
			std::cout << interco.second.max_unit() << std::endl;
		}
		++i;
	}
	status = XPRSaddcols(master, ninterco, 0, obj_interco.data(), mstart.data(), NULL, NULL, lb_interco.data(), ub_interco.data());
	if (status) {
		std::cout << "master XPRSaddcols error" << std::endl;
		std::exit(0);
	}
	// integer constraints
	int n_integer = pallier.size();
	if(n_integer>0 && master_formulation=="integer"){
		std::vector<double> zeros(n_integer, 0);
		std::vector<char> integer_type(n_integer, 'I');
		XPRSaddcols(master, n_integer, 0, zeros.data(), NULL, NULL, NULL, zeros.data(), max_unit.data());
		XPRSchgcoltype(master, n_integer, pallier_i.data(), integer_type.data());
		std::vector<double> dmatval;
		std::vector<int> colind;
		std::vector<char> rowtype;
		std::vector<double> rhs;
		std::vector<int> rstart;
		for (i = 0; i < n_integer; ++i) {
			// pMax  - n max_unit = 0 <= 0
			rstart.push_back(dmatval.size());
			rhs.push_back(0);
			rowtype.push_back('E');
			colind.push_back(pallier[i]);
			dmatval.push_back(1);
			colind.push_back(pallier_i[i]);
			dmatval.push_back(-unit_size[i]);
		}
		int n_row_interco(rowtype.size());
		int n_coeff_interco(dmatval.size());
		rstart.push_back(dmatval.size());
		status = XPRSaddrows(master, n_row_interco, n_coeff_interco, rowtype.data(), rhs.data(), NULL, rstart.data(), colind.data(), dmatval.data());
		if (status) {
			std::cout << "XPRSaddrows error l." << __LINE__ << std::endl;
			std::exit(0);
		}

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
	XPRSwriteprob(master, (rootPath + PATH_SEPARATOR + "lp" + PATH_SEPARATOR + lp_name + ".lp").c_str(), "l");
	XPRSwriteprob(master, (rootPath + PATH_SEPARATOR + "lp" + PATH_SEPARATOR + lp_name + ".mps").c_str(), "");
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
	std::ofstream coupling_file((rootPath + PATH_SEPARATOR + "lp" + PATH_SEPARATOR + "structure.txt").c_str());
	for (auto const & mps : output) {
		for (auto const & pmax : mps.second) {
			coupling_file << std::setw(50) << mps.first;
			coupling_file << std::setw(50) << pmax.first;
			coupling_file << std::setw(10) << pmax.second;
			coupling_file << std::endl;
		}
	}
	coupling_file.close();
}



/**
 * \fn int main (void)
 * \brief Main program
 *
 * \param  argc An integer argument count of the command line arguments
 * \param  argv Path to input data which is the 1st argument vector of the command line argument.
 * \return an integer 0 corresponding to exit success
 */
int main(int argc, char** argv) {
	// Test if there are enough arguments
	if (argc < 3) {
		std::cout << "usage: <exe> <Xpansion study output> <relaxed or integer>" << std::endl;
		std::exit(0);
	}

	// Instantiation of candidates
	std::string const root(argv[1]);
	Candidates candidates;
	initializedCandidates(root, candidates);
	std::string const master_formulation(argv[2]);

	std::map< std::pair<std::string, std::string>, int> couplings;
	XPRSinit("");
	candidates.treatloop(root, couplings);
	masterGeneration(root, candidates, couplings, master_formulation);

	return 0;
}

