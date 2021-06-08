#pragma once

#include "node.hpp"
#include <algorithm>

// forward declaration of the graph_t type 
typedef class graph_name graph_name_t;

class graph_name {
private:
	// node list 
	std::vector<short unsigned int> mutable nodes_; // index buffer !!
	std::vector<node_t> mutable node_buff_;
	std::vector<short unsigned int> mutable trash_collection_;

	// hash
	size_t mutable hash_ = 0;
	bool mutable hashed_ = false;

	short unsigned int push_pair_to_buffer(short int left_idx, short int right_idx) {
		if (trash_collection_.empty()) {
			node_buff_.emplace_back(left_idx, node_buff_[left_idx], right_idx, node_buff_[right_idx]);
			return node_buff_.size() - 1;
		}

		short unsigned int buff_idx = trash_collection_.back();
		trash_collection_.pop_back();
		node_buff_[buff_idx] = node_t(left_idx, node_buff_[left_idx], right_idx, node_buff_[right_idx]);
		return buff_idx;
	}

	short unsigned int push_left_or_right_to_buffer(short int idx, bool left) {
		if (trash_collection_.empty()) {
			node_buff_.emplace_back(idx, node_buff_[idx], left ? point_l_idx : point_r_idx);
			return node_buff_.size() - 1;
		}

		short unsigned int buff_idx = trash_collection_.back();
		trash_collection_.pop_back();
		node_buff_[buff_idx] = node_t(idx, node_buff_[idx], left ? point_l_idx : point_r_idx);
		return buff_idx;
	}

	// debugging
	friend void print(graph_name_t const &name, short unsigned int n);

public:
	// normal constructors 
	graph_name() {}
	
	graph_name(short unsigned int size) {
		for (short unsigned int i = 0; i < size; ++i) {
			nodes_.push_back(i);
			node_buff_.emplace_back(i);
		}
	}

	// getters
	std::vector<short unsigned int> const &nodes() const { return nodes_; }

	// size operator 
	size_t inline size() const {
		return nodes_.size();
	}

	//hasher
	size_t inline hash() const {
		if (hashed_)
			return hash_;

		// memory managment at hashing time
		trash_collection_.shrink_to_fit();
		node_buff_.shrink_to_fit();
		nodes_.shrink_to_fit();

		hash_ = 0;

		/* combine all nodes hash */
		for (auto &n : nodes_)
			boost::hash_combine(hash_, node_buff_[n].hash());

		hashed_ = true;
		return hash_;
	}

	// split and merge 
  	bool inline split(short unsigned int idx);
  	void inline merge(short unsigned int idx);
};

// split merge
bool inline graph_name::split(short unsigned int idx) {
	hashed_ = false;

	node_t node = node_buff_[nodes_[idx]];
	if (node.is_pair()) {
		// read indexes
		short unsigned int const left_idx = node.left_idx();
		short unsigned int const right_idx = node.right_idx();

		// add trash
		trash_collection_.push_back(nodes_[idx]);

		// add right node
		nodes_.insert(nodes_.begin() + idx + 1, right_idx);

		// add left node
		nodes_[idx] = left_idx;

		// check if we should rotate
		if (idx == 0 && node_buff_[nodes_[1]].has_most_left_zero()) {
			std::rotate(nodes_.begin(), nodes_.begin() + 1, nodes_.end());
			return true; 
		}

		return false;
	}

	// add current node to buffer
	short unsigned int left_idx = push_left_or_right_to_buffer(nodes_[idx], true);
	short unsigned int right_idx = push_left_or_right_to_buffer(nodes_[idx], false);

	// add left node
	nodes_[idx] = left_idx;

	// add right node
	nodes_.insert(nodes_.begin() + idx + 1, right_idx);
	
	return false;
}

void inline graph_name::merge(short unsigned int idx) {
	hashed_ = false;

	// destination idx
	short unsigned int right_idx = (idx + 1) % size();
	auto left = node_buff_[nodes_[idx]];
	auto right = node_buff_[nodes_[right_idx]];

	if (left.is_left() && right.is_right()) {
		short unsigned int const left_left = left.left_idx();
		short unsigned int const right_left = right.left_idx();

		if (node_buff_[left_left].hash() == node_buff_[right_left].hash()) {
			// trash collect
			if (left_left != right_left)
				trash_collection_.push_back(right_left);

			//add node
			nodes_[right_idx] = left_left;
			nodes_.erase(nodes_.begin() + idx);
			return;
		}
	}

	nodes_[right_idx] = push_pair_to_buffer(nodes_[idx], nodes_[right_idx]);
	nodes_.erase(nodes_.begin() + idx);
}

//---------------------------------------------------------
// non member functions
//---------------------------------------------------------

// debuging
void print(graph_name_t const &name, short unsigned int n) {
	print(name.node_buff_[name.nodes_[n]], name.node_buff_);
}

void print(graph_name_t const &name) {
	for (short unsigned int i = 0; i < name.size(); i++) {
		printf("-");
		print(name, i);
		printf("-");
	}
}