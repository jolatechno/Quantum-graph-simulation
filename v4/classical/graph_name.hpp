#pragma once

#include "node.hpp"
#include <vector>
#include <stdio.h>

/* forward declaration of the graph_t type */
typedef class graph_name graph_name_t;

/* graph class */
class graph_name {
private:
	/* node list */
	std::vector<node_t*> nodes_;

public:
	/* normal constructors */
	graph_name(unsigned int size) {
		for (int i = 0; i < size; ++i)
			nodes_.push_back(new node(i));
	}

	/* size operator */
	size_t inline size() {
		return nodes_.size();
	}

	/* getter */
	std::vector<node_t*> inline const &nodes() {
		return nodes_;
	}

	/* copy opperator */
	graph_name_t inline *copy() {
		/* just copy the vector */
		graph_name_t* this_copy = new graph_name(0);
		this_copy->nodes_ = nodes_; // !! should check copy !!
		return this_copy;
	}

	/* for debuging */
	void print();

	/* comparators */
  	bool inline equal(const graph_name* other) const;

  	/* split and merge */
  	bool inline split(unsigned int idx);
  	void inline merge(unsigned int idx);
};

/* comparator */
bool graph_name::equal(const graph_name* other) const {
	/* check if size match */
	if (nodes_.size() != other->nodes_.size())
		return false;

	/* iterate through node names */
	auto this_it = nodes_.begin();
	auto other_it = other->nodes_.begin();

	while(this_it < nodes_.end()) {
		/* compare each node pair */
		if (!(*this_it)->equal(*other_it))
			return false;

 	    ++this_it; ++other_it;
	}

	return true;
}

/* split */
bool graph_name::split(unsigned int idx) {
	/* if the node isn't the first */
	if (idx > 0) {
		/* add new node */
		nodes_.insert(nodes_.begin() + idx + 1, nodes_[idx]->right());

		/* replace old node */
		nodes_[idx] = nodes_[idx]->left();

		/* no need to rotate */
		return false;
	}
	
	/* new nodes_ */
	node_t* left = nodes_[idx]->left();
	node_t* right = nodes_[idx]->right();

	if (left->has_most_left_zero()) {
		/* add new node */
		nodes_.insert(nodes_.begin() + 1, right);

		/* replace old node */
		nodes_[0] = left;

		/* no need to rotate */
		return false;
	}

	/* add new node */
	nodes_.push_back(left);

	/* replace old node */
	nodes_[0] = right;

	/* particules should be rotated by one to the left */
	return true;
}

/* merge */
void graph_name::merge(unsigned int idx) {
	/* if the node isn't the last */
	if (idx < nodes_.size() - 1) {
		/* add new node */
		nodes_[idx] = new node(nodes_[idx], nodes_[idx + 1]);

		/* delete old node */
		nodes_.erase(nodes_.begin() + idx + 1);
		return;
	}

	/* add new node */
	nodes_[0] = new node(nodes_[idx], nodes_[0]);

	/* delete old node */
	nodes_.pop_back();
}

/* debuging */
void graph_name::print() {
	for(auto this_it = nodes_.begin(); this_it < nodes_.end(); ++this_it) {
		printf("-");
		(*this_it)->print();
		printf("-");
	}
}