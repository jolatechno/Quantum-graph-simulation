#pragma once

#include <stdio.h>
#include <string>
#include <boost/functional/hash.hpp>

/* forward declaration of the node_t type */
typedef class node node_t;

/* node class */
class node {
public:
	/* node type enum */
	typedef enum node_type {
		element_t,
		pair_t,
		left_t,
		right_t,
	} node_type_t;

private:
	/* type and node variables */
	node_type_t type_;
	int element_;
	node_t* left_or_single_;
	node_t* right_;
	size_t hash_int = 0;
	bool _has_most_left_zero = false;

	/* private constructor */
	node(node_t* n, node_type_t type) : left_or_single_(n), type_(type) {
        boost::hash_combine(hash_int, n->hash_int);
        boost::hash_combine(hash_int, type);
	}

	/* for debuging */
	void print_w_paraenthesis();
public:
	/* normal constructors */
	node(node_t* left, node_t* right);
	node(int n) : element_(n), type_(element_t) {
		hash_int = boost::hash<int>{}(n);
		_has_most_left_zero = n == 0;
	}

	/* copy opperator */
	node_t inline*copy() {
		/* no need to copy here (: */
		return this;
	}

	/* getters */
	node_type_t const &type() const { return type_; }
	int integer_name() const { return element_; }
	node_t* const &left_node() const { return left_or_single_; }
	node_t* const &right_node() const { return right_; }
	node_t* const &single_node() const { return left_or_single_; }

	/* hasher */
	std::string hash(int size) const {
		std::string str = "";
		size_t hash_buff = hash_int;
		for (int i = 0; i < size; ++i) {
			str += hash_buff % 256;
			hash_buff /= 256;
		}

		return str;
	}

	/* for debuging */
	void print();

	/* comparators */
	bool inline equal(node_t* other) const;
	bool inline const &has_most_left_zero() const { return _has_most_left_zero; }

	/* split */
	node_t* left();
	node_t* right();
};

/* comparator */
bool node::equal(node_t* other) const {
	/* check if types match */
	if (type_ != other->type_)
		return false;

	switch(type_) {
		case element_t:
			return element_ == other->element_;

		case pair_t:
			return left_or_single_->equal(other->left_or_single_) && right_->equal(other->right_);

		default:
			return left_or_single_->equal(other->left_or_single_);
	}
}

/* merge */
node::node(node_t* left, node_t* right) {
    boost::hash_combine(hash_int, left->hash_int);
    boost::hash_combine(hash_int, right->hash_int);

	_has_most_left_zero = left->has_most_left_zero() || right->has_most_left_zero();

	if (left->type_ == left_t && right->type_ == right_t) {
		if (left->left_or_single_->equal(right->left_or_single_))
			*this = *left->left_or_single_;
	} else {
		type_ = pair_t;
		left_or_single_ = left;
		right_ = right;
	}
}

/* left */
node_t* node::left() {
	/* split a node pair */
	if (type_ == pair_t)
		return left_or_single_;

	/* create a new left node */
	node_t* left = new node(this, left_t);
	left->_has_most_left_zero = _has_most_left_zero;
	return left;
}

/* right */
node_t* node::right() {
	/* split a node pair */
	if (type_ == pair_t)
		return right_;

	/* create a new left node */
	node_t* right = new node(this, right_t);
	return right;
}

/* debuging */
void node::print_w_paraenthesis() {
	switch(type_) {
		case element_t:
		{
			printf("%d", element_);
			return;
		}

		case pair_t:
		{
			printf("(");
			left_or_single_->print_w_paraenthesis();
			printf("∧");
			right_->print_w_paraenthesis();
			printf(")");
			return;
		}

		case left_t:
		{
			left_or_single_->print_w_paraenthesis();
			printf(".l");
			return;
		}

		default:
		{
			left_or_single_->print_w_paraenthesis();
			printf(".r");
			return;
		}
	}
}

void node::print() {
	switch(type_) {
		case element_t:
		{
			printf("%d", element_);
			return;
		}

		case pair_t:
		{
			left_or_single_->print_w_paraenthesis();
			printf("∧");
			right_->print_w_paraenthesis();
			return;
		}

		case left_t:
		{
			left_or_single_->print_w_paraenthesis();
			printf(".l");
			return;
		}

		case right_t:
		{
			left_or_single_->print_w_paraenthesis();
			printf(".r");
			return;
		}
	}
}