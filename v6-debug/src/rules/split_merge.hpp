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
	split_merge_rule(PROBA_TYPE teta, PROBA_TYPE phi) : rule(teta, phi) {}

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
	child_properties(state_t const &s, unsigned int parent_id, unsigned int child_id) const override {
		PROBA_TYPE real = s.real[parent_id];
		PROBA_TYPE imag = s.imag[parent_id];

		unsigned short int node_size = s.num_nodes(parent_id);
		unsigned short int num_sub_node = s.num_sub_node(parent_id);

		/* start dicreasing num_sub_node */
		/*for (unsigned int node = 0; node < num_sub_node; ++node)
			num_sub_node -= s.is_trash(parent_id, node);*/

		size_t hash_ = 0;
		size_t left_hash_ = 0;
		size_t right_hash_ = 0;

		/* check if there is a "first split overflow" to keep the lexicographic order */
		bool first_split_overflow = (child_id & 1) && (s.operation(parent_id, 0) == split_t);

		if (first_split_overflow)
			first_split_overflow = s.node_type(parent_id, s.node_id(parent_id, 0)) == state_t::pair_t;

		if (first_split_overflow)
			first_split_overflow = s.has_most_left_zero(parent_id, s.right_idx(parent_id, s.node_id(parent_id, 0)));

		/*if (first_split_overflow) {
			/* update child id */
			/*child_id >>= 1;

			/* update hashes */
			/*boost::hash_combine(hash_, s.hash(parent_id, s.right_idx(parent_id, s.node_id(parent_id, 0))));
			boost::hash_combine(right_hash_, 0);

			/* update probas */
			/*PROBA_TYPE temp = real;
			real = temp*do_real + imag*do_imag;
			imag = temp*do_imag - imag*do_real;
		}

		/* check for last merge */
		bool last_merge = s.operation(parent_id, node_size - 1) == merge_t;

		if (last_merge) {
			unsigned int child_id_ = child_id;
			for (unsigned short int node = 0; node < node_size - 1; ++node)
				if (s.operation(parent_id, node) != none_t)
					child_id_ >>= 1;

			if (child_id_ == 0)
				last_merge = false;
		}

		printf("parent id:%d, child id:%d, first split:%d, last merge:%d\n", parent_id, child_id, first_split_overflow, last_merge);

		if (first_split_overflow) {
			/* update child id */
			child_id >>= 1;

			printf("		(first-split-0)\n");
			/*printf("		combine hash_ = (%ld,%ld)\n", hash_, s.hash(parent_id, s.right_idx(parent_id, s.node_id(parent_id, 0))));
			printf("		combine right_hash_ = (%ld,%d)\n", right_hash_, 0);

			/* update hashes */
			boost::hash_combine(hash_, s.hash(parent_id, s.right_idx(parent_id, s.node_id(parent_id, 0))));
			boost::hash_combine(right_hash_, 0);

			/* update probas */
			PROBA_TYPE temp = real;
			real = temp*do_real + imag*do_imag;
			imag = temp*do_imag - imag*do_real;
		}

		/* do last merge */
		if (last_merge) {

			/*printf("		combine left_hash_ = (%ld,%d)\n", left_hash_, 0);
			printf("		combine right_hash_ = (%ld,%d)\n", right_hash_, 0);
			
			/* update hashses */
			boost::hash_combine(left_hash_, 0);
			boost::hash_combine(right_hash_, 0);

			unsigned short int node_id = s.node_id(parent_id, node_size - 1);
			unsigned short int next_node_id = s.node_id(parent_id, 0);

			if (s.node_type(parent_id, node_id) == state_t::left_t &&
				s.node_type(parent_id, next_node_id) == state_t::right_t &&
				s.hash(parent_id, s.left_idx(parent_id, node_id)) == s.hash(parent_id, s.left_idx(parent_id, next_node_id))) {
					/* update node hash */

					printf("		(last-merge-0)\n");
					//printf("		combine hash_ = (%ld,%ld)\n", hash_, s.hash(parent_id, s.left_idx(parent_id, node_id)));

					boost::hash_combine(hash_, s.hash(parent_id, s.left_idx(parent_id, node_id)));

			} else {
				/* update sub node size */
				++num_sub_node;

				printf("		(last-merge-1)\n");
				//printf("		combine hash_ = (%ld,%ld)\n", hash_, s.hash_node_by_value(parent_id, -node_id - 1, next_node_id));

				/* update node hash */
				boost::hash_combine(hash_, s.hash_node_by_value(parent_id, -node_id - 1, next_node_id));
			}

			/* update probas */
			PROBA_TYPE temp = real;
			real = temp*do_real - imag*do_imag;
			imag = temp*do_imag + imag*do_real;
		}

		short int displacement = 0;

		unsigned short int num_nodes_ = node_size - last_merge;
		for (unsigned short int node = first_split_overflow + last_merge; node < num_nodes_; ++node) {

			unsigned short int node_id = s.node_id(parent_id, node);
			auto operation = s.operation(parent_id, node);
			
			if (operation != none_t) {
				bool do_ = child_id & 1;
				child_id >>= 1;

				printf("	node:%d, disp.:%d operation:%d, do:%d\n", node, displacement, operation, do_);

				if (operation == split_t) {
					if (do_) {

						/*printf("		combine (split) left_hash_ = (%ld,%d)\n", left_hash_, node + displacement);
						printf("		combine (split) right_hash_ = (%ld,%d)\n", right_hash_, node + displacement + 1);

						/* update left hash */
						boost::hash_combine(left_hash_, node + displacement);

						/* update displacement */
						++displacement;

						/* update right hash */
						boost::hash_combine(right_hash_, node + displacement);

						if (s.node_type(parent_id, node_id) == state_t::pair_t) {
							
							printf("		(split-0)\n");
							/*printf("		combine (split-0) hash_ = (%ld,%ld)\n", hash_, s.hash(parent_id, s.left_idx(parent_id, node_id)));
							printf("		combine (split-0) hash_ = (%ld,%ld)\n", hash_, s.hash(parent_id, s.right_idx(parent_id, node_id)));

							/* update node hash */
							boost::hash_combine(hash_, s.hash(parent_id, s.left_idx(parent_id, node_id)));
							boost::hash_combine(hash_, s.hash(parent_id, s.right_idx(parent_id, node_id)));

						} else {

							printf("		(split-1)\n");
							/*printf("		combine (split-1) hash_ = (%ld,%ld)\n", hash_, s.hash_node_by_value(parent_id,
								node == 0 ? -node_id - 1 : node_id + 1,
								state::left_t));
							printf("		combine (split-1) hash_ = (%ld,%ld)\n", hash_, s.hash_node_by_value(parent_id, node_id + 1, state::right_t));

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

						printf("		(non-split)\n");
						/*printf("		combine (non-split) left_hash_ = (%ld,%d)\n", left_hash_, node + displacement);
						printf("		combine (non-split) right_hash_ = (%ld,%d)\n", right_hash_, node + displacement);
						printf("		combine (non-split) hash_ = (%ld,%ld)\n", hash_, s.hash(parent_id, node_id));

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

						/*printf("		combine (merge) left_hash_ = (%ld,%d)\n", left_hash_, node + displacement);
						printf("		combine (merge) right_hash_ = (%ld,%d)\n", right_hash_, node + displacement);*/

						/* update hashes */
						boost::hash_combine(left_hash_, node + displacement);
						boost::hash_combine(right_hash_, node + displacement);

						unsigned short int next_node = node == num_nodes_ - 1 ? 0 : node + 1;
						unsigned short int next_node_id = s.node_id(parent_id, next_node);

						if (s.node_type(parent_id, node_id) == state_t::left_t &&
						s.node_type(parent_id, next_node_id) == state_t::right_t &&
						s.hash(parent_id, s.left_idx(parent_id, node_id)) == s.hash(parent_id, s.left_idx(parent_id, next_node_id))) {
							/* update node hash */
							boost::hash_combine(hash_, s.hash(parent_id, s.left_idx(parent_id, node_id)));

							printf("		(merge-0)\n");

						} else {
							printf("		(merge-1)\n");

							/* update sub node size */
							++num_sub_node;

							/* update node hash */
							boost::hash_combine(hash_, s.hash_node_by_value(parent_id, 
								node == 0 ? -node_id - 1 : node_id + 1,
								next_node_id));
						}

						/* update probas */
						PROBA_TYPE temp = real;
						real = temp*do_real - imag*do_imag;
						imag = temp*do_imag + imag*do_real;

						/* skip next node */
						++node;

						/* update displacement */
						--displacement;
					} else {

						printf("		(non-merge)\n");
						/*printf("		combine (non-merge) left_hash_ = (%ld,%d)\n", left_hash_, node + displacement);
						printf("		combine (non-merge) hash_ = (%ld,%ld)\n", hash_,  s.hash(parent_id, node_id));

						/* update probas */
						real *= do_not;
						imag *= do_not;

						/* update hashes */
						boost::hash_combine(hash_, s.hash(parent_id, node_id));
						boost::hash_combine(left_hash_, node + displacement);
					}
			} else {

				printf("	node:%d, disp.:%d operation:%d, do:0\n", node, displacement, operation);

				/*if (s.left(parent_id, node))
					printf("		combine left_hash_ = (%ld,%d)\n", left_hash_, node + displacement);
				if (s.right(parent_id, node))
					printf("		combine right_hash_ = (%ld,%d)\n", right_hash_, node + displacement);
				printf("		combine hash_ = (%ld,%ld)\n", hash_,  s.hash(parent_id, node_id));

				/* update hashes */
				boost::hash_combine(hash_, s.hash(parent_id, node_id));

				if (s.left(parent_id, node))
					boost::hash_combine(left_hash_, node + displacement);

				if (s.right(parent_id, node))
					boost::hash_combine(right_hash_, node + displacement);
			}
		}

		/* update node_size */
		node_size += displacement + first_split_overflow - last_merge;

		if (first_split_overflow) {

			printf("		combine left_hash_ = (%ld,%d)\n", left_hash_, node_size - 1);
			printf("		combine hash_ = (%ld,%ld)\n", hash_, s.hash(parent_id, s.left_idx(parent_id, s.node_id(parent_id, 0))));

			/* update hashes */
			boost::hash_combine(hash_, s.hash(parent_id, s.left_idx(parent_id, s.node_id(parent_id, 0))));
			boost::hash_combine(left_hash_, node_size - 1);
		}

		printf("	hash_=%ld, right_hash_=%ld, left_hash_=%ld\n", hash_, right_hash_, left_hash_);

		boost::hash_combine(hash_, left_hash_);
		boost::hash_combine(hash_, right_hash_);

		return {hash_, real, imag, node_size, std::max(num_sub_node, s.num_sub_node(parent_id))};
	}

	void populate_new_graph(state_t const &s, state_t &new_state, unsigned int gid, unsigned int parent_id, unsigned int child_id) const override {
		/* copy nodes */
		unsigned int sub_node_begin = s.sub_node_begin[parent_id];
		unsigned int sub_node_end = s.sub_node_begin[parent_id + 1];
		unsigned int new_sub_node_begin = new_state.sub_node_begin[gid];

		for (unsigned int j = sub_node_begin; j < sub_node_end; ++j) {
			std::string type;

			auto node_type = s.node_type(parent_id, j - sub_node_begin);

			if (s.is_trash(parent_id, j - sub_node_begin)) {
				type = "x";
			} else
				switch (node_type) {
					case state_t::left_t:
						type = "l";
						break;

					case state_t::right_t:
						type = "r";
						break;

					case state_t::element_t:
						type = "e";
						break;

					case state_t::pair_t:
						type = "p";
						break;

					default:
						type = "!";
						break;
				}

			printf("(l:%d, r:%d, t:%s), ", s.left_idx__or_element__and_has_most_left_zero__or_is_trash_[j],
				s.right_idx__or_type_[j],
				type.c_str());
		}

		printf("\ngid:%d, parent id:%d, child id:%d, num nodes:%d, old num nodes:%d, num sub nodes:%d, old num sub nodes:%d\n",
			gid, parent_id, child_id, s.num_nodes(parent_id), new_state.num_nodes(gid), new_state.num_sub_node(gid), s.num_sub_node(parent_id));
		printf("--------------------child property:\n"); child_properties(s, parent_id, child_id);
		printf("--------------------populate:\n");

		/* copy nodes */
		std::copy(s.left_idx__or_element__and_has_most_left_zero__or_is_trash_.begin() + sub_node_begin, s.left_idx__or_element__and_has_most_left_zero__or_is_trash_.begin() + sub_node_end, new_state.left_idx__or_element__and_has_most_left_zero__or_is_trash_.begin() + new_sub_node_begin);
		std::copy(s.right_idx__or_type_.begin() + sub_node_begin, s.right_idx__or_type_.begin() + sub_node_end, new_state.right_idx__or_type_.begin() + new_sub_node_begin);
		std::copy(s.node_hash.begin() + sub_node_begin, s.node_hash.begin() + sub_node_end, new_state.node_hash.begin() + new_sub_node_begin);

		/* get trash */
		short int old_sub_node_num = s.num_sub_node(parent_id);
		short int last_trash_idx = -1;
		auto get_trash = [&]() {
			++last_trash_idx;

			/* check for old trash sub-nodes */
			for (; last_trash_idx < old_sub_node_num; ++last_trash_idx)
				if (new_state.is_trash(gid, last_trash_idx))
					return last_trash_idx;

			if (last_trash_idx >= new_state.num_sub_node(gid))
				throw;
			
			/* return newly allocated sub-nodes */
			return last_trash_idx;
		};

		auto set_trash = [&](short int sub_node) {
			new_state.set_is_trash(gid, sub_node);
			last_trash_idx = std::min((short int)(sub_node - 1), last_trash_idx);
		};

		short int displacement = 0;

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

			printf("	node:%d, disp.:%d operation:%d, do:%d\n", node, displacement, operation, do_);

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
						printf("		split-0, node id:%d\n", node_id);

						new_state.set_node_id(gid, node + displacement - 1, s.left_idx(parent_id, node_id));
						new_state.set_node_id(gid, node + displacement, s.right_idx(parent_id, node_id));

						/* set trash */
						set_trash(node_id);
					} else {

						printf("		split-1, ");

						/* left node */
						auto new_left_node_idx = get_trash();
						new_state.set_node_id(gid, node + displacement - 1, new_left_node_idx);
						new_state.set_left_idx(gid, new_left_node_idx, node_id);
						new_state.set_type(gid, new_left_node_idx, state::left_t);
						/* set has_most_left_zero */
						if (node == 0)
							new_state.set_has_most_left_zero(gid, new_left_node_idx, true);

						/* right node */
						auto new_right_node_idx = get_trash();
						new_state.set_node_id(gid, node + displacement, new_right_node_idx);
						new_state.set_left_idx(gid, new_right_node_idx, node_id);
						new_state.set_type(gid, new_right_node_idx, state::right_t);

						printf("node id:%d, left node id:%d, right node id:%d\n", node_id, new_left_node_idx, new_right_node_idx);

						/* re-hash nodes */
						new_state.hash_node(gid, new_left_node_idx);
						new_state.hash_node(gid, new_right_node_idx);
					}
				} else {
					/* check for last merge */
					if (node == num_nodes_ - 1)
						displacement = -node;

					/* set node values */
					new_state.set_left(gid, node + displacement, true);
					new_state.set_right(gid, node + displacement, true);

					/* merge node */
					unsigned short int next_node = node == num_nodes_ - 1 ? 0 : node + 1;
					unsigned short int next_node_id = s.node_id(parent_id, next_node);

					printf("		merge node:%d, displacement:%d, node id:%d, next node id:%d\n", node, displacement, node_id, next_node_id);

					printf("-\n");

					if (s.node_type(parent_id, node_id) == state_t::left_t &&
					s.node_type(parent_id, next_node_id) == state_t::right_t &&
					/*s.hash(parent_id, */s.left_idx(parent_id, node_id)/*)*/ == /*s.hash(parent_id, */s.left_idx(parent_id, next_node_id))/*)*/ {
						printf("		merge-0, node id:%d, next node id:%d\n", node_id, next_node_id);

						new_state.set_node_id(gid, node + displacement, s.left_idx(parent_id, node_id));

						/* set trash */
						set_trash(node_id);
						set_trash(next_node_id);
					} else {

						printf("		merge-1, ");

						/*printf("left hash:%ld, right hash:%ld\n",
							s.hash(parent_id, s.left_idx(parent_id, node_id)),
							s.hash(parent_id, s.left_idx(parent_id, next_node_id)));*/
						printf("left idx:%d, right idx:%d\n",
							s.left_idx(parent_id, node_id),
							s.left_idx(parent_id, next_node_id));

						/* new node */
						auto new_node_idx = get_trash();
						new_state.set_node_id(gid, node + displacement, new_node_idx);
						new_state.set_left_idx(gid, new_node_idx, node_id);
						new_state.set_right_idx(gid, new_node_idx, next_node_id);

						printf("new node id:%d\n", new_node_idx);

						/* set has_most_left_zero */
						if (node == 0 || node == num_nodes_ - 1)
							new_state.set_has_most_left_zero(gid, new_node_idx, true);

						/* re-hash nodes */
						new_state.hash_node(gid, new_node_idx);
					}

					/* update displacement */
					--displacement;

					/* skip next node */
					++node;
				}
			} else {
				new_state.set_node_id(gid, node + displacement, node_id);
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