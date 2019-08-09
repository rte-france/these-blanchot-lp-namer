#pragma once
#include "xprs_driver.hpp"
#include "common.hpp"


/*!
 *  \struct Candidate
 *  \brief Candidate structure
 *
 */
struct Candidate {
	std::map<std::string, std::string> _str; /*!<  map of string , string associated type of link (origin, destination) and the country */
	std::map<std::string, double> _dbl;

	std::vector<double> _profile;

	int _id;

	/**
	 * \fn dbl(std::string const & key)
	 * \brief Get the element of _dbl associated to the key "key" or 0.0 if the key doe not exist
	 *
	 * \param key String corresponding to the key
	 * \return string
	 */
	double dbl(std::string const & key)const {
		auto const it(_dbl.find(key));
		return it == _dbl.end() ? 0.0 : it->second;
	}

	/**
	 * \fn str(std::string const & key)
	 * \brief Get the element of _str associated to the key "key"
	 *
	 * \param key String corresponding to the key
	 * \return string
	 */
	std::string str(std::string const & key)const {
		auto const it(_str.find(key));
		return it->second;
	}

	/**
	 * \fn has(std::string const & key)
	 * \brief Check if the key "key" is present in _str
	 *
	 * \param key String corresponding to the key
	 * \return bool
	 */
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
	void getListOfIntercoCandidates(map<std::pair<std::string, std::string>, Candidate *> & key_paysor_paysex);
	void readCstrfiles(std::string const filePath, std::list<std::string> & list, size_t & sizeList);
	void readVarfiles(std::string const filePath,
			          std::list<std::string> & list,
					  size_t & sizeList,
					  std::map<int, std::vector<int> > & interco_data ,
					  std::map<std::vector<int>, int> & interco_id,
					  map<std::pair<std::string, std::string>, Candidate *> key_paysor_paysex);
	void createMpsFileAndFillCouplings(std::string const mps_name,
									   std::list<std::string> var,
									   size_t vsize,
									   std::list<std::string> cstr,
									   size_t csize,
									   std::map<int, std::vector<int> > interco_data,
									   std::map<std::vector<int>, int> interco_id,
									   std::map< std::pair<std::string, std::string>, int> & couplings,
									   map<std::pair<std::string, std::string>, Candidate *>  key_paysor_paysex,
									   std::string study_path,
									   std::string const lp_mps_name);


};

