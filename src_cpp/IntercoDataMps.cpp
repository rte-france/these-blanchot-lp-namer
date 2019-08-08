#include "IntercoDataMps.hpp"
#include "xprs_driver.hpp"


std::vector<std::vector<std::string> > Candidates::MPS_LIST = {
};

std::vector<std::tuple<int, int, int> > Candidates::intercos_map = {
//#include "interco.txt"
};

//std::vector<std::vector<std::string>> Candidates::candidates_map = {
//#include "candidates.txt"
//};
std::map<std::tuple<std::string, std::string>, int> Candidates::or_ex_id = std::map<std::tuple<std::string, std::string>, int>();
std::set<std::string> Candidates::str_fields = std::set<std::string>({
	"name",
	"investment_type",
	"link",
	"linkor",
	"linkex",
	"link-profile"
	});

std::set<std::string> Candidates::dbl_fields = std::set<std::string>({
	"annual-cost-per-mw",
	"max-investment",
	"unit-size",
	"max-units"
	});

std::vector<std::string> Candidates::area_names = {
};


double Candidate::profile(size_t i, std::string const & study_path) {
	if (_profile.empty()) {
		if (has("link-profile")) {
			std::string const file_name = str("link-profile");
			std::string const profile_path(study_path + PATH_SEPARATOR + "user" + PATH_SEPARATOR + "expansion" + PATH_SEPARATOR + "capa" + PATH_SEPARATOR + file_name);
			std::ifstream infile(profile_path.c_str());
			if (!infile.good()) {
				std::cout << "unable to open : " << profile_path << std::endl;
			}
			_profile.reserve(8760);
			double value;
			while (infile >> value) {
				_profile.push_back(value);
			}
			infile.close();
		}
	}
	if (!_profile.empty()) {
		return _profile[i];
	}
	else {
		return 1.0;
	}
}

double Candidate::obj()const {
	return _dbl.find("annual-cost-per-mw")->second;
}
double Candidate::lb() const {
	return 0;
}
double Candidate::ub() const {
	auto it(_dbl.find("max-investment"));
	if (it != _dbl.end()) {
		return it->second;
	}
	else {
		return _dbl.find("unit-size")->second*_dbl.find("max-units")->second;
	}
}
//Candidates::Candidates(std::vector<std::vector<std::string>>  const & datas) {
//	for (auto const & data : datas) {
//		std::string const & id(data[0]);
//		std::string const & key(data[1]);
//		std::stringstream value(data[2]);
//		if (Candidates::str_fields.find(key) != Candidates::str_fields.end()) {
//			(*this)[id]._str[key] = value.str();
//		}
//		else {
//			value >> (*this)[id]._dbl[key];
//		}
//	}
//	//area_names.assign(size(), )
//}
#include "INIReader.h"
Candidates::Candidates(std::string  const & ini_file) {
	INIReader reader(ini_file.c_str());
	std::stringstream ss;
	std::set<std::string> sections = reader.Sections();
	for (auto const & candidate : sections) {
		std::cout << "-------------------------------------------" << std::endl;
		for (auto const & str : Candidates::str_fields) {
			std::string val = reader.Get(candidate, str, "NA");
			if (val != "NA") {
				std::cout << candidate << " : " << str << " = " << val << std::endl;
				if (str == "link") {
					size_t i = val.find(" - ");
					if (i != std::string::npos) {
						std::string s1 = val.substr(0, i);
						std::string s2 = val.substr(i+3, val.size());
						std::cout << s1 << " and " << s2 << std::endl;
						(*this)[candidate]._str["linkor"] = s1;
						(*this)[candidate]._str["linkex"] = s2;
					}
				}
				else {
					(*this)[candidate]._str[str] = val;
				}
			}
		}		
		for (auto const & str : Candidates::dbl_fields) {
			std::string val = reader.Get(candidate, str, "NA");
			if (val != "NA") {
				//std::cout << candidate << " : " << str << " = " << val << std::endl;
				std::stringstream buffer(val);
				double d_val(0);
				buffer >> d_val;
				(*this)[candidate]._dbl[str] = d_val;
			}
		}
	}
	std::cout << "-------------------------------------------" << std::endl;
	//for (auto const & data : datas) {
	//	std::string const & id(data[0]);
	//	std::string const & key(data[1]);
	//	std::stringstream value(data[2]);
	//	if (Candidates::str_fields.find(key) != Candidates::str_fields.end()) {
	//		(*this)[id]._str[key] = value.str();
	//	}
	//	else {
	//		value >> (*this)[id]._dbl[key];
	//	}
	//}
	////area_names.assign(size(), )
}


void Candidates::treat(std::string const & root, std::vector<std::string> const & mps, std::map< std::pair<std::string, std::string>, int> & couplings) {
	std::map<std::pair<std::string, std::string>, Candidate *> key_paysor_paysex;
	std::string const study_path = root + PATH_SEPARATOR + ".." + PATH_SEPARATOR + "..";
	for (auto & kvp : *this) {
		Candidate const & interco(kvp.second);

		std::string const & paysor(interco.str("linkor"));
		std::string const & paysex(interco.str("linkex"));

		//std::cout << paysor<<" --- "<<paysex<< std::endl;
		if (key_paysor_paysex.find({ paysor, paysex }) != key_paysor_paysex.end()) {
			std::cout << "duplicate interco : " << paysor << " - " << paysex << std::endl;
			std::exit(0);
		}
		if (key_paysor_paysex.find({ paysex, paysor }) != key_paysor_paysex.end()) {
			std::cout << "reverse interco already defined : " << paysex << " - " << paysor << std::endl;
			std::exit(0);
		}
		key_paysor_paysex[{paysor, paysex }] = &kvp.second;

	}
	//std::exit(0);
	XPRSprob xpr = NULL;
	//std::cout << std::setw(45) << mps[0];
	//std::cout << std::setw(45) << mps[1];
	//std::cout << std::setw(45) << mps[2] << std::endl;

	std::string const mps_name(root + PATH_SEPARATOR + mps[0]);
	std::string const var_name(root + PATH_SEPARATOR + mps[1]);
	std::string const ctr_name(root + PATH_SEPARATOR + mps[2]);
	std::string line;

	std::list<std::string> var(0);
	std::list<std::string> ctr(0);
	size_t vsize(0);
	size_t csize(0);
	std::map<int, std::vector<int> > interco_data;
	std::map<std::vector<int>, int> interco_id;
	{
		std::ifstream file(ctr_name.c_str());
		if (!file.good()) {
			std::cout << "unable to open " << ctr_name << std::endl;
			std::exit(0);
		}
		while (std::getline(file, line)) {
			std::istringstream buffer(line);
			std::ostringstream name;
			std::string part;
			bool is_first(true);
			while (std::getline(buffer, part)) {
				if (!is_first) {
					name << part << "_";
				}
				else {
					is_first = false;
				}
			}
			ctr.push_back(name.str());
			csize += name.str().size() + 1;
		}
		file.close();
	}
	{
		std::ifstream file(var_name.c_str());
		if (!file.good()) {
			std::cout << "unable to open " << var_name << std::endl;
			std::exit(0);
		}
		while (std::getline(file, line)) {
			std::ostringstream name;
			{
				std::istringstream buffer(line);
				std::string part;
				bool is_first(true);
				while (buffer >> part) {
					if (!is_first) {
						name << part << "_";
					}
					else {
						is_first = false;
					}
				}
			}
			if (contains(name.str(), "ValeurDeNTC")) {
				std::istringstream buffer(line);
				int id;
				int pays;
				int interco;
				int pdt;
				std::string trash;
				buffer >> id;
				buffer >> trash;
				buffer >> pays;
				buffer >> interco;
				buffer >> pdt;

				std::string const & paysor(Candidates::area_names[std::get<1>(intercos_map[interco])]);
				std::string const & paysex(Candidates::area_names[std::get<2>(intercos_map[interco])]);
				if (key_paysor_paysex.find({ paysor, paysex }) != key_paysor_paysex.end()) {
					interco_data[id] = { pays, interco, pdt };
					if (interco_id.find({ pays, interco }) == interco_id.end()) {
						interco_id[{pays, interco }] = interco_id.size();
					}
				}
				else {
					//std::cout << "Not candidate : " << paysor << " - " << paysex << std::endl;
				}
			}
			var.push_back(name.str());
			vsize += name.str().size() + 1;
		}
		file.close();
	}
	XPRScreateprob(&xpr);
	XPRSsetintcontrol(xpr, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_NO_OUTPUT);
	//XPRSsetintcontrol(xpr, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_FULL_OUTPUT);
	XPRSsetcbmessage(xpr, optimizermsg, NULL);
	XPRSreadprob(xpr, mps_name.c_str(), "");

	int ncols;
	int nrows;
	XPRSgetintattrib(xpr, XPRS_COLS, &ncols);
	XPRSgetintattrib(xpr, XPRS_ROWS, &nrows);

	int nintero_pdt = interco_data.size();

	if (nrows != ctr.size() + 1) {
		std::cout << "WRONG NUMBER OF CTR NAMES, XPRESS = " << nrows << ", " << ctr.size() << " given" << std::endl;
	}
	if (ncols != var.size()) {
		std::cout << "WRONG NUMBER OF VAR NANES, XPRESS = " << ncols << ", " << var.size() << " given" << std::endl;
	}
	std::vector<char> vnames(vsize, '\0');
	int iname(0);
	for (auto const & name : var) {
		for (int ichar(0); ichar < name.size(); ++ichar)
			vnames[iname + ichar] = name[ichar];
		iname += name.size() + 1;
	}
	int status = XPRSaddnames(xpr, 2, vnames.data(), 0, ncols - 1);
	if (status) {
		std::cout << "XPRSaddnames error" << std::endl;
		std::exit(0);
	}
	std::vector<double> lb(ncols);
	std::vector<double> ub(ncols);
	XPRSgetlb(xpr, lb.data(), 0, ncols - 1);
	XPRSgetub(xpr, ub.data(), 0, ncols - 1);
	std::vector<double> posinf(ncols, XPRS_PLUSINFINITY);
	std::vector<double> neginf(ncols, XPRS_MINUSINFINITY);
	std::vector<char> lb_char(ncols, 'L');
	std::vector<char> ub_char(ncols, 'U');
	std::vector<int> indexes;
	indexes.reserve(nintero_pdt);
	for (auto const & id : interco_data) {
		indexes.push_back(id.first);
	}
	// remove bounds on intero
	XPRSchgbounds(xpr, nintero_pdt, indexes.data(), lb_char.data(), neginf.data());
	XPRSchgbounds(xpr, nintero_pdt, indexes.data(), ub_char.data(), posinf.data());
	// create pMax variable
	int nintero = interco_id.size();
	std::vector<int> mstart(nintero + 1, 0);
	std::vector<double> obj_interco(nintero, 0);
	std::vector<double> lb_interco(nintero, +XPRS_MINUSINFINITY);
	std::vector<double> ub_interco(nintero, +XPRS_PLUSINFINITY);
	status = XPRSaddcols(xpr, nintero, 0, obj_interco.data(), mstart.data(), NULL, NULL, lb_interco.data(), ub_interco.data());
	if (status) {
		std::cout << "interco XPRSaddcols error" << std::endl;
		std::exit(0);
	}
	for (auto const & interco : interco_id) {
		std::stringstream buffer;
		int interco_i = interco.first[1];
		int pays_or = std::get<1>(intercos_map[interco_i]);
		int pays_ex = std::get<2>(intercos_map[interco_i]);
		int pays = interco.first[0];
		buffer << "INVEST_INTERCO_" << interco_i;
		//buffer << "INVEST_INTERCO_" << area_names[pays == pays_or ? pays_or : pays_ex] << "_" << area_names[pays == pays_or ? pays_ex : pays_or];
		status = XPRSaddnames(xpr, 2, buffer.str().c_str(), ncols + interco.second, ncols + interco.second);
		if (status) {
			std::cout << "interco XPRSaddnames error" << std::endl;
			std::exit(0);
		}
		couplings[{buffer.str(), mps_name}] = interco.second + ncols;
	}
	std::vector<double> dmatval;
	std::vector<int> colind;
	std::vector<char> rowtype;
	std::vector<double> rhs;
	std::vector<int> rstart;
	// create plower and upper constraint
	for (auto const & kvp : interco_data) {
		int const i_interco_pmax(interco_id.find({ kvp.second[0], kvp.second[1] })->second);
		int const i_interco_p(kvp.first);

		std::string const & paysor(Candidates::area_names[std::get<1>(intercos_map[kvp.second[1]])]);
		std::string const & paysex(Candidates::area_names[std::get<2>(intercos_map[kvp.second[1]])]);

		Candidate & candidate(*(key_paysor_paysex.find({ paysor, paysex })->second));
		// p[t] - alpha.pMax <= 0
		rstart.push_back(dmatval.size());
		rhs.push_back(0);
		rowtype.push_back('L');
		colind.push_back(i_interco_p);
		dmatval.push_back(1);
		colind.push_back(ncols + i_interco_pmax);
		dmatval.push_back(-candidate.profile(kvp.second[2], study_path));
		// p[t] + alpha.pMax >= 0
		rstart.push_back(dmatval.size());
		rhs.push_back(0);
		rowtype.push_back('G');
		colind.push_back(i_interco_p);
		dmatval.push_back(1);
		colind.push_back(ncols + i_interco_pmax);
		dmatval.push_back(candidate.profile(kvp.second[2], study_path));
	}
	int n_row_interco(rowtype.size());
	int n_coeff_interco(dmatval.size());
	rstart.push_back(dmatval.size());
	status = XPRSaddrows(xpr, n_row_interco, n_coeff_interco, rowtype.data(), rhs.data(), NULL, rstart.data(), colind.data(), dmatval.data());
	if (status) {
		std::cout << "interco XPRSaddnames error" << std::endl;
		std::exit(0);
	}
	std::string const lp_name = mps[0].substr(0, mps[0].size() - 4);
	//XPRSwriteprob(xpr, (root + PATH_SEPARATOR + "lp" + PATH_SEPARATOR + lp_name + ".lp").c_str(), "l");
	XPRSwriteprob(xpr, (root + PATH_SEPARATOR + "lp" + PATH_SEPARATOR + lp_name + ".mps").c_str(), "");
	XPRSdestroyprob(xpr);
	std::cout << "lp_name : " << lp_name <<" done" << std::endl;
}


void Candidates::treatloop(std::string const & root, std::map< std::pair<std::string, std::string>, int>& couplings) {
	int n_mps(0);
	for (auto const & mps : Candidates::MPS_LIST) {
		treat(root, mps, couplings);
		n_mps += 1;
	}
}

