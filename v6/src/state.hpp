#pragma once

#include <vector>
#include <parallel/algorithm>
#include <parallel/numeric>
#include <boost/functional/hash.hpp>
#include <random>
#include <iostream>

#ifdef USE_MPRF
	//import
	#include "utils/mpreal.h"
	
	// namespace for math functions
	namespace precision = mpfr;

	//type
	#define PROBA_TYPE precision::mpreal

	// precision setter
	#define SET_PRECISION(precision_) PROBA_TYPE::set_default_prec(precision_);
#else
	// standard type
	#define PROBA_TYPE long double

	// precision setter
	#define SET_PRECISION(precision)

	// namespace for math functions
	namespace precision = std;
#endif

// type definition
typedef class state state_t;
typedef class rule rule_t;
typedef char op_type_t;

// global variable definition
PROBA_TYPE tolerance = 0;
float resize_policy = 1.3;

// rule interface definition
class rule {
public:
	rule() {}

	virtual op_type_t operation(state_t const &s, unsigned int gid, unsigned short int node) const { return '\0'; } /* step (1) */

	virtual unsigned short int numb_childs(state_t const &s, unsigned int gid) const { return 0; } /* step (2) */

	virtual std::tuple<size_t /* hash */,
		PROBA_TYPE /* real*/, PROBA_TYPE /* iamg */,
		unsigned short int /* numb_nodes */, unsigned short int /* numb sub-nodes */> 
		child_properties(state_t const &s, unsigned int parent_id, unsigned int child_id) const { return {0, 0., 0., 0, 0}; } /* step (4) */

	virtual void populate_new_graph(state_t const &s, state_t &new_state, unsigned int gid, unsigned int parent_id, unsigned int child_id) const {} /* step (8) */
};

/*
!!!!!
Iteration protocol is:
  - (1): use a rule to put all operation inside of the vector "operations"

  - (2): calculate the number of subgraphs generated by each graph and populate "num_childs" (*)

  - (3): read the total number of sub_graph generated by each graph and populate "new_gid", "parent_gid" and "parent_sub_id"

  - (4): by iterating on "new_gid" and using the rule, populate "new_hash" and "new_real" and "new_imag" through a simbolic iteration (**),
  	and populate "new_size_b" and "new_size_c" which are respectively the number of node and of sub-nodes for each new graph

  - (5): using "new_hash" sum the probability ("new_real" and "new_imag") of equal graphs, while setting the probability of graph which are equal to 0

  - (6): sort "new_gid" based on probability ("new_real" and "new_imag")

  - (7): reserve all public vector for a new state by iterating over the N first "new_gid" andusing "parent_gid" and "parent_sub_id",
  	and assign "b_begin" and "c_begin".

  - (8): generate all new graphs of the new state by iterating over the N first "new_gid" andusing "parent_gid" and "parent_sub_id".


(*) to compute the number of child you just compute "pow(2, numb_operations)""
(**) it's possible to compute the hash of a child graph without actually computing it for most rules
*/

class state {
public:
	// type definition of a node
	typedef enum node_type {
		left_t = -3,
		right_t,
		element_t,
		pair_t,
	} node_type_t;

	/* empty constructor */
	state() {}

	/* constructor for a single graph of a given size */
	state(unsigned int size) : numb_graphs(1) {
		real = {1};
		imag = {0};

		b_begin = {0, size};
		c_begin = {0, size};

		left_.resize(size, false);
		right_.resize(size, false);
		nid.resize(size);
		left_idx__or_element__and_has_most_left_zero_.resize(size);
		right_idx__or_type_.resize(size, element_t);
		node_hash.resize(size);
		operations.resize(size);
		num_childs.resize(size);

		std::iota(nid.begin(), nid.end(), 0);
		std::iota(left_idx__or_element__and_has_most_left_zero_.begin(), left_idx__or_element__and_has_most_left_zero_.end(), 1);
		left_idx__or_element__and_has_most_left_zero_[0] = -1;

		for (unsigned int i = 0; i < size; ++i) {
			node_hash[i] = i;
			boost::hash_combine(node_hash[i], (short int)element_t);
		}
	}

	/* constructor for a temp state */
	state(size_t size_a_next, size_t size_a, size_t size_b, size_t size_c) {
		resize_a_next(size_a_next);
		resize_a(size_a);
		resize_b(size_b);
		resize_c(size_c);
	}

	/* vectors can have 3 different sizes :
		- (a) the number of graph (so a single element per graph), and + 1
		- (b) the total number of nodes (sum of the number of nodes for each graph)
		- (c) the total number of nodes (including sub-nodes used to generate node tree)
	*/

	size_t numb_graphs = 0;

	// graph magnitude
	std::vector<PROBA_TYPE> real; /* size (a) */
	std::vector<PROBA_TYPE> imag; /* size (a) */

	// begin for each size
	std::vector<unsigned int> b_begin; /* size (a + 1), refers to vector of size (b) */
	std::vector <unsigned int> c_begin; /* size (a + 1), refers to vectors of size (c) */

	// graph properties
	std::vector<bool> left_; /* size (b) */
	std::vector<bool> right_; /* size (b) */
	std::vector<short int> nid; /* size (b) */

	// node properties
	std::vector<short int> left_idx__or_element__and_has_most_left_zero_; /* size (c) */
	std::vector<short int> right_idx__or_type_; /* size (c) */
	std::vector<size_t> node_hash; /* size (c) */

	/*
	intermediary vectors used to generate the next state :
	*/

	// vector of operations
	std::vector<op_type_t> operations; /* size (b) */

	// number of sub-graph for each parent
	std::vector<unsigned short int> num_childs; /* size (a) */

	// new graphs
	std::vector<unsigned int> new_gid; /* size (a) for the next iteration */
	std::vector<unsigned int> parent_gid; /* size (a) for the next iteration */
	std::vector<unsigned int> parent_sub_id; /* size (a) for the next iteration */

	// new graph hash
	std::vector<size_t> new_hash; /* size (a) for the next iteration */
	std::vector<unsigned short int> new_size_b; /* size (a) for the next iteration */
	std::vector<unsigned short int> new_size_c; /* size (a) for the next iteration */

	// new graph magnitude
	std::vector<PROBA_TYPE> new_real; /* size (a) for the next iteration */
	std::vector<PROBA_TYPE> new_imag; /* size (a) for the next iteration */

	// resize operators
	void resize_a_next(size_t size) {
		if (new_gid.size() < size) {
			new_gid.resize(resize_policy * size);
			parent_gid.resize(resize_policy * size);
			parent_sub_id.resize(resize_policy * size);
			new_hash.resize(resize_policy * size);
			new_real.resize(resize_policy * size);
			new_imag.resize(resize_policy * size);
			new_size_c.resize(resize_policy * size);
			new_size_b.resize(resize_policy * size);
		}

		std::iota(new_gid.begin(), new_gid.begin() + size, 0);
	}

	void resize_a(size_t size) {
		if (real.size() < size) {
			real.resize(resize_policy * size);
			imag.resize(resize_policy * size);
			b_begin.resize(resize_policy * size + 1);
			c_begin.resize(resize_policy * size + 1);
		}
	}

	void resize_b(size_t size) {
		if (left_.size() < size) {
			left_.resize(resize_policy * size);
			right_.resize(resize_policy * size);
			nid.resize(resize_policy * size);
			operations.resize(resize_policy * size);
			num_childs.resize(resize_policy * size);
		}
	}

	void resize_c(size_t size) {
		if (node_hash.size() < size) {
			left_idx__or_element__and_has_most_left_zero_.resize(resize_policy * size);
			right_idx__or_type_.resize(resize_policy * size);
			node_hash.resize(resize_policy * size);
		}
	}

	/* hashers */
	size_t inline hash_node_by_value(unsigned int gid, short int left_idx__or_element__and_has_most_left_zero, short int right_idx__or_type) const {
		size_t hash_ = 0;
		unsigned int left_idx = std::abs(left_idx__or_element__and_has_most_left_zero) - 1;

		if (right_idx__or_type == element_t) {
			hash_ = left_idx;
		} else
			hash_ = hash(gid, left_idx);

		if (right_idx__or_type < 0) {
			boost::hash_combine(hash_, right_idx__or_type);
		} else
			boost::hash_combine(hash_, hash(gid, right_idx__or_type));

		return hash_;
	}
	void inline hash_node(unsigned int gid, unsigned int node) {
		unsigned int id = c_begin[gid] + node;
		node_hash[id] = hash_node_by_value(gid, left_idx__or_element__and_has_most_left_zero_[id], right_idx__or_type_[id]);
	}

	/* getters */
	unsigned short int inline numb_nodes(unsigned int gid) const { return b_begin[gid + 1] - b_begin[gid]; }
	unsigned short int inline c_size(unsigned int gid) const { return c_begin[gid + 1] - c_begin[gid]; }

	unsigned short int inline node_id(unsigned int gid, unsigned int node) const { return nid[b_begin[gid] + node]; }
	bool inline left(unsigned int gid, unsigned int node) const { return left_[b_begin[gid] + node]; }
	bool inline right(unsigned int gid, unsigned int node) const { return right_[b_begin[gid] + node]; }

	size_t hash(unsigned int gid, unsigned int node) const { return node_hash[c_begin[gid] + node]; }
	short int inline right_idx(unsigned int gid, unsigned int node) const { return right_idx__or_type_[c_begin[gid] + node]; }
	short int inline &right_idx(unsigned int gid, unsigned int node) { return right_idx__or_type_[c_begin[gid] + node]; }
	bool inline has_most_left_zero(unsigned int gid, unsigned int node) const {
		return left_idx__or_element__and_has_most_left_zero_[c_begin[gid] + node] < 0;
	}
	unsigned short int inline left_idx(unsigned int gid, unsigned int node) const { 
		return std::abs(left_idx__or_element__and_has_most_left_zero_[c_begin[gid] + node]) - 1;
	}
	unsigned short int inline element(unsigned int gid, unsigned int node) const { return left_idx(gid, node); }
	node_type_t inline node_type(unsigned int gid, unsigned int node) const {
		return std::min((node_type_t)right_idx__or_type_[c_begin[gid] + node], pair_t);
	}

	/* setters */
	void inline set_node_id(unsigned int gid, unsigned int node, unsigned short int value) { nid[b_begin[gid] + node] = value; }
	void inline set_left(unsigned int gid, unsigned int node, bool value) { left_[b_begin[gid] + node] = value; }
	void inline set_right(unsigned int gid, unsigned int node, unsigned short int value) { right_[b_begin[gid] + node] = value; }

	void inline set_right_idx(unsigned int gid, unsigned int node, unsigned short int value) { right_idx__or_type_[c_begin[gid] + node] = value; }
	void inline set_type(unsigned int gid, unsigned int node, node_type_t value) { set_right_idx(gid, node, value); }
	void inline set_left_idx(unsigned int gid, unsigned int node, unsigned short int value) {
		if (has_most_left_zero(gid, node)) {
			left_idx__or_element__and_has_most_left_zero_[c_begin[gid] + node] = -value - 1;
		} else
			left_idx__or_element__and_has_most_left_zero_[c_begin[gid] + node] = value + 1;
	}
	void inline set_element(unsigned int gid, unsigned int node, unsigned short int value) { set_left_idx(gid, node, value); }
	void inline set_most_left_zero(unsigned int gid, unsigned int node, bool has_most_left_zero_) {
		short int &temp = left_idx__or_element__and_has_most_left_zero_[c_begin[gid] + node];

		if (has_most_left_zero_) {
			temp = -std::abs(temp);
		} else
			temp = std::abs(temp);
	}
	

	/* step function */
	void step(state_t &new_state, rule_t &rule, unsigned int n_graphs) {
		size_t total_num_graphs = 0;

		#pragma omp parallel
		#pragma omp master
		{

			/* !!!!!!!!!!!!!!!!
			step (1) 
			 !!!!!!!!!!!!!!!! */

			std::cout << "step 1\n";

			#pragma omp parallel for
			for (unsigned int gid = 0; gid < numb_graphs; ++gid) {
				auto begin = b_begin[gid];
				auto numb_nodes_ = numb_nodes(gid);

				/* get operations for each nodes of each graph */
				for (unsigned int nid = 0; nid < numb_nodes_; ++nid)
					operations[begin + nid] = rule.operation(*this, gid, nid);
			}

			/* !!!!!!!!!!!!!!!!
			step (2) 
			 !!!!!!!!!!!!!!!! */

			std::cout << "step 2\n";

			#pragma omp parallel for
			for (unsigned int gid = 0; gid < numb_graphs; ++gid)
				/* get the number of child for each graph */
				num_childs[gid] = rule.numb_childs(*this, gid);

			/* !!!!!!!!!!!!!!!!
			step (3) 
			 !!!!!!!!!!!!!!!! */
			
			std::cout << "step 3\n";

			/* compute the total number of child in parallel */
			#pragma omp parallel for reduction(+:total_num_graphs)
			for (auto &num_graph : num_childs)
				total_num_graphs += num_graph;

			/* resize variables with the right_ number of elements */
			resize_a_next(total_num_graphs);

			unsigned int i = 0;
			for (unsigned int gid = 0; gid < numb_graphs; ++gid) {
				unsigned int const num_child = num_childs[gid];

				/* assign parent ids and child ids for each child */
				for (unsigned int child_id = 0; child_id < num_child; ++child_id) {
					parent_gid[i] = gid;
					parent_sub_id[i] = child_id;
					++i;
				}
			}

			/* !!!!!!!!!!!!!!!!
			step (4) 
			 !!!!!!!!!!!!!!!! */

			std::cout << "step 4\n";

			#pragma omp parallel for
			for (unsigned int gid = 0; gid < total_num_graphs; ++gid) {
				auto [hash_, real_, imag_, size_b_, size_c_] = rule.child_properties(*this, parent_gid[gid], parent_sub_id[gid]);

				/* assign propreties for each child */
				new_hash[gid] = hash_;
				new_real[gid] = real_;
				new_imag[gid] = imag_;
				new_size_b[gid] = size_b_;
				new_size_c[gid] = size_c_;
			}

			/* !!!!!!!!!!!!!!!!
			step (5) 
			 !!!!!!!!!!!!!!!! */

			std::cout << "step 5\n";

			/* sort graphs hash to compute interference
			!! TODO: implement it in parallel (https://cw.fel.cvut.cz/old/_media/courses/b4m35pag/lab6_slides_advanced_openmp.pdf) !! */
			std::sort(new_gid.begin(), new_gid.begin() + total_num_graphs, [&](unsigned int const &gid1, unsigned int const &gid2) {
				return new_hash[gid1] > new_hash[gid2];
			});

			/* compute is_first_index */
			std::vector<bool> is_first_index(total_num_graphs);
			is_first_index[new_gid[0]] = true;

			#pragma omp parallel for
			for (unsigned int gid = 1; gid < total_num_graphs; ++gid)
				is_first_index[new_gid[gid]] = new_hash[new_gid[gid]] != new_hash[new_gid[gid - 1]];

			#pragma omp parallel for
			for (unsigned int gid = 0; gid < total_num_graphs; ++gid)
			  if (is_first_index[new_gid[gid]])
					/* sum magnitude of equal graphs */
					for (unsigned int gid_ = gid + 1; gid_ < total_num_graphs && !is_first_index[new_gid[gid_]]; ++gid_) {
						new_real[new_gid[gid]] += new_real[new_gid[gid_]];
						new_imag[new_gid[gid]] += new_imag[new_gid[gid_]];
					}

			/* get all graphs with a non zero probability */
			auto partitioned_it = __gnu_parallel::partition(new_gid.begin(), new_gid.begin() + total_num_graphs, [&](unsigned int const &gid) {
				/* check for duplicates */
				if (!is_first_index[gid])
					return false;

				/* check for zero probability */
				auto r = new_real[gid];
				auto i = new_imag[gid];

				return r*r + i*i > tolerance; 
			});

			new_state.numb_graphs = std::distance(new_gid.begin(), partitioned_it);

			/* !!!!!!!!!!!!!!!!
			step (6) 
			 !!!!!!!!!!!!!!!! */

			std::cout << "step 6\n";

			if (n_graphs > 0 && new_state.numb_graphs > n_graphs) {
				/* sort graphs according to probability */
				__gnu_parallel::nth_element(new_gid.begin(), new_gid.begin() + n_graphs, new_gid.begin() + new_state.numb_graphs,
				[&](unsigned int const &gid1, unsigned int const &gid2) {
					auto r1 = new_real[gid1];
					auto i1 = new_imag[gid1];

					auto r2 = new_real[gid2];
					auto i2 = new_imag[gid2];

					return r1*r1 + i1*i1 > r2*r2 + i2*i2;
				});

				new_state.numb_graphs = n_graphs;
			}

			/* !!!!!!!!!!!!!!!!
			step (7) 
			 !!!!!!!!!!!!!!!! */

			std::cout << "step 7\n";

			/* resize new step variables */
			new_state.resize_a(new_state.numb_graphs);

			/* compute size (b) and (c) ... */
			size_t size_b = 0;
			size_t size_c = 0;
			for (unsigned int gid = 0; gid < new_state.numb_graphs; ++gid) {
				/* ...and assign "b_begin" and "c_begin" */
				new_state.b_begin[gid] = size_b;
				new_state.c_begin[gid] = size_c;

				auto id = new_gid[gid];

				new_state.real[gid] = new_real[id];
				new_state.imag[gid] = new_imag[id];

				size_b += new_size_b[id];
				size_c += new_size_c[id];
			}

			new_state.b_begin[new_state.numb_graphs] = size_b;
			new_state.c_begin[new_state.numb_graphs] = size_c;

			/* resize new step variables */
			new_state.resize_b(size_b);
			new_state.resize_c(size_c);

			/* !!!!!!!!!!!!!!!!
			step (8) 
			 !!!!!!!!!!!!!!!! */

			std::cout << "step 8\n";

			//#pragma omp parallel for
			for (unsigned int gid = 0; gid < new_state.numb_graphs; ++gid) {
				auto id = new_gid[gid];

				/* populate graphs */
				rule.populate_new_graph(*this, new_state, gid, parent_gid[id], parent_sub_id[id]);
			}
		}
	}

	void randomize() {
		// random genearator
		size_t size = left_.size();
    	for (int i = 0; i < size; ++i) {
    		left_[i] = std::rand() % 2;
    		right_[i] = std::rand() % 2;
    	}
	}
};

void print(state_t &s) {
	std::function<void(unsigned int, unsigned short int, bool)> const print_node = [&] (unsigned int gid, unsigned short int node, bool parenthesis) {
		switch (s.node_type(gid, node)) {
			case state_t::left_t:
				print_node(gid, s.left_idx(gid, node), true);
				std::cout << ".l";
				break;

			case state_t::right_t:
				print_node(gid, s.left_idx(gid, node), true);
				std::cout << ".r";
				break;
			
			case state_t::element_t:
				std::cout << s.element(gid, node);
				break;

			case state_t::pair_t:
				if (parenthesis)
					std::cout << "(";

				print_node(gid, s.left_idx(gid, node), true);

				std::cout << "∧";

				print_node(gid, s.right_idx(gid, node), true);

				if (parenthesis)
					std::cout << ")";

				break;

			default:
				throw;
		}
	};

	for (int gid = 0; gid < s.numb_graphs; ++gid) {
		std::cout << s.real[gid] << (s.imag[gid] >= 0 ? "+" : "") << s.imag[gid] << "i   ";

		unsigned int numb_nodes_ = s.numb_nodes(gid);
		for (unsigned int node = 0; node < numb_nodes_; ++node) {
			std::cout << "-|" << (s.left(gid, node) ? "<" : " ") << "|";

			print_node(gid, s.node_id(gid, node), false);

			std::cout << "|" << (s.right(gid, node) ? ">" : " ") << "|-";
		}

		std::cout << "\n";
	}
}