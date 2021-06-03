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
  	std::vector<bool> mutable left;
	std::vector<bool> mutable right;

private:
	// variables 
	graph_name_t* name_;

	// hash
	size_t mutable hash_ = 0;
	bool mutable hashed_ = false;

public:
	// normal constructors 
	graph(short int n) {
		left = std::vector<bool>(n, false);
		right = std::vector<bool>(n, false);
		name_ = new graph_name(n);
	}

	graph(std::vector<bool> left, std::vector<bool> right) : left(left), right(right) {
		if (left.size() != right.size())
			throw;

		name_ = new graph_name(left.size());
	}

	// size operator 
	size_t inline size() const {
		return name_->size();
	};

	// copy opperator 
	graph_t inline *copy() {
		// just copy the vectors 
		graph_t* this_copy = new graph(0);
		this_copy->name_ = name_->copy();
		this_copy->left = left;
		this_copy->right = right;

		// save memory 
		left.shrink_to_fit();
		right.shrink_to_fit();
	
		return this_copy;
	}

	// getters 
	graph_name_t inline const *name() const { return name_; };

	// hasher 
	size_t inline hash() const;

	// randomizer 
	void randomize();

	// comparators 
  	//bool inline equal(graph_t* other) const;

	// split and merge 
  	void inline split_merge(std::vector<op_t>& split_merge); //indexes have to be sorted !!
  	void inline erase_create(std::vector<op_t>& erase_create); //indexes have to be sorted !!

  	// step function 
  	void inline step();
  	void inline reversed_step() {
  		std::swap(left, right);
  		step();
  		std::swap(left, right);
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
	for (int i = 0; i < size(); ++i)
		if (left[i])
			boost::hash_combine(hash_, i);

 	 // separator 
  	boost::hash_combine(hash_, separator);

  	// right hash 
  	for (int i = 0; i < size(); ++i)
		if (right[i])
			boost::hash_combine(hash_, i);

  	// separator 
  	boost::hash_combine(hash_, separator);

  	// name hash 
  	boost::hash_combine(hash_, name_->hash());
		

  hashed_ = true;
  return hash_;
}

//step
void inline graph::step() {
	hashed_ = false;

	// step right particules
	bool previous = right.back();
	for (int i = 0; i < size(); ++i)
		swap(right[i], previous);

	previous = left[0];
	for (int i = size() - 1; i >= 0; --i)
		swap(left[i], previous);
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
			first_split |= name_->split(pos);

			// change right
			right[pos] = false;

			//insert
			right.insert(right.begin() + pos + 1, true);
			left.insert(left.begin() + pos + 1, false);
		} else if (type == merge_t) {
			name_->merge(pos);

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