#pragma once

#include "graph_name.hpp"
#include <cstdlib>
#include <ctime>
#include <ranges>

// forward declaration of the graph_t type 
typedef class graph graph_t;
size_t const separator = -1; 

class graph {
public:
  	// split_merge type enum 
  	typedef enum op_type {
		split_t,
		merge_t,
		erase_t,
		create_t,
	} op_type_t;

	// typedef of the pair of an index and a op_type 
  	typedef std::pair<unsigned short int, op_type_t> op_t;

  	// particules positions
  	std::vector<char /*bool*/> mutable left; //not bool to not use space efficient bool vectors
	std::vector<char /*bool*/> mutable right;

private:
	// variables 
	graph_name_t name_;

	// hash
	size_t mutable hash_ = 0;
	bool mutable hashed_ = false;

public:
	// normal constructors 
	graph(short int n) {
		left = std::vector<char /*bool*/>(n, false);
		right = std::vector<char /*bool*/>(n, false);
		name_ = graph_name_t(n);
	}

	graph(std::vector<char /*bool*/> left_, std::vector<char /*bool*/> right_) : left(left_), right(right_) {
		if (left.size() != right.size())
			throw;

		name_ = graph_name_t(left.size());
	}

	// size operator 
	size_t inline size() const {
		return left.size();
	};

	// getters 
	graph_name_t inline const &name() const { return name_; };

	// hasher 
	size_t inline hash() const;

	// randomizer 
	void randomize();

	// split and merge 
  	void inline split_merge(std::vector<op_t>& split_merge); //indexes have to be sorted !!
  	void inline erase_create(std::vector<op_t>& erase_create); //indexes have to be sorted !!

  	// step function 
  	void inline step() {
  		hashed_ = false;

  		std::rotate(left.begin(), left.begin() + 1, left.end());
		std::rotate(right.rbegin(), right.rbegin() + 1, right.rend());
  	}
  	void inline reversed_step() {
  		hashed_ = false;

  		std::rotate(left.rbegin(), left.rbegin() + 1, left.rend());
		std::rotate(right.begin(), right.begin() + 1, right.end());
  	}
};

// hasher 
size_t graph::hash() const {
	if (hashed_)
		return hash_;

	// save memory 
	left.shrink_to_fit();
	right.shrink_to_fit();

	hash_ = 0;
	boost::hash<unsigned int> hasher;

	// left hash 
	auto const size_ = size();
	for (short int i = 0; i < size_; ++i)
		if (left[i])
			boost::hash_combine(hash_, i);

 	 // separator 
  	boost::hash_combine(hash_, separator);

  	// right hash 
  	for (short int i = 0; i < size_; ++i)
		if (right[i])
			boost::hash_combine(hash_, i);

  	// separator 
  	boost::hash_combine(hash_, separator);

  	// name hash 
  	boost::hash_combine(hash_, name_.hash());
		

  hashed_ = true;
  return hash_;
}

// split merge 
void graph::split_merge(std::vector<op_t>& split_merge) {
	if (split_merge.empty())
		return;
	
	hashed_ = false;

	bool first_split = false;
	auto const size_ = size();
	for (auto & [pos, type] : split_merge | std::ranges::views::reverse)
		if (type == split_t) {
			first_split |= name_.split(pos);

			// change right
			right[pos] = false;

			//insert
			right.insert(right.begin() + pos + 1, true);
			left.insert(left.begin() + pos + 1, false);
		} else if (type == merge_t) {
			name_.merge(pos);

			// next pos
			unsigned short int next_pos = (pos + 1) % size_;
			
			// change left
			left[next_pos] = true;

			//erase
			right.erase(right.begin() + pos);
			left.erase(left.begin() + pos);
		}

	/* finish first split */
	if (first_split) {
		std::rotate(left.begin(), left.begin() + 1, left.end());
		std::rotate(right.begin(), right.begin() + 1, right.end());
	}
}

void inline graph::erase_create(std::vector<op_t>& erase_create) {
	if (erase_create.empty())
		return;
	
	hashed_ = false;

	for (auto & [pos, type] : erase_create)
		if (type == erase_t) {
			left[pos] = false;
			right[pos] = false;
		} else if (type == create_t) {
			left[pos] = true;
			right[pos] = true;
		}
}

// randomize function 
void graph::randomize() {
	hashed_ = false;

	// random genearator 
	std::srand(std::time(nullptr)); // use current time as seed for random generator
    for (int i = 0; i < size(); ++i) {
    	left[i] = std::rand() % 2;
    	right[i] = std::rand() % 2;
    }
}

//---------------------------------------------------------
// non member functions
//---------------------------------------------------------

// debuging 
void print(graph_t const *g) {
	// iterate through particules positions 
	auto left_it = g->left.begin();
	auto right_it = g->right.begin();

	for(unsigned short int i = 0; i < g->size(); ++i) {
		printf("-|%c|", g->left[i] ? '<' : ' ');
		print(g->name(), i);
		printf("|%c|-", g->right[i] ? '>' : ' ');
	}
}