#pragma once

#include "../state.hpp"

class split_merge_rule : public rule {
private:
	PROBA_TYPE merge_real = 1;
	PROBA_TYPE merge_imag = 0;
	PROBA_TYPE non_merge = 0;

public:
	enum {
		none_t,
		split_t,
		merge_t,
	};

	/* constructor */
	split_merge_rule(PROBA_TYPE teta, PROBA_TYPE phi) {
		merge_real = precision::cos(teta)*precision::cos(phi);
		merge_imag = precision::cos(teta)*precision::sin(phi);
		non_merge = precision::sin(teta);
	}

	/* rule implementation */
	op_type_t operation(state_t const &s, unsigned int gid, unsigned short int node) const override {
		unsigned short int node_id = s.node_id(gid, node);
		if (s.left(gid, node_id) && s.right(gid, node_id))
			return split_t;

		unsigned short int next_node = node_id == s.numb_nodes(gid) - 1 ? 0 : node + 1;
		unsigned short int next_node_id = s.node_id(gid, next_node);
		if (s.left(gid, node_id) && s.right(gid, next_node_id) && !s.left(gid, next_node_id))
			return merge_t;

		return none_t;
	}

	unsigned short int numb_childs(state_t const &s, unsigned int gid) const override {
		if (non_merge == 0 || non_merge == 1)
			return 1;

		unsigned int numb_op = 0;
		unsigned int numb_nodes_ = s.numb_nodes(gid);
		for (unsigned int nid = 0; nid < numb_nodes_; ++nid)
			numb_op += s.operation(gid, nid) != none_t;

		return std::pow(2, numb_op);
	}

	std::tuple<size_t, PROBA_TYPE, PROBA_TYPE, unsigned short int, unsigned short int> 
	child_properties(state_t const &s, unsigned int parent_id, unsigned int child_id) const override {

		PROBA_TYPE real = s.real[parent_id];
		PROBA_TYPE imag = s.imag[parent_id];

		unsigned short int b_size = s.numb_nodes(parent_id);
		unsigned short int c_size = s.c_size(parent_id);

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

		unsigned short int numb_nodes_ = s.numb_nodes(parent_id);
		for (unsigned short int node = first_split_overflow; node < numb_nodes_; ++node) {

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
							/* update c_size */
							--c_size;

							/* update node hash */
							boost::hash_combine(hash_, s.hash(parent_id, s.left_idx(parent_id, node_id)));
							boost::hash_combine(hash_, s.hash(parent_id, s.right_idx(parent_id, node_id)));
						} else {
							/* update c_size */
							c_size += 2;

							/* update node hash */
							boost::hash_combine(hash_, s.hash_node_by_value(parent_id, node_id, state::left_t));
							boost::hash_combine(hash_, s.hash_node_by_value(parent_id, node_id, state::right_t));
						}

						/* update probas */
						PROBA_TYPE temp = real;
						real = temp*merge_real + imag*merge_imag;
						imag = temp*merge_imag - imag*merge_real;
					} else {
						/* update probas */
						real *= -non_merge;
						imag *= -non_merge;

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

						unsigned short int next_node = node == numb_nodes_ - 1 ? 0 : node + 1;
						unsigned short int next_node_id = s.node_id(parent_id, next_node);

						if (s.node_type(parent_id, node_id) == state_t::left_t &&
						s.node_type(parent_id, next_node_id) == state_t::right_t &&
						s.hash(parent_id, s.left_idx(parent_id, node_id)) == s.hash(parent_id, s.left_idx(parent_id, next_node_id))) {
							/* update c_size */
							c_size -= 2;

							/* update node hash */
							boost::hash_combine(hash_, s.hash(parent_id, s.left_idx(parent_id, node_id)));
						} else {
							/* update c_size */
							++c_size;

							/* update node hash */
							boost::hash_combine(hash_, s.hash_node_by_value(parent_id, node_id, next_node_id));
						}

						/* update probas */
						PROBA_TYPE temp = real;
						real = temp*merge_real - imag*merge_imag;
						imag = temp*merge_imag + imag*merge_real;
					} else {
						/* update probas */
						real *= non_merge;
						imag *= non_merge;

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

		/* update b_size */
		b_size += displacement;

		if (first_split_overflow) {
			/* update hashes */
			boost::hash_combine(hash_, s.hash(parent_id, s.left_idx(parent_id, s.node_id(parent_id, 0))));
			boost::hash_combine(left_hash_, b_size - 1);
		}

		boost::hash_combine(hash_, left_hash_);
		boost::hash_combine(hash_, right_hash_);

		return {hash_, real, imag, b_size, std::max(c_size, s.c_size(parent_id))};
	}

	void populate_new_graph(state_t const &s, state_t &new_state, unsigned int gid, unsigned int parent_id, unsigned int child_id) const override {
		/* copy nodes */
		auto c_begin = s.c_begin[parent_id];
		auto c_end = s.c_begin[parent_id + 1];
		auto new_c_begin = new_state.c_begin[gid];

		std::copy(s.left_idx__or_element__and_has_most_left_zero_.begin() + c_begin, s.left_idx__or_element__and_has_most_left_zero_.begin() + c_end, new_state.left_idx__or_element__and_has_most_left_zero_.begin() + new_c_begin);
		std::copy(s.right_idx__or_type_.begin() + c_begin, s.right_idx__or_type_.begin() + c_end, new_state.right_idx__or_type_.begin() + new_c_begin);
		std::copy(s.node_hash.begin() + c_begin, s.node_hash.begin() + c_end, new_state.node_hash.begin() + new_c_begin);

		/* get trash */
		std::vector<unsigned short int> trash;

		auto child_id_ = child_id;
		unsigned short int numb_nodes_ = s.numb_nodes(parent_id);
		for (unsigned short int node = 0; node < numb_nodes_; ++node) {
			auto operation = s.operation(parent_id, node);

			if (operation != none_t) {
				bool do_ = child_id_ & 1;
				child_id_ >>= 1;

				unsigned short int node_id = s.node_id(parent_id, node);
				if (do_)
					if (operation == split_t) {
						if (s.node_type(parent_id, node_id) == state_t::pair_t)
							trash.push_back(node_id);
					} else {
						unsigned short int next_node = node == numb_nodes_ - 1 ? 0 : node + 1;
						unsigned short int next_node_id = s.node_id(parent_id, next_node);

						if (s.node_type(parent_id, node_id) == state_t::left_t &&
						s.node_type(parent_id, next_node_id) == state_t::right_t &&
						s.hash(parent_id, s.left_idx(parent_id, node_id)) == s.hash(parent_id, s.left_idx(parent_id, next_node_id))) {
							trash.push_back(node_id);
							trash.push_back(next_node_id);
						}
					}
			}
		}

		/* finish filling the trash */
		for (unsigned int i = s.c_size(parent_id); i < new_state.c_size(gid); ++i)
			trash.push_back(i);

		/* get trash */
		auto const get_trash = [&]() {
			auto idx = trash.back();
			trash.pop_back();
			return idx;
		};

		int displacement = 0;

		/* check if there is a "first split overflow" to keep the lexicographic order */
		bool first_split_overflow = (child_id & 1) && (s.operation(parent_id, 0) == split_t);

		if (first_split_overflow)
			first_split_overflow = s.node_type(parent_id, s.node_id(parent_id, 0)) == state_t::pair_t;

		if (first_split_overflow)
			first_split_overflow = s.has_most_left_zero(parent_id, s.right_idx(parent_id, s.node_id(parent_id, 0)));

		for (unsigned short int node = 0; node < numb_nodes_; ++node) {
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
					} else {
						auto new_left_node_idx = get_trash();
						new_state.set_node_id(gid, node + displacement - 1, new_left_node_idx);
						new_state.set_left_idx(gid, new_left_node_idx, node_id);
						new_state.set_type(gid, new_left_node_idx, state::left_t);
						new_state.set_most_left_zero(gid, new_left_node_idx, s.has_most_left_zero(parent_id, node_id));

						auto new_right_node_idx = get_trash();
						new_state.set_node_id(gid, node + displacement, new_right_node_idx);
						new_state.set_left_idx(gid, new_right_node_idx, node_id);
						new_state.set_type(gid, new_right_node_idx, state::right_t);
					}
				} else {
					/* set node values */
					new_state.set_left(gid, node + displacement, true);
					new_state.set_right(gid, node + displacement, true);

					/* merge node */
					unsigned short int next_node = node == numb_nodes_ - 1 ? 0 : node + 1;
					unsigned short int next_node_id = s.node_id(parent_id, next_node);

					if (s.node_type(parent_id, node_id) == state_t::left_t &&
					s.node_type(parent_id, next_node_id) == state_t::right_t &&
					s.hash(parent_id, s.left_idx(parent_id, node_id)) == s.hash(parent_id, s.left_idx(parent_id, next_node_id))) {
						new_state.set_node_id(gid, node + displacement, s.left_idx(parent_id, node_id));
					} else {
						auto new_node_idx = get_trash();
						new_state.set_node_id(gid, node + displacement, new_node_idx);

						new_state.set_left_idx(gid, new_node_idx, node_id);
						new_state.set_right_idx(gid, new_node_idx, next_node_id);

						new_state.set_most_left_zero(gid, new_node_idx, s.has_most_left_zero(parent_id, node_id) || s.has_most_left_zero(parent_id, next_node_id));
					}

					/* update displacement */
					--displacement;
				}
			} else {
				new_state.set_node_id(gid, node + displacement, s.node_id(parent_id, node));
				new_state.set_left(gid, node + displacement, s.left(parent_id, node));
				new_state.set_right(gid, node + displacement, s.right(parent_id, node));
			}
		}

		if (first_split_overflow) {
			auto begin = new_state.b_begin[gid];
			auto end = new_state.b_begin[gid + 1];

			/* rotate nodes */
			std::rotate(new_state.left_.begin() + begin, new_state.left_.begin() + begin + 1, new_state.left_.begin() + end);
			std::rotate(new_state.right_.begin() + begin, new_state.right_.begin() + begin + 1, new_state.right_.begin() + end);
			std::rotate(new_state.nid.begin() + begin, new_state.nid.begin() + begin + 1, new_state.nid.begin() + end);
		}
	}
};