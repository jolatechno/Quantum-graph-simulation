#pragma once

#include "graph_name.hpp"
#include <vector>
#include <stdio.h>
#include <utility> //for pairs
#include <random>
#include <chrono>
#include <algorithm>    // std::rotate
#include <boost/functional/hash.hpp>

// forward declaration of the graph_t type 
typedef class graph graph_t;
size_t const separator = boost::hash<int>{}(-1); 

class graph {
private:
	// variables 
	graph_name_t* name_;
	std::vector<unsigned int> left_;
	std::vector<unsigned int> right_;

	// hash
	size_t mutable hash_ = 0;
	bool mutable hashed_ = false;

	// useful functions 
	void overflow_left(std::vector<unsigned int>& pos);
	void overflow_right(std::vector<unsigned int>& pos);
	void rotate_once_left(std::vector<unsigned int>& pos);
	void rotate_once_right(std::vector<unsigned int>& pos);

public:
	// normal constructors 
	graph(int n) {
		name_ = new graph_name(n);
	}

	graph(int n, std::vector<unsigned int> left, std::vector<unsigned int> right) : left_(left), right_(right) {
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
		this_copy->left_ = left_;
		this_copy->right_ = right_;

		// save memory 
		left_.shrink_to_fit();
		right_.shrink_to_fit();
	
		return this_copy;
	}

	// getters 
	std::vector<unsigned int> inline const &left() const { return left_; };
	std::vector<unsigned int> inline const &right() const { return right_; };
	graph_name_t inline const *name() const { return name_; };

	// hasher 
	size_t inline hash() const;

	// randomizer 
	void randomize(unsigned int n);
	void randomize();

	// for debuging 
	void print();

	// comparators 
  //bool inline equal(graph_t* other) const;

  // split and merge 
  // split_merge type enum 
  typedef enum split_merge_type {
		split_t,
		merge_t,
	} split_merge_type_t;
	// typedef of the pair of an index and a split_merge_type 
  	typedef std::pair<unsigned int, split_merge_type_t> split_merge_t;
  	// functions 
  	void inline split_merge(std::vector<split_merge_t>& split_merge); //indexes have to be sorted !!

  	// step function 
  	void inline step() {
  		hashed_ = false;

  		rotate_once_right(right_);
  		rotate_once_left(left_);
  	}
  	void inline reversed_step() {
  		hashed_ = false;

  		rotate_once_right(left_);
  		rotate_once_left(right_);
  	}
};

// hasher 
size_t graph::hash() const {
	if (hashed_)
		return hash_;

	hash_ = 0;
	boost::hash<unsigned int> hasher;

	// left hash 
	for (auto &l : left_)
		boost::hash_combine(hash_, hasher(l));

  // separator 
  boost::hash_combine(hash_, separator);

  // right hash 
  for (auto &r : right_)
   	boost::hash_combine(hash_, hasher(r));

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
void graph::split_merge(std::vector<split_merge_t>& split_merge) {
	hashed_ = false;

	if (left_.size() == 0 ||
	right_.size() == 0 ||
	split_merge.size() == 0)
		return;

	// check for last merge 
	auto last_split_merge = split_merge.end() - 1;
	int last_idx = size() - 1;
	if ((*last_split_merge).first == last_idx &&
	(*last_split_merge).second == merge_t) {
		name_->merge(last_idx);

		overflow_right(left_);
		
		split_merge.pop_back();

		--last_split_merge;
		--last_idx;
	}
		
	// check for first element split 
	bool first_split = false;

	// split and merge names 
	// and calculating max displacement 
	int total_displacement = 0;
	for (auto split_merge_it = split_merge.rbegin(); split_merge_it != split_merge.rend(); ++split_merge_it)
		if ((*split_merge_it).second == split_t) {
			first_split |= name_->split((*split_merge_it).first) && (*split_merge_it).first == 0;

			++total_displacement;
		} else {
			name_->merge((*split_merge_it).first);
			--total_displacement;
		}

	// move particules 
	int displacement = total_displacement;
	auto split_merge_it = last_split_merge;
	for (auto left_it = left_.rbegin(); left_it != left_.rend(); ++left_it) {
		// check if there are any nodes left 
		while (split_merge_it >= split_merge.begin()) {
			// check if the node is split or merged 
			if ((*split_merge_it).first >= *left_it) {
				if ((*split_merge_it).second == split_t) {
					--displacement; // decrement the displacement 
				} else
					++displacement; // decrement the displacement 
				--split_merge_it;
			} else
				break;

		}

		if (split_merge_it == split_merge.begin() && displacement == 0)
			break;

		*left_it += displacement;
	}

	split_merge_it = last_split_merge;
	for (auto right_it = right_.rbegin(); right_it != right_.rend(); ++right_it) {
		// check if there are any nodes left 
		while (split_merge_it >= split_merge.begin()) {
			// check if the node is split or merged 
			if ((*split_merge_it).first > *right_it) {
				if ((*split_merge_it).second == split_t) {
					--total_displacement; // decrement the displacement 
				} else
					++total_displacement; // decrement the displacement 
				--split_merge_it;
			} else
				break;

		}

		if (split_merge_it == split_merge.begin() && total_displacement == 0)
			break;

		*right_it += total_displacement;
	}

	// finish first split 
	if (first_split) {
		rotate_once_left(left_);
		rotate_once_left(right_);
	}
}

// randomize function 
void graph::randomize() {
	hashed_ = false;

	// clear both vector 
	left_.clear();
	right_.clear();

	// random genearator 
	unsigned seed = std::chrono::steady_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	std::exponential_distribution<double> distribution(1);

	// useful variables 
	unsigned int x_max = size();
	unsigned int x = ((unsigned int)distribution(generator))%x_max;
	
	// generate left values 
	while (x < x_max) {
		left_.push_back(x);
		x += 1 + (unsigned int)distribution(generator);
	}

	// generate right values 
	x = ((unsigned int)distribution(generator))%x_max;
	while (x < x_max) {
		right_.push_back(x);
		x += 1 + (unsigned int)distribution(generator);
	}

	// save memory 
	left_.shrink_to_fit();
	right_.shrink_to_fit();
}

// randomize with a set number of particules going each ways 
void graph::randomize(unsigned int n) {
	hashed_ = false;

	// clear both vector 
	left_.clear();
	right_.clear();

	// random genearator 
	unsigned seed = std::chrono::steady_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	std::exponential_distribution<double> distribution(1);

	// useful variables 
	unsigned int x_max = size() - n;
	unsigned int x = ((unsigned int)distribution(generator))%x_max;

	// generate left values 
	for (int i = 0; i < n; i++) {
		left_.push_back(x);
		x_max = size() - x - (n - i - 1);
		x += 1 + (unsigned int)distribution(generator)%(x_max - 1);
	}

	// generate left values 
	x_max = size() - n;
	x = ((unsigned int)distribution(generator))%x_max;
	for (int i = 0; i < n; i++) {
		right_.push_back(x);
		x_max = size() - x - (n - i - 1);
		x += 1 + (unsigned int)distribution(generator)%(x_max - 1);
	}

	// save memory 
	left_.shrink_to_fit();
	right_.shrink_to_fit();
}

// debuging 
void graph::print() {
	// iterate through particules positions 
	auto left_it = left_.begin();
	auto right_it = right_.begin();

	// node list 
	auto nodes = name_->nodes();
	for(unsigned int i = 0; i < size(); ++i) {
		// left character 
		char l = ' ';

		// if there is any left particules left 
		if (left_it < left_.end())
			// check if there is a particule at the left port 
			if (*left_it == i) {
				l = '<';

				// iterate left iterator
				++left_it;
			}

		// right character 
		char r = ' ';

		// if there is any right particules left 
		if (right_it < right_.end())
			// check if there is a particule at the right port 
			if (*right_it == i) {
				r = '>';

				// iterate right iterator
				++right_it;
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
	if (left_ != other->left_)
		return false;

	// check positions of right going particules 
	if (right_ != other->right_)
		return false;

	 // check if names match 
	return name_->equal(other->name_);
}*/