#pragma once

#include "../state.hpp"

class coin_rule : public rule {
private:
	unsigned short int raw_num_childs(state_t const &s, unsigned int gid) const {
		unsigned int num_op = 0;
		unsigned int num_nodes_ = s.num_nodes(gid);
		for (unsigned int node_id_c = 0; node_id_c < num_nodes_; ++node_id_c)
			num_op += s.operation(gid, node_id_c) != none_t;

		return std::pow(2, num_op);
	}

public:
	enum {
		none_t,
		flip_t,
	};

	/* constructor */
	coin_rule(PROBA_TYPE teta, PROBA_TYPE phi) : rule(teta, phi) {}

	op_type_t operation(state_t const &s, unsigned int gid, unsigned short int node) const override {
		short int sum = s.left(gid, node) + s.right(gid, node);
		return sum == 1 ? flip_t : none_t;
	}

	unsigned short int num_childs(state_t const &s, unsigned int gid) const override {
		/* check for calssical cases */
		if (do_not == 0)
			return 1;

		return raw_num_childs(s, gid);
	}

	void child_properties(size_t &hash_, PROBA_TYPE &real, PROBA_TYPE &imag, unsigned int &num_nodes, unsigned int &num_sub_node,
		state_t const &s, unsigned int parent_id, unsigned int child_id) const override {
		/* check for one calssical case */
		if (do_not == 0)
			child_id = raw_num_childs(s, parent_id) - 1;

		real = s.real[parent_id];
		imag = s.imag[parent_id];

		num_sub_node = s.num_sub_node(parent_id);

		hash_ = 0;
		size_t left_hash_ = 0;
		size_t right_hash_ = 0;

		PROBA_TYPE temp, sign;

		num_nodes = s.num_nodes(parent_id);
		for (unsigned short int node = 0; node < num_nodes; ++node) {
			unsigned short int node_id = s.node_id(parent_id, node);
			auto operation = s.operation(parent_id, node);

			/* update hash */
			boost::hash_combine(hash_, s.hash(parent_id, node_id));
			
			if (operation == flip_t) {
				bool do_ = child_id & 1;
				child_id >>= 1;

				/* update hashes */
				if (do_ == s.left(parent_id, node)) {
					boost::hash_combine(right_hash_, node);
				} else
					boost::hash_combine(left_hash_, node);

				/* update probas */
				sign = (PROBA_TYPE)(s.left(parent_id, node)*2 - 1);
				if (do_) {
					temp = real;
					real = temp*do_real + sign*imag*do_imag;
					imag = imag*do_real - sign*temp*do_imag;
				} else {
					real *= sign*do_not;
					imag *= sign*do_not;
				}
			} else {
				/* update hashes */
				if (s.left(parent_id, node))
					boost::hash_combine(left_hash_, node);

				if (s.right(parent_id, node))
					boost::hash_combine(right_hash_, node);
			}

		}

		boost::hash_combine(hash_, left_hash_);
		boost::hash_combine(hash_, right_hash_);
	}

	void populate_new_graph(state_t const &s, state_t &new_state, unsigned int next_gid, unsigned int parent_id, unsigned int child_id) const override {
		/* copy nodes */
		auto node_begin = s.node_begin[parent_id];
		auto node_end = s.node_begin[parent_id + 1];
		auto new_node_begin = new_state.node_begin[next_gid];

		auto sub_node_begin = s.sub_node_begin[parent_id];
		auto sub_node_end = s.sub_node_begin[parent_id + 1];
		auto new_sub_node_begin = new_state.sub_node_begin[next_gid];

		std::copy(s.node_id_c.begin() + node_begin, s.node_id_c.begin() + node_end, new_state.node_id_c.begin() + new_node_begin);

		std::copy(s.left_idx__or_element__and_has_most_left_zero__or_is_trash_.begin() + sub_node_begin, s.left_idx__or_element__and_has_most_left_zero__or_is_trash_.begin() + sub_node_end, new_state.left_idx__or_element__and_has_most_left_zero__or_is_trash_.begin() + new_sub_node_begin);
		std::copy(s.right_idx__or_type_.begin() + sub_node_begin, s.right_idx__or_type_.begin() + sub_node_end, new_state.right_idx__or_type_.begin() + new_sub_node_begin);
		std::copy(s.node_hash.begin() + sub_node_begin, s.node_hash.begin() + sub_node_end, new_state.node_hash.begin() + new_sub_node_begin);

		/* check for the second classical case */
		if (do_not == 0)
			child_id = raw_num_childs(s, parent_id) - 1;

		unsigned short int num_nodes_ = node_end - node_begin;
		for (unsigned short int node = 0; node < num_nodes_; ++node) {

			unsigned short int node_id = s.node_id(parent_id, node);
			auto operation = s.operation(parent_id, node);
			
			if (operation != none_t) {
				bool do_ = child_id & 1;

				if (do_ == s.left(parent_id, node)) {
					new_state.set_left(next_gid, node, false);
					new_state.set_right(next_gid, node, true);
				} else {
					new_state.set_left(next_gid, node, true);
					new_state.set_right(next_gid, node, false);
				}

				child_id >>= 1;
			} else {
				new_state.set_left(next_gid, node, s.left(parent_id, node));
				new_state.set_right(next_gid, node, s.right(parent_id, node));
			}

		}
	}
};