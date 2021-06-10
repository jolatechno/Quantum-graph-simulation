#pragma once

#include <stdio.h>
#include <vector>
#include <boost/functional/hash.hpp>

// forward declaration of the node_t type 
typedef struct node node_t;

short int const point_l_idx = -1;
short int const point_r_idx = -2;
short int const element_idx = -3;

class node {
private:
	size_t hash_ = 0;
	short int left_idx__or_element__and_has_most_left_zero_; // use sign bit for has_most_left_zero_ !!
	short int right_idx__or_type_;
public:

	// constructors
	node(short int n) :
		left_idx__or_element__and_has_most_left_zero_(n + 1), right_idx__or_type_(element_idx) {

		hash_ = n;
		boost::hash_combine(hash_, element_idx);
		
		if (n == 0)
			left_idx__or_element__and_has_most_left_zero_ = -left_idx__or_element__and_has_most_left_zero_;
	}

	node(short int const idx, node_t const &other, short int const type) :
		left_idx__or_element__and_has_most_left_zero_(idx + 1), right_idx__or_type_(type) {

		boost::hash_combine(hash_, other.hash_);
		boost::hash_combine(hash_, type);

		// check for most left zero
		if (is_left() && other.has_most_left_zero())
			left_idx__or_element__and_has_most_left_zero_ = -left_idx__or_element__and_has_most_left_zero_;
	}

	node(short int const left_idx, node_t const &left, short int const right_idx, node_t const &right) :
		left_idx__or_element__and_has_most_left_zero_(left_idx + 1), right_idx__or_type_(right_idx){

		boost::hash_combine(hash_, left.hash_);
		boost::hash_combine(hash_, right.hash_);

		if (left.has_most_left_zero() || right.has_most_left_zero())
			left_idx__or_element__and_has_most_left_zero_ = -left_idx__or_element__and_has_most_left_zero_;
	}

	// getters
	size_t inline hash() const { return hash_; }
	short int inline left_idx() const { return std::abs(left_idx__or_element__and_has_most_left_zero_) - 1; }
	short int inline right_idx() const { return right_idx__or_type_; }
	bool inline has_most_left_zero() const { return left_idx__or_element__and_has_most_left_zero_ < 0; }

	// type getters
	bool inline is_element() const { return right_idx__or_type_ == element_idx; }
	bool inline is_pair() const { return right_idx__or_type_ >= 0; }
	bool inline is_left() const { return right_idx__or_type_ == point_l_idx; };
	bool inline is_right() const { return right_idx__or_type_ == point_r_idx; };
};

//---------------------------------------------------------
// non member functions
//---------------------------------------------------------

// debuging 
void print(node_t const &node, std::vector<node_t> const &node_buff) {
	std::function<void(node_t const&, std::vector<node_t> const&)> print_w_paraenthesis;
	print_w_paraenthesis = [&](node_t const &n, std::vector<node_t> const &node_buff) {
		if (n.is_element()) {
			printf("%d", n.left_idx());
			return;
		}

		if (n.is_pair()) {
			printf("(");
			print_w_paraenthesis(node_buff[n.left_idx()], node_buff);
			printf("∧");
			print_w_paraenthesis(node_buff[n.right_idx()], node_buff);
			printf(")");
			return;
		}

		print_w_paraenthesis(node_buff[n.left_idx()], node_buff);

		if (n.is_left()) {
			printf(".l");
			return;
		}
		printf(".r");
	};

	if (node.is_element()) {
		printf("%d", node.left_idx());
		return;
	}

	if (node.is_pair()) {
		print_w_paraenthesis(node_buff[node.left_idx()], node_buff);
		printf("∧");
		print_w_paraenthesis(node_buff[node.right_idx()], node_buff);
		return;
	}

	print_w_paraenthesis(node_buff[node.left_idx()], node_buff);

	if (node.is_left()) {
		printf(".l");
		return;
	}

	printf(".r");
}
