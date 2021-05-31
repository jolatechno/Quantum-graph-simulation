#pragma once

#include <stdio.h>
#include <vector>
#include <utility> //for pairs
#include <boost/functional/hash.hpp>

// forward declaration of the node_t type 
typedef struct node node_t;

int const point_l_idx = -1;
int const point_r_idx = -2;
int const element_idx = -3;

struct node {
private:
	int left_idx__or_element_;
	int right_idx__or_type_;
	size_t hash_ = 0;
	bool has_most_left_zero_ = false;
public:

	// constructors
	node(int n) : left_idx__or_element_(n), right_idx__or_type_(element_idx) {
		hash_ = boost::hash<int>{}(n);
		has_most_left_zero_ = n == 0;
	}

	node(int const left_idx, int const right_idx__or_type, std::vector<node_t> const &node_buff) : left_idx__or_element_(left_idx), right_idx__or_type_(right_idx__or_type) {
		boost::hash_combine(hash_, node_buff[left_idx].hash_);
		
		// check if right_idx__or_type is an index or a type
		if (right_idx__or_type >= 0) {
			boost::hash_combine(hash_, node_buff[right_idx__or_type].hash_);
			has_most_left_zero_ = node_buff[left_idx].has_most_left_zero_ || node_buff[right_idx__or_type].has_most_left_zero_;
		} else {
			boost::hash_combine(hash_, right_idx__or_type);

			// check for most left zero
			if (is_left())
				has_most_left_zero_ = node_buff[left_idx].has_most_left_zero_;
		}
	}

	// getters
	size_t inline hash() const { return hash_; }
	int inline left_idx() const { return left_idx__or_element_; }
	int inline right_idx() const { return right_idx__or_type_; }
	bool inline has_most_left_zero() const { return has_most_left_zero_; }

	//comparator
	/*bool inline equal(node_t const other, std::vector<node_t> const &this_node_buff, std::vector<node_t> const &other_node_buff) const {
		// check if both nodes have the same type
		if (right_idx__or_type_ < 0 && right_idx__or_type_ != other.right_idx__or_type_)
			return false;

		// if both nodes are element check if they are equal
		if (is_element())
			return left_idx__or_element_ == other.left_idx__or_element_;

		//if both nodes are pairs check if both element are equal
		if (is_pair())
			return this_node_buff[left_idx__or_element_].equal(other_node_buff[other.left_idx__or_element_],
				this_node_buff, other_node_buff) &&
			this_node_buff[right_idx__or_type_].equal(other_node_buff[other.right_idx__or_type_],
				this_node_buff, other_node_buff);

		//else check both left elements
		return this_node_buff[left_idx__or_element_].equal(other_node_buff[other.left_idx__or_element_],
			this_node_buff, other_node_buff);
	}*/

	// type getters
	bool inline is_element() const { return right_idx__or_type_ == element_idx; }
	bool inline is_pair() const { return right_idx__or_type_ >= 0; }
	bool inline is_left() const { return right_idx__or_type_ == point_l_idx; };
	bool inline is_right() const { return right_idx__or_type_ == point_r_idx; };

// debuging 
private:
	void print_w_paraenthesis(std::vector<node_t> const &node_buff) const {
		if (is_element()) {
			printf("%d", left_idx__or_element_);
			return;
		}

		if (is_pair()) {
			printf("(");
			node_buff[left_idx__or_element_].print_w_paraenthesis(node_buff);
			printf("∧");
			node_buff[right_idx__or_type_].print_w_paraenthesis(node_buff);
			printf(")");
			return;
		}

		node_buff[left_idx__or_element_].print_w_paraenthesis(node_buff);

		if (is_left()) {
			printf(".l");
			return;
		}

		printf(".r");
	}

public:
	void print(std::vector<node_t> const &node_buff) const {
		if (is_element()) {
			printf("%d", left_idx__or_element_);
			return;
		}

		if (is_pair()) {
			node_buff[left_idx__or_element_].print_w_paraenthesis(node_buff);
			printf("∧");
			node_buff[right_idx__or_type_].print_w_paraenthesis(node_buff);
			return;
		}

		node_buff[left_idx__or_element_].print_w_paraenthesis(node_buff);

		if (is_left()) {
			printf(".l");
			return;
		}

		printf(".r");
	}
};


/*// forward declaration of the node_t type 
typedef class node node_t;

// node class
class node {
public:
	// node type enum
	typedef enum node_type {
		element_t,
		pair_t,
		left_t,
		right_t,
	} node_type_t;

private:
	// type and node variables 
	node_type_t type_;
	int element_;
	node_t* left_or_single_;
	node_t* right_;
	size_t hash_int = 0;
	bool _has_most_left_zero = false;

	// private constructor 
	node(node_t* n, node_type_t type) : left_or_single_(n), type_(type) {
        boost::hash_combine(hash_int, n->hash_int);
        boost::hash_combine(hash_int, type);
	}

	// for debuging 
	void print_w_paraenthesis();
public:
	// normal constructors 
	node(node_t* left, node_t* right);
	node(int n) : element_(n), type_(element_t) {
		hash_int = boost::hash<int>{}(n);
		_has_most_left_zero = n == 0;
	}

	// copy opperator 
	node_t inline*copy() {
		// no need to copy here (: 
		return this;
	}

	// getters 
	node_type_t const &type() const { return type_; }
	int integer_name() const { return element_; }
	node_t* const &left_node() const { return left_or_single_; }
	node_t* const &right_node() const { return right_; }
	node_t* const &single_node() const { return left_or_single_; }

	// hasher 
	std::string hash(int size) const {
		std::string str = "";
		size_t hash_buff = hash_int;
		for (int i = 0; i < size; ++i) {
			str += hash_buff % 256;
			hash_buff /= 256;
		}

		return str;
	}

	// for debuging 
	void print();

	// comparators 
	bool inline equal(node_t* other) const;
	bool inline const &has_most_left_zero() const { return _has_most_left_zero; }

	// split 
	node_t* left();
	node_t* right();
};

// comparator 
bool node::equal(node_t* other) const {
	// check if types match 
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

// merge 
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

// left 
node_t* node::left() {
	// split a node pair 
	if (type_ == pair_t)
		return left_or_single_;

	// create a new left node 
	node_t* left = new node(this, left_t);
	left->_has_most_left_zero = _has_most_left_zero;
	return left;
}

// right 
node_t* node::right() {
	// split a node pair 
	if (type_ == pair_t)
		return right_;

	// create a new left node 
	node_t* right = new node(this, right_t);
	return right;
}

// debuging 
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
}*/