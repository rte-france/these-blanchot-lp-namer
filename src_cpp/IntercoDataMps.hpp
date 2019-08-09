#pragma once
#include "common.hpp"


/*!
 *  \struct Candidate
 *  \brief Candidate structure
 *
 */
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


/*!
 *  \struct Candidates
 *  \brief Candidates structure
 *
 */
struct Candidates : public std::map<std::string, Candidate> {

	static std::vector<std::vector<std::string> > MPS_LIST;			/*!< vector of 3 strings in a vector corresponding to the name of a mps , variable and constraint file */
	static std::vector<std::tuple<int, int, int> >  intercos_map;	/*!< vector of 3 int in tuple which correspond to interconnections */

	static std::map<std::tuple<std::string, std::string>, int> or_ex_id; /*!< map of tuple < origin country, destination country> associated to the int id of the interconnection */

	static std::set<std::string> str_fields;
	static std::set<std::string> dbl_fields;
	
	static std::vector<std::string> area_names;						/*!< vector of string corresponding to area */


	Candidates() {

	}
	Candidates(std::string  const & datas);

	void treat(std::string const & root, std::vector<std::string> const &, std::map< std::pair<std::string, std::string>, int>& couplings);
	void treatloop(std::string const & root, std::map< std::pair<std::string, std::string>, int>& couplings);
	void getCandidatesFromFile(std::string  const & dataPath);

};

