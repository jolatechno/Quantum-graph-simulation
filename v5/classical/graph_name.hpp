#pragma once

#include "node.hpp"
#include <algorithm>

// forward declaration of the graph_t type 
typedef class graph_name graph_name_t;

class graph_name {
private:
	// node list 
	std::vector<node_t> mutable nodes_;
	std::vector<node_t> mutable node_buff_;
	std::vector<short int> mutable trash_collection_;

	// hash
	size_t mutable hash_ = 0;
	bool mutable hashed_ = false;

	short int push_to_buffer(node_t &n) {
		if (trash_collection_.empty()) {
			node_buff_.push_back(n);
			return node_buff_.size() - 1;
		}

		short int buff_idx = trash_collection_.back();
		trash_collection_.pop_back();
		node_buff_[buff_idx] = n;
		return buff_idx;
	}

	// debugging
	friend void print(graph_name_t const &name, short int n);

public:
	// normal constructors 
	graph_name() {}
	
	graph_name(unsigned short int size) {
		for (short int i = 0; i < size; ++i)
			nodes_.emplace_back(i);
	}

	// getters
	std::vector<node_t> const &nodes() const { return nodes_; }

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
			boost::hash_combine(hash_, n.hash());

		hashed_ = true;
		return hash_;
	}

	// split and merge 
  	bool inline split(unsigned short int idx);
  	void inline merge(unsigned short int idx);
};

// split merge
bool inline graph_name::split(unsigned short int idx) {
	hashed_ = false;

	node_t node = nodes_[idx];
	if (node.is_pair()) {
		// read indexes
		short int const left_idx = node.left_idx();
		short int const right_idx = node.right_idx();

		// add left node
		nodes_[idx] = node_buff_[left_idx];
		trash_collection_.push_back(left_idx);

		// add right node
		nodes_.insert(nodes_.begin() + idx + 1, node_buff_[right_idx]);
		trash_collection_.push_back(right_idx);

		// check if we should rotate
		if (idx == 0 && nodes_[1].has_most_left_zero()) {
			std::rotate(nodes_.begin(), nodes_.begin() + 1, nodes_.end());
			return true; 
		}

		return false;
	}

	// add current node to buffer
	short int buff_idx = push_to_buffer(node);

	// add left node
	nodes_[idx] = node_t(buff_idx, node, point_l_idx);

	// add right node
	nodes_.emplace(nodes_.begin() + idx + 1, buff_idx, node, point_r_idx);
	
	return false;
}

void inline graph_name::merge(unsigned short int idx) {
	hashed_ = false;

	// destination idx
	unsigned short int right_idx = (idx + 1) % size();
	auto left = nodes_[idx];
	auto right = nodes_[right_idx];

	if (left.is_left() && right.is_right()) {
		short int const left_left = left.left_idx();
		short int const right_left = right.left_idx();

		if (node_buff_[left_left].hash() == node_buff_[right_left].hash()) {
			// trash collect
			trash_collection_.push_back(left_left);
			if (left_left != right_left)
				trash_collection_.push_back(right_left);

			//add node
			nodes_[right_idx] = node_buff_[left_left];
			nodes_.erase(nodes_.begin() + idx);
			return;
		}
	}

	//add node
	short int const left_idx = push_to_buffer(left);
	short int const right_idx_ = push_to_buffer(right);

	nodes_[right_idx] = node_t(left_idx, left, right_idx_, right);
	nodes_.erase(nodes_.begin() + idx);
}

//---------------------------------------------------------
// non member functions
//---------------------------------------------------------

// debuging
void print(graph_name_t const &name, short int n) {
	print(name.nodes_[n], name.node_buff_);
}

void print(graph_name_t const &name) {
	for (short int i = 0; i < name.size(); i++) {
		printf("-");
		print(name, i);
		printf("-");
	}
}