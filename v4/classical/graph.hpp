#pragma once

#include "graph_name.hpp"
#include <vector>
#include <stdio.h>
#include <utility> //for pairs
#include <random>
#include <chrono>
#include <algorithm>    // std::rotate
#include <boost/functional/hash.hpp>
#include<ranges>

// forward declaration of the graph_t type 
typedef class graph graph_t;
size_t const separator = -1; 

class graph {
private:
	// variables 
	graph_name_t* name_;

	// hash
	size_t mutable hash_ = 0;
	bool mutable hashed_ = false;

	// useful functions 
	void overflow_left(std::vector<unsigned int>& pos);
	void overflow_right(std::vector<unsigned int>& pos);
	void rotate_once_left(std::vector<unsigned int>& pos);
	void rotate_once_right(std::vector<unsigned int>& pos);

public:
	std::vector<unsigned int> left;
	std::vector<unsigned int> right;

	// normal constructors 
	graph(int n) {
		name_ = new graph_name(n);
	}

	graph(int n, std::vector<unsigned int> left, std::vector<unsigned int> right) : left(left), right(right) {
		name_ = new graph_name(n);
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

	// add particules
	void inline add_left(unsigned int n) {
		left.push_back(n);
		std::inplace_merge(left.begin(), left.end() - 1, left.end());
	}
	void inline add_left(std::vector<unsigned int> n) {
		left.insert(left.end(), n.begin(), n.end());
		std::inplace_merge(left.begin(), left.end() - n.size(), left.end());
	}
	void inline add_right(int n) {
		right.push_back(n);
		std::inplace_merge(right.begin(), right.end() - 1, right.end());
	}
	void inline add_right(std::vector<unsigned int> n) {
		right.insert(right.end(), n.begin(), n.end());
		std::inplace_merge(right.begin(), right.end() - n.size(), right.end());
	}

	// randomizer 
	void randomize(unsigned int n);
	void randomize();

	// for debuging 
	void print();

	// comparators 
  //bool inline equal(graph_t* other) const;

  // split and merge 
  // split_merge type enum 
  typedef enum op_type {
		split_t,
		merge_t,
	} op_type_t;
	// typedef of the pair of an index and a op_type 
  	typedef std::pair<unsigned int, op_type_t> op_t;
  	// functions 
  	void inline split_merge(std::vector<op_t>& split_merge); //indexes have to be sorted !!

  	// step function 
  	void inline step() {
  		hashed_ = false;

  		rotate_once_right(right);
  		rotate_once_left(left);
  	}
  	void inline reversed_step() {
  		hashed_ = false;

  		rotate_once_right(left);
  		rotate_once_left(right);
  	}
};

// hasher 
size_t graph::hash() const {
	if (hashed_)
		return hash_;

	hash_ = 0;
	boost::hash<unsigned int> hasher;

	// left hash 
	for (auto &l : left)
		boost::hash_combine(hash_, l);

 	 // separator 
  	boost::hash_combine(hash_, separator);

  	// right hash 
  	for (auto &r : right)
   		boost::hash_combine(hash_, r);

  	// separator 
  	boost::hash_combine(hash_, separator);

  	// name hash 
  	boost::hash_combine(hash_, name_->hash());
		

  hashed_ = true;
  return hash_;
}

// usefull functions 
void graph::overflow_left(std::vector<unsigned int>& pos) {
	if (pos.size() == 0)
		return;

	pos[0] = size() - 1;
	std::rotate(pos.begin(), pos.begin() + 1, pos.end());
}

void graph::overflow_right(std::vector<unsigned int>& pos) {
	if (pos.size() == 0)
		return;

	std::rotate(pos.begin(), pos.end() - 1, pos.end());
	pos[0] = 0;
}

void graph::rotate_once_left(std::vector<unsigned int>& pos) {
	if (pos.size() == 0)
		return;

	bool overflowed = pos[0] == 0;
	for (auto it = pos.begin(); it < pos.end(); ++it)
		--(*it);

	if (overflowed)
		overflow_left(pos);
}

void graph::rotate_once_right(std::vector<unsigned int>& pos) {
	if (pos.size() == 0)
		return;

	bool overflowed = *(pos.end() - 1) == size() - 1;
	for (auto it = pos.begin(); it < pos.end(); ++it)
		++(*it);

	if (overflowed)
		overflow_right(pos);
}

// split merge 
void graph::split_merge(std::vector<op_t>& split_merge) {
	if (split_merge.empty())
		return;
	
	hashed_ = false;

	// check for last merge
	if (split_merge.back() == op_t(size() - 1, merge_t)) {
		name_->merge(size() - 1);
		overflow_right(left);	
		split_merge.pop_back();
	}
		
	// check for first element split 
	bool first_split = false;

	// and calculating max displacement 
	int total_displacement = 0;
	for (auto & [pos, type] : split_merge | std::ranges::views::reverse)
		if (type == split_t) {
			++total_displacement;
		} else
			--total_displacement;

	// reserve space for nodes
	name_->reserve(total_displacement);

	// split and merge names
	for (auto & [pos, type] : split_merge | std::ranges::views::reverse)
		if (type == split_t) {
			first_split |= name_->split(pos);
		} else
			name_->merge(pos);

	// move particules
	auto const split_merge_begin = split_merge.rend();
	int displacement = total_displacement;
	auto split_merge_it = split_merge.rbegin();
	for (auto &leftit : left | std::ranges::views::reverse) {
		// check if there are any nodes left
		for (;split_merge_it->first >= leftit && split_merge_it != split_merge_begin; ++split_merge_it)
			// check if the node is split or merged
			if (split_merge_it->second == split_t) {
					--displacement; // decrement the displacement 
				} else
					++displacement; // decrement the displacement

		//check if we can exit
		if (split_merge_it == split_merge_begin && displacement == 0)
			break;

		leftit += displacement;
	}

	split_merge_it = split_merge.rbegin();
	for (auto &rightit : right | std::ranges::views::reverse) {
		// check if there are any nodes left
		for (;split_merge_it->first > rightit && split_merge_it != split_merge_begin; ++split_merge_it)
			// check if the node is split or merged
			if (split_merge_it->second == split_t) {
					--total_displacement; // decrement the displacement 
				} else
					++total_displacement; // decrement the displacement

		//check if we can exit
		if (split_merge_it == split_merge_begin && total_displacement == 0)
			break;

		//increment position
		rightit += total_displacement;
	}

	// finish first split 
	if (first_split) {
		rotate_once_left(left);
		rotate_once_left(right);
	}
}

// randomize function 
void graph::randomize() {
	hashed_ = false;

	// clear both vector 
	left.clear();
	right.clear();

	// random genearator 
	unsigned seed = std::chrono::steady_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	std::exponential_distribution<double> distribution(1);

	// useful variables 
	unsigned int x_max = size();
	unsigned int x = ((unsigned int)distribution(generator))%x_max;
	
	// generate left values 
	while (x < x_max) {
		left.push_back(x);
		x += 1 + (unsigned int)distribution(generator);
	}

	// generate right values 
	x = ((unsigned int)distribution(generator))%x_max;
	while (x < x_max) {
		right.push_back(x);
		x += 1 + (unsigned int)distribution(generator);
	}

	// save memory 
	left.shrink_to_fit();
	right.shrink_to_fit();
}

// randomize with a set number of particules going each ways 
void graph::randomize(unsigned int n) {
	hashed_ = false;

	// clear both vector 
	left.clear();
	right.clear();

	// random genearator 
	unsigned seed = std::chrono::steady_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	std::exponential_distribution<double> distribution(1);

	// useful variables 
	unsigned int x_max = size() - n;
	unsigned int x = ((unsigned int)distribution(generator))%x_max;

	// generate left values 
	for (int i = 0; i < n; i++) {
		left.push_back(x);
		x_max = size() - x - (n - i - 1);
		x += 1 + (unsigned int)distribution(generator)%(x_max - 1);
	}

	// generate left values 
	x_max = size() - n;
	x = ((unsigned int)distribution(generator))%x_max;
	for (int i = 0; i < n; i++) {
		right.push_back(x);
		x_max = size() - x - (n - i - 1);
		x += 1 + (unsigned int)distribution(generator)%(x_max - 1);
	}

	// save memory 
	left.shrink_to_fit();
	right.shrink_to_fit();
}

// debuging 
void graph::print() {
	// iterate through particules positions 
	auto leftit = left.begin();
	auto rightit = right.begin();

	// node list 
	auto nodes = name_->nodes();
	for(unsigned int i = 0; i < size(); ++i) {
		// left character 
		char l = ' ';

		// if there is any left particules left 
		if (leftit < left.end())
			// check if there is a particule at the left port 
			if (*leftit == i) {
				l = '<';

				// iterate left iterator
				++leftit;
			}

		// right character 
		char r = ' ';

		// if there is any right particules left 
		if (rightit < right.end())
			// check if there is a particule at the right port 
			if (*rightit == i) {
				r = '>';

				// iterate right iterator
				++rightit;
			}

		printf("-|%c|", l);
		name_->print(i);
		printf("|%c|-", r);
	}
}

// comparator 
/*bool graph::equal(graph_t* other) const {
	// check if the  number of nodes match 
	if (size() != other->size())
    	return false;

	// check positions of left going particules 
	if (left != other->left)
		return false;

	// check positions of right going particules 
	if (right != other->right)
		return false;

	 // check if names match 
	return name_->equal(other->name_);
}*/