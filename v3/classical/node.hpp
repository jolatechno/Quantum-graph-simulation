#pragma once

#include <stdio.h>

/* forward declaration of the node_t type */
typedef class node node_t;

/* node class */
class node {
private:
  /* node type enum */
	typedef enum node_type {
		element_t,
		pair_t,
		left_t,
		right_t,
	} node_type_t;

	/* type and node variables */
	node_type_t type;
	int element_;
	node_t* left_right_;
	node_t* left_;
	node_t* right_;
	bool _has_most_left_zero = false;

	/* private constructor */
	node(node_t* left_right) : left_right_(left_right) {}

	/* for debuging */
	void print_w_paraenthesis();
public:
	/* normal constructors */
	node(node_t* left, node_t* right);
	node(int n) : element_(n) {
		type = element_t;
		_has_most_left_zero = n == 0;
	}

	/* copy opperator */
	node_t inline*copy() {
		/* no need to copy here (: */
		return this;
	}

	/* for debuging */
	void print();

	/* comparators */
	bool inline equal(node_t* other);
	bool inline const &has_most_left_zero() { return _has_most_left_zero; }

	/* split */
	node_t* left();
	node_t* right();
};

/* comparator */
bool node::equal(node_t* other) {
	/* check if types match */
	if (type != other->type)
		return false;

	switch(type) {
		case element_t:
		{
			return element_ == other->element_;
		}

		case pair_t:
		{
			return left_->equal(other->left_) && right_->equal(other->right_);
		}

		default:
		{
			return left_right_->equal(other->left_right_);
		}
	}
}

/* debuging */
void node::print_w_paraenthesis() {
	switch(type) {
		case element_t:
		{
			printf("%d", element_);
			return;
		}

		case pair_t:
		{
			printf("(");
			left_->print_w_paraenthesis();
			printf("∧");
			right_->print_w_paraenthesis();
			printf(")");
			return;
		}

		case left_t:
		{
			left_right_->print_w_paraenthesis();
			printf(".l");
			return;
		}

		default:
		{
			left_right_->print_w_paraenthesis();
			printf(".r");
			return;
		}
	}
}

void node::print() {
	switch(type) {
		case element_t:
		{
			printf("%d", element_);
			return;
		}

		case pair_t:
		{
			left_->print_w_paraenthesis();
			printf("∧");
			right_->print_w_paraenthesis();
			return;
		}

		case left_t:
		{
			left_right_->print_w_paraenthesis();
			printf(".l");
			return;
		}

		case right_t:
		{
			left_right_->print_w_paraenthesis();
			printf(".r");
			return;
		}
	}
}


/* merge */
node::node(node_t* left, node_t* right) {
	_has_most_left_zero = left->has_most_left_zero() || right->has_most_left_zero();

	if (left->type == left_t && right->type == right_t) {
		if (left->left_right_->equal(right->left_right_))
			*this = *left->left_right_;
	} else {
		type = pair_t;
		left_ = left;
		right_ = right;
	}
}

/* left */
node_t* node::left() {
	/* split a node pair */
	if (type == pair_t)
		return left_;

	/* create a new left node */
	node_t* left = new node(this);
	left->type = left_t;
	left->_has_most_left_zero = _has_most_left_zero;
	return left;
}

/* right */
node_t* node::right() {
	/* split a node pair */
	if (type == pair_t)
		return right_;

	/* create a new left node */
	node_t* right = new node(this);
	right->type = right_t;
	return right;
}
