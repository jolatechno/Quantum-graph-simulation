#pragma once

#include "../state.hpp"

class split_merge_rule : public rule {
public:
	enum {
		none_t,
		split_t,
		merge_t,
	};

	/* constructor */
	split_merge_rule(PROBA_TYPE teta, PROBA_TYPE phi) {
		do_real = precision::cos(teta)*precision::cos(phi);
		do_imag = precision::cos(teta)*precision::sin(phi);
		do_not = precision::sin(teta);
	}

	/* rule implementation */
	op_type_t operation(state_t const &s, unsigned int gid, unsigned short int node) const override {
		if (s.left(gid, node) && s.right(gid, node))
			return split_t;

		unsigned short int next_node = node == s.num_nodes(gid) - 1 ? 0 : node + 1;
		if (s.left(gid, node) && s.right(gid, next_node) && !s.left(gid, next_node))
			return merge_t;

		return none_t;
	}

	unsigned short int num_childs(state_t const &s, unsigned int gid) const override {
		if (do_not == 0 || do_not == 1)
			return 1;

		unsigned int num_op = 0;
		unsigned int num_nodes_ = s.num_nodes(gid);
		for (unsigned int node = 0; node < num_nodes_; ++node)
			num_op += s.operation(gid, node) != none_t;

		return std::pow(2, num_op);
	}

	std::tuple<size_t, PROBA_TYPE, PROBA_TYPE, unsigned short int, unsigned short int> 
	child_properties(state_t &s, unsigned int parent_id, unsigned int child_id) const override {
		PROBA_TYPE real = s.real[parent_id];
		PROBA_TYPE imag = s.imag[parent_id];

		unsigned short int node_size = s.num_nodes(parent_id);
		unsigned short int num_sub_node = s.num_sub_node(parent_id);

		/* start dicreasing num_sub_node */
		for (unsigned int node = s.sub_node_begin[parent_id]; node < s.sub_node_begin[parent_id + 1]; ++node)
			num_sub_node -= s.is_trash(parent_id, node);

		size_t hash_ = 0;
		size_t left_hash_ = 0;
		size_t right_hash_ = 0;

		/* check if there is a "first split overflow" to keep the lexicographic order */
		bool first_split_overflow = (child_id & 1) && (s.operation(parent_id, 0) == split_t);

		if (first_split_overflow)
			first_split_overflow = s.node_type(parent_id, s.node_id(parent_id, 0)) == state_t::pair_t;

		if (first_split_overflow)
			first_split_overflow = s.has_most_left_zero(parent_id, s.right_idx(parent_id, s.node_id(parent_id, 0)));

		if (first_split_overflow) {
			/* update hashes */
			boost::hash_combine(hash_, s.hash(parent_id, s.right_idx(parent_id, s.node_id(parent_id, 0))));
			boost::hash_combine(right_hash_, 0);
		}

		int displacement = -first_split_overflow;

		unsigned short int num_nodes_ = node_size;
		for (unsigned short int node = first_split_overflow; node < num_nodes_; ++node) {

			unsigned short int node_id = s.node_id(parent_id, node);
			auto operation = s.operation(parent_id, node);
			
			if (operation != none_t) {
				bool do_ = child_id & 1;
				child_id >>= 1;

				if (operation == split_t) {
					if (do_) {
						/* update left hash */
						boost::hash_combine(left_hash_, node + displacement);

						/* update displacement */
						++displacement;

						/* update right hash */
						boost::hash_combine(right_hash_, node + displacement);

						if (s.node_type(parent_id, node_id) == state_t::pair_t) {
							/* update node hash */
							boost::hash_combine(hash_, s.hash(parent_id, s.left_idx(parent_id, node_id)));
							boost::hash_combine(hash_, s.hash(parent_id, s.right_idx(parent_id, node_id)));
						} else {
							/* update sub node size */
							num_sub_node += 2;

							/* update node hash */
							boost::hash_combine(hash_, s.hash_node_by_value(parent_id,
								node == 0 ? -node_id - 1 : node_id + 1,
								state::left_t));

							boost::hash_combine(hash_, s.hash_node_by_value(parent_id, node_id + 1, state::right_t));
						}

						/* update probas */
						PROBA_TYPE temp = real;
						real = temp*do_real + imag*do_imag;
						imag = temp*do_imag - imag*do_real;
					} else {
						/* update probas */
						real *= -do_not;
						imag *= -do_not;

						/* update hashes */
						boost::hash_combine(hash_, s.hash(parent_id, node_id));
						boost::hash_combine(left_hash_, node + displacement);
						boost::hash_combine(right_hash_, node + displacement);
					}
					
				} else
					if (do_) {
						/* update left and right hashes */
						boost::hash_combine(left_hash_, node + displacement);
						boost::hash_combine(right_hash_, node + displacement);

						/* update b size */
						--displacement;

						unsigned short int next_node = node == num_nodes_ - 1 ? 0 : node + 1;
						unsigned short int next_node_id = s.node_id(parent_id, next_node);

						if (s.node_type(parent_id, node_id) == state_t::left_t &&
						s.node_type(parent_id, next_node_id) == state_t::right_t &&
						s.hash(parent_id, s.left_idx(parent_id, node_id)) == s.hash(parent_id, s.left_idx(parent_id, next_node_id))) {
							/* update node hash */
							boost::hash_combine(hash_, s.hash(parent_id, s.left_idx(parent_id, node_id)));
						} else {
							/* update sub node size */
							++num_sub_node;

							/* update node hash */
							boost::hash_combine(hash_, s.hash_node_by_value(parent_id, 
								node == 0 || node == num_nodes_ - 1 ? -node_id - 1 : node_id + 1,
								next_node_id + state_t::pair_t));
						}

						/* update probas */
						PROBA_TYPE temp = real;
						real = temp*do_real - imag*do_imag;
						imag = temp*do_imag + imag*do_real;

						/* skip next node */
						++node;
					} else {
						/* update probas */
						real *= do_not;
						imag *= do_not;

						/* update hashes */
						boost::hash_combine(hash_, s.hash(parent_id, node_id));
						boost::hash_combine(left_hash_, node + displacement);
					}
			} else {
				/* update hashes */
				boost::hash_combine(hash_, s.hash(parent_id, node_id));

				if (s.left(parent_id, node))
					boost::hash_combine(left_hash_, node + displacement);

				if (s.right(parent_id, node))
					boost::hash_combine(right_hash_, node + displacement);
			}
		}

		/* update node_size */
		node_size += displacement;

		if (first_split_overflow) {
			/* update hashes */
			boost::hash_combine(hash_, s.hash(parent_id, s.left_idx(parent_id, s.node_id(parent_id, 0))));
			boost::hash_combine(left_hash_, node_size - 1);
		}

		boost::hash_combine(hash_, left_hash_);
		boost::hash_combine(hash_, right_hash_);

		return {hash_, real, imag, node_size, std::max(num_sub_node, s.num_sub_node(parent_id))};
	}

	void populate_new_graph(state_t const &s, state_t &new_state, unsigned int gid, unsigned int parent_id, unsigned int child_id) const override {
		/* copy nodes */
		auto sub_node_begin = s.sub_node_begin[parent_id];
		auto sub_node_end = s.sub_node_begin[parent_id + 1];
		auto new_sub_node_begin = new_state.sub_node_begin[gid];

		/* copy nodes */
		std::copy(s.left_idx__or_element__and_has_most_left_zero_.begin() + sub_node_begin, s.left_idx__or_element__and_has_most_left_zero_.begin() + sub_node_end, new_state.left_idx__or_element__and_has_most_left_zero_.begin() + new_sub_node_begin);
		std::copy(s.right_idx__or_type__and_is_trash_.begin() + sub_node_begin, s.right_idx__or_type__and_is_trash_.begin() + sub_node_end, new_state.right_idx__or_type__and_is_trash_.begin() + new_sub_node_begin);
		std::copy(s.node_hash.begin() + sub_node_begin, s.node_hash.begin() + sub_node_end, new_state.node_hash.begin() + new_sub_node_begin);

		/* get trash */
		short int last_trash_idx = -1;
		short int old_sub_node_num = s.num_sub_node(parent_id);
		auto const get_trash = [&]() {
			while (true) {
				++last_trash_idx;
				if (new_state.is_trash(gid, last_trash_idx) || last_trash_idx >= old_sub_node_num) {
					new_state.set_is_trash(gid, last_trash_idx, false);
					return last_trash_idx;
				}
			}
		};

		int displacement = 0;

		/* check if there is a "first split overflow" to keep the lexicographic order */
		bool first_split_overflow = (child_id & 1) && (s.operation(parent_id, 0) == split_t);

		if (first_split_overflow)
			first_split_overflow = s.node_type(parent_id, s.node_id(parent_id, 0)) == state_t::pair_t;

		if (first_split_overflow)
			first_split_overflow = s.has_most_left_zero(parent_id, s.right_idx(parent_id, s.node_id(parent_id, 0)));

		unsigned short int num_nodes_ = s.num_nodes(parent_id);
		for (unsigned short int node = 0; node < num_nodes_; ++node) {
			auto operation = s.operation(parent_id, node);			
			bool do_ = operation != none_t;
			unsigned short int node_id = s.node_id(parent_id, node);

			if (do_) {
				do_ = child_id & 1;
				child_id >>= 1;
			}

			if (do_) {
				if (operation == split_t) {
					/* set left node values */
					new_state.set_left(gid, node + displacement, true);
					new_state.set_right(gid, node + displacement, false);

					/* update displacement */
					++displacement;

					/* set left node values */
					new_state.set_left(gid, node + displacement, false);
					new_state.set_right(gid, node + displacement, true);

					/* split node */
					if (s.node_type(parent_id, node_id) == state_t::pair_t) {
						new_state.set_node_id(gid, node + displacement - 1, s.left_idx(parent_id, node_id));
						new_state.set_node_id(gid, node + displacement, s.right_idx(parent_id, node_id));

						/* set trash */
						new_state.set_is_trash(gid, node_id, true);
					} else {
						/* left node */
						auto new_left_node_idx = get_trash();
						new_state.set_node_id(gid, node + displacement - 1, new_left_node_idx);
						new_state.set_left_idx(gid, new_left_node_idx, node_id);
						new_state.set_type(gid, new_left_node_idx, state::left_t);
						/* set has_most_left_zero */
						if (node == 0)
							new_state.set_most_left_zero(gid, new_left_node_idx, true);

						/* right node */
						auto new_right_node_idx = get_trash();
						new_state.set_node_id(gid, node + displacement, new_right_node_idx);
						new_state.set_left_idx(gid, new_right_node_idx, node_id);
						new_state.set_type(gid, new_right_node_idx, state::right_t);

						/* re-hash nodes */
						new_state.hash_node(gid, new_left_node_idx);
						new_state.hash_node(gid, new_right_node_idx);
					}
				} else {
					/* set node values */
					new_state.set_left(gid, node + displacement, true);
					new_state.set_right(gid, node + displacement, true);

					/* merge node */
					unsigned short int next_node = node == num_nodes_ - 1 ? 0 : node + 1;
					unsigned short int next_node_id = s.node_id(parent_id, next_node);

					if (s.node_type(parent_id, node_id) == state_t::left_t &&
					s.node_type(parent_id, next_node_id) == state_t::right_t &&
					s.hash(parent_id, s.left_idx(parent_id, node_id)) == s.hash(parent_id, s.left_idx(parent_id, next_node_id))) {
						new_state.set_node_id(gid, node + displacement, s.left_idx(parent_id, node_id));

						/* set trash */
						new_state.set_is_trash(gid, node_id, true);
						new_state.set_is_trash(gid, next_node_id, true);
					} else {
						/* new node */
						auto new_node_idx = get_trash();
						new_state.set_node_id(gid, node + displacement, new_node_idx);
						new_state.set_left_idx(gid, new_node_idx, node_id);
						new_state.set_right_idx(gid, new_node_idx, next_node_id);

						/* set has_most_left_zero */
						if (node == 0 || node == num_nodes_ - 1)
							new_state.set_most_left_zero(gid, new_node_idx, true);

						/* re-hash nodes */
						new_state.hash_node(gid, new_node_idx);
					}

					/* update displacement */
					--displacement;

					/* skip next node */
					++node;
				}
			} else {
				new_state.set_node_id(gid, node + displacement, s.node_id(parent_id, node));
				new_state.set_left(gid, node + displacement, s.left(parent_id, node));
				new_state.set_right(gid, node + displacement, s.right(parent_id, node));
			}
		}

		if (first_split_overflow) {
			auto begin = new_state.node_begin[gid];
			auto end = new_state.node_begin[gid + 1];

			/* rotate nodes */
			std::rotate(new_state.left_.begin() + begin, new_state.left_.begin() + begin + 1, new_state.left_.begin() + end);
			std::rotate(new_state.right_.begin() + begin, new_state.right_.begin() + begin + 1, new_state.right_.begin() + end);
			std::rotate(new_state.node_id_c.begin() + begin, new_state.node_id_c.begin() + begin + 1, new_state.node_id_c.begin() + end);
		}
	}
};