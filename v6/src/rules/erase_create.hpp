#pragma once

#include "../state.hpp"

class erase_create_rule : public rule {
public:
	enum {
		none_t,
		create_t,
		erase_t,
	};

	/* constructor */
	erase_create_rule(PROBA_TYPE teta, PROBA_TYPE phi) {
		do_real = precision::cos(teta)*precision::cos(phi);
		do_imag = precision::cos(teta)*precision::sin(phi);
		do_not = precision::sin(teta);
	}

	op_type_t operation(state_t const &s, unsigned int gid, unsigned short int node) const override {
		short int sum = s.left(gid, node) + s.right(gid, node);

		if (sum == 2)
			return erase_t;

		if (sum == 0)
			return create_t;

		return none_t;
	}

	unsigned short int num_childs(state_t const &s, unsigned int gid) const override {
		if (do_not == 0 || do_not == 1)
			return 1;

		unsigned int num_op = 0;
		unsigned int num_nodes_ = s.num_nodes(gid);
		for (unsigned int node_id_c = 0; node_id_c < num_nodes_; ++node_id_c)
			num_op += s.operation(gid, node_id_c) != none_t;

		return std::pow(2, num_op);
	}

	std::tuple<size_t, PROBA_TYPE, PROBA_TYPE, unsigned short int, unsigned short int> 
	child_properties(state_t &s, unsigned int parent_id, unsigned int child_id) const override {
		PROBA_TYPE real = s.real[parent_id];
		PROBA_TYPE imag = s.imag[parent_id];

		unsigned short int num_sub_node = s.num_sub_node(parent_id);

		size_t hash_ = 0;
		size_t left_hash_ = 0;
		size_t right_hash_ = 0;

		unsigned short int num_nodes_ = s.num_nodes(parent_id);
		for (unsigned short int node = 0; node < num_nodes_; ++node) {
			unsigned short int node_id = s.node_id(parent_id, node);
			auto operation = s.operation(parent_id, node);

			/* update hash */
			boost::hash_combine(hash_, s.hash(parent_id, node_id));
			
			if (operation != none_t) {
				bool do_ = child_id & 1;

				/* update hashes */
				if (do_ == (operation == create_t)) {
					boost::hash_combine(left_hash_, node);
					boost::hash_combine(right_hash_, node);
				}

				/* update probas */
				if (operation == create_t) {
					if (do_) {
						PROBA_TYPE temp = real;
						real = temp*do_real - imag*do_imag;
						imag = imag*do_real + temp*do_imag;
					} else {
						real *= -do_not;
						imag *= -do_not;
					}
				} else {
					if (do_) {
						PROBA_TYPE temp = real;
						real = temp*do_real + imag*do_imag;
						imag = imag*do_real - temp*do_imag;
					} else {
						real *= do_not;
						imag *= do_not;
					}
				}

				child_id >>= 1;
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

		return {hash_, real, imag, num_nodes_, num_sub_node};
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

		std::copy(s.left_idx__or_element__and_has_most_left_zero_.begin() + sub_node_begin, s.left_idx__or_element__and_has_most_left_zero_.begin() + sub_node_end, new_state.left_idx__or_element__and_has_most_left_zero_.begin() + new_sub_node_begin);
		std::copy(s.right_idx__or_type__and_is_trash_.begin() + sub_node_begin, s.right_idx__or_type__and_is_trash_.begin() + sub_node_end, new_state.right_idx__or_type__and_is_trash_.begin() + new_sub_node_begin);
		std::copy(s.node_hash.begin() + sub_node_begin, s.node_hash.begin() + sub_node_end, new_state.node_hash.begin() + new_sub_node_begin);

		unsigned short int num_nodes_ = node_end - node_begin;
		for (unsigned short int node = 0; node < num_nodes_; ++node) {

			unsigned short int node_id = s.node_id(parent_id, node);
			auto operation = s.operation(parent_id, node);
			
			if (operation != none_t) {
				bool do_ = child_id & 1;

				if (do_ == (operation == create_t)) {
					new_state.set_left(next_gid, node, true);
					new_state.set_right(next_gid, node, true);
				} else {
					new_state.set_left(next_gid, node, false);
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