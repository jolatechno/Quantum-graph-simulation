#pragma once

#include "node.hpp"
#include <vector>
#include <stdio.h>
#include <boost/functional/hash.hpp>

// forward declaration of the graph_t type 
typedef class graph_name graph_name_t;

class graph_name {
private:
	// node list 
	std::vector<node_t> nodes_;
	std::vector<node_t> node_buff_;
	std::vector<int> trash_collection_;

	int push_to_buffer(node_t &n) {
		if (trash_collection_.empty()) {
			node_buff_.push_back(n);
			return node_buff_.size() - 1;
		}

		int buff_idx = trash_collection_.back();
		trash_collection_.pop_back();
		node_buff_[buff_idx] = n;
		return buff_idx;
	}

public:
	// normal constructors 
	graph_name(unsigned int size) {
		for (int i = 0; i < size; ++i)
			nodes_.push_back(node(i));
	}

	// getters
	std::vector<node_t> const &nodes() const { return nodes_; }

	// size operator 
	size_t inline size() const {
		return nodes_.size();
	}

	//  debuging
	void print(int n) const;
	void print() const;

	// copy opperator 
	graph_name_t inline *copy() {
		// just copy the vector 
		graph_name_t* this_copy = new graph_name(0);
		this_copy->nodes_ = nodes_;
		this_copy->node_buff_ = node_buff_;
		this_copy->trash_collection_ = trash_collection_;

		return this_copy;
	}

	//hasher
	size_t hash() {
		size_t h = 0;

		/* combine all nodes hash */
		for (auto &n : nodes_)
			boost::hash_combine(h, n.hash());

		return h;
	}

	// split and merge 
  	bool inline split(unsigned int idx);
  	void inline merge(unsigned int idx);
};

// split merge
bool inline graph_name::split(unsigned int idx) {
	node_t node = nodes_[idx];

	if (node.is_pair()) {
		// read indexes
		int const left_idx = node.left_idx();
		int const right_idx = node.right_idx();

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
	int buff_idx = push_to_buffer(node);

	// add left node
	nodes_[idx] = node_t(buff_idx, point_l_idx, node_buff_);

	// add right node
	nodes_.insert(nodes_.begin() + idx + 1,
		node_t(buff_idx, point_r_idx, node_buff_));
	
	return false;
}

void inline graph_name::merge(unsigned int idx) {
	// destination idx
	unsigned int right_idx = (idx + 1) % size();
	auto left = nodes_[idx];
	auto right = nodes_[right_idx];

	if (left.is_left() && right.is_right()) {
		int const left_left = left.left_idx();
		int const right_left = right.left_idx();

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
	int const left_idx = push_to_buffer(left);
	int const right_idx_ = push_to_buffer(right);

	nodes_[right_idx] = node_t(left_idx, right_idx_, node_buff_);
	nodes_.erase(nodes_.begin() + idx);
}

// debuging
void graph_name::print(int n) const {
	nodes_[n].print(node_buff_);
}

void graph_name::print() const {
	for (int i = 0; i < size(); i++) {
		printf("-");
		print(i);
		printf("-");
	}
}