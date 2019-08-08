#pragma once
#include "common.hpp"



struct Candidate {
	std::map<std::string, std::string> _str;
	std::map<std::string, double> _dbl;

	std::vector<double> _profile;

	int _id;

	double dbl(std::string const & key)const {
		auto const it(_dbl.find(key));
		return it == _dbl.end() ? 0.0 : it->second;
	}
	std::string str(std::string const & key)const {
		auto const it(_str.find(key));
		return it->second;
	}
	bool has(std::string const & key)const {
		auto const it(_str.find(key));
		return it != _str.end();
	}

	double profile(size_t i, std::string const & study_path);

	double obj()const;
	double lb()const;
	double ub()const;
};

struct Candidates : public std::map<std::string, Candidate> {

	static std::vector<std::vector<std::string> > MPS_LIST;
	//static std::vector<double> capa_pv;
	static std::vector<std::tuple<int, int, int> >  intercos_map;

	static std::map<std::tuple<std::string, std::string>, int> or_ex_id;

	//static std::vector<std::vector<std::string>> candidates_map;
	static std::set<std::string> str_fields;
	static std::set<std::string> dbl_fields;
	
	static std::vector<std::string> area_names;


	Candidates() {

	}
	Candidates(std::string  const & datas);
	//Candidates(std::vector<std::vector<std::string>>  const & datas);

	void treat(std::string const & root, std::vector<std::string> const &, std::map< std::pair<std::string, std::string>, int>& couplings);
	void treatloop(std::string const & root, std::map< std::pair<std::string, std::string>, int>& couplings);


};

