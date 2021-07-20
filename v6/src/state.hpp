#pragma once

#include <vector>
#include <parallel/algorithm>
#include <parallel/numeric>
#include <boost/functional/hash.hpp>
#include <random>
#include <iostream>

#include "utils/allocator.hpp"
#include "utils/memory.hpp"
#include "utils/vector.hpp"
#include "utils/complex.hpp"

// debug levels
#define STEP_DEBUG_LEVEL 1
#define PRINT_DEBUG_LEVEL_1 0.5
#define PRINT_DEBUG_LEVEL_2 0.75
#define PRINT_DEBUG_LEVEL_3 1.5

#ifdef USE_MPRF
	// import
	#include "utils/mpreal.h"
	
	// namespace for math functions
	namespace precision = mpfr;

	// type
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

#ifndef _OPENMP
	#define omp_set_nested(i)
#endif

// type definition
typedef class state state_t;
typedef char op_type_t;

// global variable definition
PROBA_TYPE tolerance = 0;
float upsize_policy = 1.1;
float downsize_policy = 0.9;
float safety_margin = 0.2;
size_t min_state_size = 100000;

// debugging options
float verbose = 0;
int max_num_graph_print = 20;

/*
!!!!!
What are called "sub-nodes" are nodes containers that are used to point to other node container (to generate pairs, left or right nodes).

Iteration protocol is:
  - (1): use a rule to put all operation inside of the vector "operations"

  - (2): calculate the numer of child graph generated by each parent graph and populate "num_childs" (*)

  - (2.1): normalize the old state here if wanted, since we need to sum over all "num_childs",
  	we can in the same reduction sum over all probabilities.

  - (3): read the total numer of child graph generated by each parent graph and populate "next_gid", "symbolic_iteration.parent_gid" and "symbolic_iteration.child_id"

  - (4): by iterating on "next_gid" and using the rule, populate "next_hash" and "next_real" and "next_imag" through a symbolic iteration (**),
  	and populate "node_begin" and "sub_node_begin" which are respectively the numer of node and of sub-nodes for each new graph

  - (5): using "next_hash" sum their probability ("next_real" and "next_imag") of equal graphs, while setting the probability of graph which are equal to 0.
  	when sorting graphs according to "next_hash" to sum their probability, we also sort accroding to "num_childs" so the graphs with the smallest number of child
  	are the one for which we end up computing the dynamic since they also tend to be simpler to compute (since they have less interactions).  
	
	- (6): compute the average memory usage of a graph.
  	We can then compute "max_num_graphs" according to the value of "get_free_mem_size()" and a "safety_factor".

  - (6.1): partition "next_gid" based on probability ("next_real" and "next_imag"), and keep only the "max_num_graphs" most probable graph

  - (7): reserve all public vector for a new state by iterating over the N first "next_gid" andusing "symbolic_iteration.parent_gid" and "symbolic_iteration.child_id",
  	and assign "node_begin" and "sub_node_begin".

  - (8): generate all new graphs of the new state by iterating over the N first "next_gid" andusing "symbolic_iteration.parent_gid" and "symbolic_iteration.child_id".

	- (9): swap the previous iteration with the "buffer_state".

(*) to compute the numer of child you just compute "pow(2, num_operations)""
(**) it's possible to compute the hash of a child graph without actually computing it for most rules
*/

class state {
public:
/* 
rule virtual class definition:
	- A rule can be "probabilist" or "quantum".
	- For each rule, the "none" operation has to be represented by 0
	- It has virtual memeber function that needs to be overloaded by each individual rule.

Non-virtual member functions are:
	- Constructor (for both probabilist and quantum rules).
	- "multiply_proba(PROBA_TYPE &real, PROBA_TYPE &imag, op_type_t op, bool do_)" to mutliply a magnitude by the rule's matrix (according to the operation type).
	- "num_childs()" which is used at step 2 of the iteration.
	- "do_operation(unsigned short int &child_id)" that check if some operation should be done or not according to the "child_id", and the type of rule.
	- "write_operation(op_type_t op)" which apply the probabilist decision if the rule is probabilist, else returns op.
*/
	typedef class rule {
	public:
		/* parameters of a stochiastic matrix */
		PROBA_TYPE p = 1;
		PROBA_TYPE q = 1;
		bool probabilist = false;

		/* parameters of a unitary matrix */
		PROBA_TYPE do_real = 1;
		PROBA_TYPE do_imag = 0;
		PROBA_TYPE do_not_real = 0;
		PROBA_TYPE do_not_imag = 0;

		/* parameters for print */
		PROBA_TYPE teta, phi, xi;
		std::string name = "";
		bool move = true;
		unsigned int n_iter = 0;

		/* constructor for a unitary matrix */
		rule(PROBA_TYPE teta_, PROBA_TYPE phi_, PROBA_TYPE xi_) : teta(teta_), phi(phi_), xi(xi_) {
			do_real = precision::sin(teta)* precision::cos(phi);
			do_imag = precision::sin(teta)* precision::sin(phi);
			do_not_real = precision::cos(teta)* precision::cos(xi);
			do_not_imag = precision::cos(teta)* precision::sin(xi);
		}
		rule(PROBA_TYPE p_, PROBA_TYPE q_) : p(p_), q(q_), probabilist(true) {
			do_real = precision::sqrt(p);
			do_imag = precision::sqrt(q);
			do_not_real = precision::sqrt(1 - p);
			do_not_imag = precision::sqrt(1 - q);
		}
		rule(PROBA_TYPE p_) : p(p_), q(p_), probabilist(true) {
			do_real = precision::sqrt(p);
			do_imag = precision::sqrt(q);
			do_not_real = precision::sqrt(1 - p);
			do_not_imag = precision::sqrt(1 - q);
		}
		rule() {}

		/* getter */
		void inline multiply_proba(PROBA_TYPE &real, PROBA_TYPE &imag, op_type_t op /* either 0 or 1 */, bool do_) const {
			if (!probabilist) {

				/* quantum case */
				PROBA_TYPE sign = (PROBA_TYPE)(op*2 - 1);
				if (do_) {
					time_equal(real, imag, do_real, sign*do_imag);
				} else
					time_equal(real, imag, sign*do_not_real, do_not_imag);
			}
		}

		op_type_t inline write_operation(op_type_t op) const {
			if (!probabilist || op == 0)
				return op;
			
			/* generate random number */
			PROBA_TYPE r = static_cast<PROBA_TYPE> (rand()) / static_cast<PROBA_TYPE> (RAND_MAX);

			if (r < (op == 1 ? p : q))
				return op;

			return 0; 
		}

		bool inline do_operation(unsigned short int &child_id) const {
			/* check if the rule is classical */
			if (probabilist || (do_not_real == 0 && do_not_imag == 0))
				return true;

			/* check if the rule is the identity */
			if (do_real == 0 && do_imag == 0)
				return false;

			bool do_ = child_id & 1;
			child_id >>= 1;

			return do_;
		}

		/* step (1) */
		virtual op_type_t operation(state_t const &s, unsigned int gid, unsigned short int node) const { return '\0'; }

		/* step (2) */
		unsigned short int num_childs(state_t const &s, unsigned int gid) const {
			/* check for "classical case" */
			if (probabilist || (do_not_real == 0 && do_not_imag == 0) || (do_real == 0 && do_imag == 0))
				return 1;

			/* count operations */
			unsigned int num_op = 0;
			unsigned int num_nodes_ = s.num_nodes(gid);
			for (unsigned int node = 0; node < num_nodes_; ++node)
				num_op += s.operation(gid, node) != 0; // 0 is the "none" operation for all dynamics

			/* 2^n_op childs */
			return std::pow(2, num_op);
		}

		/* step (4) */
		virtual void child_properties(size_t& hash_,
			PROBA_TYPE& real, PROBA_TYPE& imag,
			unsigned int& num_nodes, unsigned int& num_sub_node,
			state_t const &s, unsigned int parent_id, unsigned short int child_id) const { }

		/* step (8) */
		virtual void populate_new_graph(state_t const &s, state_t &buffer_state, unsigned int next_gid, unsigned int parent_id, unsigned short int child_id) const {}
	} rule_t;

	/* !!!!!!!!!
	definition of the state class
	!!!!!!!!! */

	// type definition of a node
	typedef enum node_type {
		left_t = -3,
		right_t,
		element_t,
		pair_t,
	} node_type_t;

	/* empty constructor */
	state() {
		/* minimum size */
		resize_num_graphs(min_state_size);
		resize_num_nodes(min_state_size);
		resize_num_sub_nodes(min_state_size);
	}

	/* constructor for a single graph of a given size */
	state(unsigned int size) : state(size, 1) {}

	/* constructor for multiple graph of a given size */
	state(unsigned int size, unsigned int n) : num_graphs(n) {
		resize_num_graphs(n);
		symbolic_iteration.resize_num_graphs(n);
		resize_num_nodes(size*n);
		resize_num_sub_nodes(size*n);

		real[0] = 1 / precision::sqrt((PROBA_TYPE)n);
		node_begin[0] = 0;
		sub_node_begin[0] = 0;

		std::fill(right_idx__or_type_.begin(), right_idx__or_type_.begin() + size*n, element_t);

		for (unsigned int gid = 0; gid < n; ++gid) {
			real[gid] = real[0]; imag[gid] = 0;
			node_begin[gid + 1] = size*(gid + 1);
			sub_node_begin[gid + 1] = size*(gid + 1);

			std::iota(node_id_c.begin() + node_begin[gid], node_id_c.begin() + node_begin[gid + 1], 0);
			std::iota(left_idx__or_element__and_has_most_left_zero__or_is_trash_.begin() + node_begin[gid], left_idx__or_element__and_has_most_left_zero__or_is_trash_.begin() + node_begin[gid + 1], 1);
			left_idx__or_element__and_has_most_left_zero__or_is_trash_[node_begin[gid]] = -1;

			for (unsigned int node = 0; node < size; ++node)
				hash_node(gid, node);
		}
	}

	/* constructor for a temp state */
	state(size_t num_a, size_t num_b, size_t num_c) {
		resize_num_graphs(num_a);
		resize_num_nodes(num_b);
		resize_num_sub_nodes(num_c);
	}

	/* vectors can have 3 different sizes :
		- (a) the numer of graph (so a single element per graph), and + 1
		- (b) the total numer of nodes (sum of the numer of nodes for each graph)
		- (c) the total numer of sub-nodes
	*/

	size_t num_graphs = 0;

	// graph magnitude
	std::vector<PROBA_TYPE, allocator<PROBA_TYPE>> real; /* of size (a) */
	std::vector<PROBA_TYPE, allocator<PROBA_TYPE>> imag; /* of size (a) */

	// begin for each size
	std::vector<unsigned int, allocator<unsigned int>> node_begin; /* of size (a + 1), refers to vector of size (b) */
	std::vector <unsigned int, allocator<unsigned int>> sub_node_begin; /* of size (a + 1), refers to vectors of size (c) */

	// graph properties
	std::vector</*bool*/ char, allocator<char>> left_; /* of size (b) */
	std::vector</*bool*/ char, allocator<char>> right_; /* of size (b) */
	std::vector<unsigned short int, allocator<unsigned short int>> node_id_c; /* of size (b), points to the sunode_node (c) namming the ith node (b) */

	// node properties
	std::vector<short int, allocator<short int>> left_idx__or_element__and_has_most_left_zero__or_is_trash_; /* of size (c) */
	std::vector<short int, allocator<short int>> right_idx__or_type_; /* of size (c) */
	std::vector<size_t, allocator<size_t>> node_hash; /* of size (c) */

	/*
	intermediary vectors used to generate the next state :
	*/

	// vector of operations
	std::vector<op_type_t, allocator<op_type_t>> operations; /* of size (b) */

	// numer of sub-graph for each parent
	std::vector<unsigned short int, allocator<unsigned short int>> num_childs; /* of size (a) */

	/* symbolic iteration */
	struct symbolic_t {
		size_t num_graphs = 0;

		// new graphs
		std::vector</*bool*/ char, allocator<char>> is_first_index; /* of size (a) for the symbolic iteration */
		std::vector<unsigned int, allocator<unsigned int>> next_gid; /* of size (a) for the symbolic iteration */
		std::vector<unsigned int, allocator<unsigned int>> parent_gid; /* of size (a) for the symbolic iteration */
		std::vector<unsigned short int, allocator<unsigned short int>> child_id; /* of size (a) for the symbolic iteration */

		// new graph hash
		std::vector<size_t, allocator<size_t>> next_hash; /* of size (a) for the symbolic iteration */

		// new graph random selector
		std::vector<float, allocator<float>> random_selector; /* of size (a) for the symbolic iteration */

		// new graph magnitude
		std::vector<PROBA_TYPE, allocator<PROBA_TYPE>> next_real; /* of size (a) for the symbolic iteration */
		std::vector<PROBA_TYPE, allocator<PROBA_TYPE>> next_imag; /* of size (a) for the symbolic iteration */

		// size for each size
		std::vector<unsigned int, allocator<unsigned int>> node_size; /* of size (a + 1), refers to vector of size (b) */
		std::vector <unsigned int, allocator<unsigned int>> sub_node_size; /* of size (a + 1), refers to vectors of size (c) */

		// resize operator
		void resize_num_graphs(size_t size) {
			size = std::max(min_state_size, size);
			if (next_gid.size() < size ||
			downsize_policy * next_gid.size() > size * upsize_policy) {

				resize(next_gid, (size_t)(upsize_policy * size));
				resize(parent_gid, (size_t)(upsize_policy * size));
				resize(child_id, (size_t)(upsize_policy * size));
				resize(is_first_index, (size_t)(upsize_policy * size));
				resize(next_hash, (size_t)(upsize_policy * size));
				resize(next_real, (size_t)(upsize_policy * size));
				resize(next_imag, (size_t)(upsize_policy * size));
				resize(node_size, (size_t)(upsize_policy * size));
				resize(sub_node_size, (size_t)(upsize_policy * size));
				resize(random_selector, (size_t)(upsize_policy * size));
			}

			std::iota(next_gid.begin(), next_gid.begin() + size, 0);
		}
	} symbolic_iteration;

	// resize operators
	void resize_num_graphs(size_t size) {
		size = std::max(min_state_size, size);
		if (num_childs.size() < size ||
		downsize_policy * num_childs.size() > size * upsize_policy) {

			resize(real, (size_t)(upsize_policy * size));
			resize(imag, (size_t)(upsize_policy * size));
			resize(node_begin, (size_t)(upsize_policy * size) + 1);
			resize(sub_node_begin, (size_t)(upsize_policy * size) + 1);
			resize(num_childs, (size_t)(upsize_policy * size));
		}
	}

	void resize_num_nodes(size_t size) {
		size = std::max(min_state_size, size);
		if (node_id_c.size() < size ||
		downsize_policy * node_id_c.size() > size * upsize_policy) {

			resize(left_, (size_t)(upsize_policy * size));
			resize(right_, (size_t)(upsize_policy * size));
			resize(node_id_c, (size_t)(upsize_policy * size));
			resize(operations, (size_t)(upsize_policy * size));
		}
	}

	void resize_num_sub_nodes(size_t size) {
		size = std::max(min_state_size, size);
		if (node_hash.size() < size ||
		downsize_policy * node_hash.size() > size * upsize_policy) {

			resize(left_idx__or_element__and_has_most_left_zero__or_is_trash_, (size_t)(upsize_policy * size));
			resize(right_idx__or_type_, (size_t)(upsize_policy * size));
			resize(node_hash, (size_t)(upsize_policy * size));
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

		if (right_idx__or_type < pair_t) {
			boost::hash_combine(hash_, right_idx__or_type);
		} else
			boost::hash_combine(hash_, hash(gid, right_idx__or_type));

		return hash_;
	}
	void inline hash_node(unsigned int gid, unsigned short int node) {
		unsigned int id = sub_node_begin[gid] + node;
		node_hash[id] = hash_node_by_value(gid, left_idx__or_element__and_has_most_left_zero__or_is_trash_[id], right_idx__or_type_[id]);
	}

	/* getters */
	// sizes
	unsigned int inline num_nodes(unsigned int gid) const { return node_begin[gid + 1] - node_begin[gid]; }
	unsigned int inline num_sub_node(unsigned int gid) const { return sub_node_begin[gid + 1] - sub_node_begin[gid]; }

	// getter for nodes
	unsigned short int inline node_id(unsigned int gid, unsigned short int node) const { return node_id_c[node_begin[gid] + node]; }
	bool inline left(unsigned int gid, unsigned short int node) const { return left_[node_begin[gid] + node]; }
	bool inline right(unsigned int gid, unsigned short int node) const { return right_[node_begin[gid] + node]; }
	op_type_t inline operation(unsigned int gid, unsigned short int node) const { return operations[node_begin[gid] + node]; }

	// raw getters for sub-nodes
	size_t hash(unsigned int gid, unsigned short int node) const { return node_hash[sub_node_begin[gid] + node]; }
	short int inline right_idx(unsigned int gid, unsigned short int node) const { return right_idx__or_type_[sub_node_begin[gid] + node]; }
	short int inline &right_idx(unsigned int gid, unsigned short int node) { return right_idx__or_type_[sub_node_begin[gid] + node]; }
	short int inline raw_left_idx(unsigned int gid, unsigned short int node) const { return left_idx__or_element__and_has_most_left_zero__or_is_trash_[sub_node_begin[gid] + node]; }
	short int inline &raw_left_idx(unsigned int gid, unsigned short int node) { return left_idx__or_element__and_has_most_left_zero__or_is_trash_[sub_node_begin[gid] + node]; }
	
	// composits getters for raw_left_idx
	unsigned short int inline element(unsigned int gid, unsigned short int node) const { return left_idx(gid, node); }
	bool inline is_trash(unsigned int gid, unsigned short int node) const { return raw_left_idx(gid, node) == 0; }	
	bool inline has_most_left_zero(unsigned int gid, unsigned short int node) const {
		return raw_left_idx(gid, node) < 0;
	}
	unsigned short int inline left_idx(unsigned int gid, unsigned short int node) const { 
		return std::abs(raw_left_idx(gid, node)) - 1;
	}

	// composits getters for raw_right_idx
	node_type_t inline node_type(unsigned int gid, unsigned short int node) const {
		return std::min((node_type_t)right_idx(gid, node), pair_t);
	}

	/* setters */
	// setters for nodes
	void inline set_node_id(unsigned int gid, unsigned short int node, unsigned short int value) { node_id_c[node_begin[gid] + node] = value; }
	void inline set_left(unsigned int gid, unsigned short int node, bool value) { left_[node_begin[gid] + node] = value; }
	void inline set_right(unsigned int gid, unsigned short int node, unsigned short int value) { right_[node_begin[gid] + node] = value; }
	void inline set_operation(unsigned int gid, unsigned short int node, op_type_t value) { operations[node_begin[gid] + node] = value; }

	// raw setters for sub-nodes
	void inline set_raw_left(unsigned int gid, unsigned short int node, short int value) { raw_left_idx(gid, node) = value; }
	
	// composite setters for raw_left_idx
	void inline set_is_trash(unsigned int gid, unsigned short int node) { set_raw_left(gid, node, 0); }
	void inline set_left_idx(unsigned int gid, unsigned short int node, unsigned short int value) { set_raw_left(gid, node, value + 1); }
	void inline set_element(unsigned int gid, unsigned short int node, unsigned short int value) { set_left_idx(gid, node, value); }
	void inline set_has_most_left_zero(unsigned int gid, unsigned short int node, bool has_most_left_zero_) {
		short int &temp = raw_left_idx(gid, node);

		if (has_most_left_zero_ == (temp > 0))
			temp *= -1;
	}

	// composite setters for raw_right_idx
	void inline set_right_idx(unsigned int gid, unsigned short int node, unsigned short int value) { right_idx(gid, node) = value; }
	void inline set_type(unsigned int gid, unsigned short int node, node_type_t value) { right_idx(gid, node) = value; }
	
	/* randomize function */
	void randomize() {
		// random genearator
		size_t size = left_.size();
		for (int i = 0; i < size; ++i) {
			left_[i] = std::rand() & 1;
			right_[i] = std::rand() & 1;
		}
	}

	/* step function */
	void step(state_t &buffer_state, rule_t const &rule) { step(buffer_state, rule, false); }
	void step(state_t &buffer_state, rule_t const &rule, bool normalize) {
		/* check for calssical cases */
		if (!rule.probabilist && rule.do_real == 0 && rule.do_imag == 0)
			return;

		/* allow nested parallism for __gnu_parallel */
		omp_set_nested(1);

		symbolic_iteration.num_graphs = 0;
		PROBA_TYPE total_proba = !normalize;

		/* for reduction */
		size_t &total_num_graphs = symbolic_iteration.num_graphs;

		/* !!!!!!!!!!!!!!!!
		step (1) 
		 !!!!!!!!!!!!!!!! */

		if (verbose >= STEP_DEBUG_LEVEL)
			std::cout << "step 1\n";

		#pragma omp parallel
		{
			#pragma omp for
			for (unsigned int gid = 0; gid < num_graphs; ++gid) {
				auto num_nodes_ = num_nodes(gid);

				/* get operations for each nodes of each graph */
				for (unsigned int node = 0; node < num_nodes_; ++node)
					set_operation(gid, node, rule.operation(*this, gid, node));
			}

			/* !!!!!!!!!!!!!!!!
			step (2) 
			 !!!!!!!!!!!!!!!! */

			#pragma omp single
			if (verbose >= STEP_DEBUG_LEVEL)
				std::cout << "step 2\n";

			#ifndef USE_MPRF
				#pragma omp for reduction(+:total_num_graphs) reduction(+:total_proba)
			#else
				#pragma omp single
			#endif
			for (unsigned int gid = 0; gid < num_graphs; ++gid) {
				/* get the numer of child for each graph */
				num_childs[gid] = rule.num_childs(*this, gid);

				/* compute the total numer of child in parallel */
				total_num_graphs += num_childs[gid];

				/* compute total proba */
				if (normalize) {
					PROBA_TYPE r = real[gid];
					PROBA_TYPE i = imag[gid];

					total_proba += r*r + i*i;
				}
			}

			/* !!!!!!!!!!!!!!!!
			step (2.1) 
			 !!!!!!!!!!!!!!!! */

			if (normalize) {
				#pragma omp single
				total_proba = precision::sqrt(total_proba);

				#pragma omp for
				for (unsigned int gid = 0; gid < num_graphs; ++gid) {
					real[gid] /= total_proba;
					imag[gid] /= total_proba;
				}
			}

			/* !!!!!!!!!!!!!!!!
			step (3) 
			 !!!!!!!!!!!!!!!! */
			
			#pragma omp single
			{
				if (verbose >= STEP_DEBUG_LEVEL)
					std::cout << "step 3\n";

				/* resize variables with the right_ numer of elements */
				symbolic_iteration.resize_num_graphs(symbolic_iteration.num_graphs);
				
				unsigned int id = 0;
				for (unsigned int gid = 0; gid < num_graphs; ++gid) {
					unsigned int const num_child = num_childs[gid];
					
					/* assign parent ids and child ids for each child */
					for (unsigned int child_id_ = 0; child_id_ < num_child; ++child_id_, ++id) {
						symbolic_iteration.parent_gid[id] = gid;
						symbolic_iteration.child_id[id] = child_id_;
					}
				}

				/* !!!!!!!!!!!!!!!!
				step (4) 
				 !!!!!!!!!!!!!!!! */

				if (verbose >= STEP_DEBUG_LEVEL)
					std::cout << "step 4\n";
			}

			#pragma omp for
			for (unsigned int gid = 0; gid < symbolic_iteration.num_graphs; ++gid)
				rule.child_properties(symbolic_iteration.next_hash[gid],
					symbolic_iteration.next_real[gid], symbolic_iteration.next_imag[gid],
					symbolic_iteration.node_size[gid], symbolic_iteration.sub_node_size[gid],
					*this, symbolic_iteration.parent_gid[gid], symbolic_iteration.child_id[gid]);

			/* !!!!!!!!!!!!!!!!
			step (5) 
			 !!!!!!!!!!!!!!!! */

			#pragma omp single
			{
				if (verbose >= STEP_DEBUG_LEVEL)
				std::cout << "step 5\n";
		
				/* sort graphs hash to compute interference */
				__gnu_parallel::sort(symbolic_iteration.next_gid.begin(), symbolic_iteration.next_gid.begin() + symbolic_iteration.num_graphs, [&](unsigned int const &gid1, unsigned int const &gid2) {
					size_t hash1 = symbolic_iteration.next_hash[gid1];
					size_t hash2 = symbolic_iteration.next_hash[gid2];

					if (hash1 != hash2)
						return hash1 > hash2;

					/* graph with less child are less costly to compute */
					return num_childs[symbolic_iteration.parent_gid[gid1]] < num_childs[symbolic_iteration.parent_gid[gid2]];
				});

				/* compute is_first_index */
				symbolic_iteration.is_first_index[symbolic_iteration.next_gid[0]] = true;
			}

			#pragma omp for
			for (unsigned int gid = 1; gid < symbolic_iteration.num_graphs; ++gid)
				symbolic_iteration.is_first_index[symbolic_iteration.next_gid[gid]] = symbolic_iteration.next_hash[symbolic_iteration.next_gid[gid]] != symbolic_iteration.next_hash[symbolic_iteration.next_gid[gid - 1]];

			/* compute interferances */
			/* !!!!!!!!!!!!!!!!!!!!!!!!
			potentiel d'optimisation
			!!!!!!!!!!!!!!!!!!!!!!!! */
			if (rule.probabilist) {
				#pragma omp for
				for (unsigned int gid = 0; gid < symbolic_iteration.num_graphs; ++gid) {
					auto id = symbolic_iteration.next_gid[gid];

					if (symbolic_iteration.is_first_index[id]) {
						/* delete phase */
						PROBA_TYPE r = symbolic_iteration.next_real[id];
						PROBA_TYPE i = symbolic_iteration.next_imag[id];

						symbolic_iteration.next_real[id] = r*r + i*i;
						symbolic_iteration.next_imag[id] = 0;

						/* sum magnitude of equal graphs */
						for (unsigned int gid_ = gid + 1; gid_ < symbolic_iteration.num_graphs && !symbolic_iteration.is_first_index[symbolic_iteration.next_gid[gid_]]; ++gid_) {
							auto id_ = symbolic_iteration.next_gid[gid_];

							PROBA_TYPE r = symbolic_iteration.next_real[id_];
							PROBA_TYPE i = symbolic_iteration.next_imag[id_];

							symbolic_iteration.next_real[id] += r*r + i*i;
						}

						/* take square root */
						symbolic_iteration.next_real[id] = precision::sqrt(symbolic_iteration.next_real[id]);
					}
				}
			} else
				#pragma omp for
				for (unsigned int gid = 0; gid < symbolic_iteration.num_graphs; ++gid) {
					auto id = symbolic_iteration.next_gid[gid];

					if (symbolic_iteration.is_first_index[id])
						/* sum magnitude of equal graphs */
						for (unsigned int gid_ = gid + 1; gid_ < symbolic_iteration.num_graphs && !symbolic_iteration.is_first_index[symbolic_iteration.next_gid[gid_]]; ++gid_) {
							auto id_ = symbolic_iteration.next_gid[gid_];

							symbolic_iteration.next_real[id] += symbolic_iteration.next_real[id_];
							symbolic_iteration.next_imag[id] += symbolic_iteration.next_imag[id_];
						}
				}

			#pragma omp single
			{
				/* get all graphs with a non zero probability */
				auto partitioned_it = __gnu_parallel::partition(symbolic_iteration.next_gid.begin(), symbolic_iteration.next_gid.begin() + symbolic_iteration.num_graphs,
				[&](unsigned int const &gid) {
					/* check if graph is unique */
					if (!symbolic_iteration.is_first_index[gid])
						return false;

					/* check for zero probability */
					PROBA_TYPE r = symbolic_iteration.next_real[gid];
					PROBA_TYPE i = symbolic_iteration.next_imag[gid];

					return r*r + i*i > tolerance; 
				});

				long int next_num_graphs = std::distance(symbolic_iteration.next_gid.begin(), partitioned_it);
						
				/* !!!!!!!!!!!!!!!!
				step (6) 
				 !!!!!!!!!!!!!!!! */

				if (verbose >= STEP_DEBUG_LEVEL)
						std::cout << "step 6\n";

				if (num_graphs >= min_state_size) {
					auto [total_memory, free_mem] = get_mem_usage_and_free_mem();

					const long int graph_mem_usage = 2*sizeof(PROBA_TYPE) + 2*sizeof(unsigned int) + sizeof(unsigned short int);
					const long int node_mem_usage = 2*sizeof(char) + sizeof(unsigned short int) + sizeof(op_type_t);
					const long int sub_node_mem_usage = 2*sizeof(short int) + sizeof(size_t);
					const long int symbolic_mem_usage = sizeof(char) + 2*sizeof(unsigned int) + sizeof(unsigned short int) + sizeof(size_t) + 2*sizeof(PROBA_TYPE) + sizeof(float);

					long int mem_usage_per_graph = (graph_mem_usage +
						(node_mem_usage * node_begin[num_graphs] +
						sub_node_mem_usage * sub_node_begin[num_graphs] +
						symbolic_mem_usage * symbolic_iteration.num_graphs) / num_graphs
						) * upsize_policy;

					long int mem_difference = free_mem - total_memory*safety_margin;
					long int max_num_graphs = (num_graphs + buffer_state.num_graphs + mem_difference / mem_usage_per_graph) / 2;

					/* !!!!!!!!!!!!!!!!
					step (6.1) 
					 !!!!!!!!!!!!!!!! */

					if (next_num_graphs > max_num_graphs) {

						/* generate random selectors */
						//const float size_max = (float)SIZE_MAX;
						#pragma omp parallel for
						for (auto gid_it = symbolic_iteration.next_gid.begin(); gid_it != partitioned_it; ++gid_it)  {
							PROBA_TYPE r = symbolic_iteration.next_real[*gid_it];
							PROBA_TYPE i = symbolic_iteration.next_imag[*gid_it];

							float &random_number = symbolic_iteration.random_selector[*gid_it];
							random_number = static_cast<float> (rand()) / static_cast<float> (RAND_MAX);
							random_number = -std::log(1 - random_number) / (r*r + i*i);
						} 

						/* select graphs according to random selectors */
						__gnu_parallel::nth_element(symbolic_iteration.next_gid.begin(), symbolic_iteration.next_gid.begin() + max_num_graphs, partitioned_it,
						[&](unsigned int const &gid1, unsigned int const &gid2) {
							return symbolic_iteration.random_selector[gid1] < symbolic_iteration.random_selector[gid2];
						});

						next_num_graphs = max_num_graphs;
					}
				}

				buffer_state.num_graphs = next_num_graphs;

				/* !!!!!!!!!!!!!!!!
				step (7) 
				 !!!!!!!!!!!!!!!! */

				if (verbose >= STEP_DEBUG_LEVEL)
					std::cout << "step 7\n";

				/* sort to make memory access more continuous */
				__gnu_parallel::sort(symbolic_iteration.next_gid.begin(), symbolic_iteration.next_gid.begin() + buffer_state.num_graphs);

				/* resize new step variables */
				buffer_state.resize_num_graphs(buffer_state.num_graphs);
			}

			/* prepare for partial sum */
			#pragma omp for
			for (unsigned int gid = 0; gid < buffer_state.num_graphs; ++gid) {
				unsigned int id = symbolic_iteration.next_gid[gid];

				buffer_state.node_begin[gid + 1] = symbolic_iteration.node_size[id];
				buffer_state.sub_node_begin[gid + 1] = symbolic_iteration.sub_node_size[id];

				/* assign magnitude */
				buffer_state.real[gid] = symbolic_iteration.next_real[id];
				buffer_state.imag[gid] = symbolic_iteration.next_imag[id];
			}

			#pragma omp single
			{
				/* compute the partial sums to get new node_begin and sub_node_begin */
				buffer_state.node_begin[0] = 0;
				__gnu_parallel::partial_sum(buffer_state.node_begin.begin() + 1, buffer_state.node_begin.begin() + buffer_state.num_graphs + 1, buffer_state.node_begin.begin() + 1);

				buffer_state.sub_node_begin[0] = 0;
				__gnu_parallel::partial_sum(buffer_state.sub_node_begin.begin() + 1, buffer_state.sub_node_begin.begin() + buffer_state.num_graphs + 1, buffer_state.sub_node_begin.begin() + 1);

				/* resize new step variables */
				buffer_state.resize_num_nodes(buffer_state.node_begin[buffer_state.num_graphs]);
				buffer_state.resize_num_sub_nodes(buffer_state.sub_node_begin[buffer_state.num_graphs]);

				/* !!!!!!!!!!!!!!!!
				step (8) 
				 !!!!!!!!!!!!!!!! */

				if (verbose >= STEP_DEBUG_LEVEL)
					std::cout << "step 8\n";
			}

			#pragma omp for
			for (unsigned int gid = 0; gid < buffer_state.num_graphs; ++gid) {
				auto id = symbolic_iteration.next_gid[gid];
				/* populate graphs */
				rule.populate_new_graph(*this, buffer_state, gid, symbolic_iteration.parent_gid[id], symbolic_iteration.child_id[id]);
			}
		}

		/* !!!!!!!!!!!!!!!!
		step (9) 
		 !!!!!!!!!!!!!!!! */

		/* swap states */
		std::swap(symbolic_iteration, buffer_state.symbolic_iteration);
		std::swap(*this, buffer_state);
	}
};


/*
-----------------------------------------------------------------
for graphing
-----------------------------------------------------------------
*/

void start_json(state_t::rule_t const &rule_1, state_t::rule_t const &rule_2, unsigned int n_iter) {
	// print number of iterations
	std::cout << "{\n\t\"n_iter\" : " << n_iter << ",";

	// print rules
	std::cout << "\n\t\"rules\" : [";

	auto print_rule = [](state_t::rule_t const &rule, bool next) {
		std::cout << "\n\t\t{\n\t\t\t\"name\" : \"" << rule.name << "\",";
		std::cout << "\n\t\t\t\"n_iter\" : " << rule.n_iter << ",";
		std::cout << "\n\t\t\t\"move\" : " << (rule.move ? "true" : "false") << ",";

		if (!rule.probabilist) {
			std::cout << "\n\t\t\t\"teta\" : " << rule.teta << ",";
			std::cout << "\n\t\t\t\"phi\" : " << rule.phi << ",";
			std::cout << "\n\t\t\t\"xi\" : " << rule.xi;
		} else {
			std::cout << "\n\t\t\t\"p\" : " << rule.p << ",";
			std::cout << "\n\t\t\t\"q\" : " << rule.q;
		}
		
		std::cout << "\n\t\t}";

		if (next)
			std::cout << ", ";
	};

	if (rule_1.n_iter > 0)
		print_rule(rule_1, rule_2.n_iter > 0);

	if (rule_2.n_iter > 0)
		print_rule(rule_2, false);
	
	std::cout << "\n\t],\n\t\"iterations\" : [\n\t\t";
}


void serialize_state_to_json(state_t const &s, bool last) {
	PROBA_TYPE avg_size = 0;
	PROBA_TYPE avg_size_squared = 0;

	PROBA_TYPE avg_density = 0;
	PROBA_TYPE avg_density_squared = 0;

	PROBA_TYPE total_proba = 0;

	#ifndef USE_MPRF
		#pragma omp parallel for \
			reduction(+ : avg_size) reduction(+ : avg_size_squared) \
			reduction(+ : avg_density) reduction(+ : avg_density_squared) \
			reduction(+ : total_proba)
	#endif
	for (unsigned int gid = 0; gid < s.num_graphs; ++gid) {
		unsigned int num_nodes_ = s.num_nodes(gid);

		PROBA_TYPE size = (PROBA_TYPE)num_nodes_;
		PROBA_TYPE density = 0;

		PROBA_TYPE real = s.real[gid];
		PROBA_TYPE imag = s.imag[gid];
		PROBA_TYPE proba = real*real + imag*imag;

		for (unsigned i = 0; i < size; ++i)
			density += s.left(gid, i) + s.right(gid, i);
		density /= size;

		avg_size += proba*size;
		avg_size_squared += proba*size*size;

		avg_density += proba*density;
		avg_density_squared += proba*density*density;

		total_proba += proba;
	}

	// correct
	avg_size /= total_proba;
	avg_size_squared /= total_proba;
	avg_density /= total_proba;
	avg_density_squared /= total_proba;

	// compute size standard deviation
	PROBA_TYPE std_dev_size = avg_size_squared - avg_size*avg_size;
	std_dev_size = std_dev_size <= 0 ? 0 : precision::sqrt(std_dev_size);

	// compute density standard deviation
	PROBA_TYPE std_dev_density = avg_density_squared - avg_density*avg_density;
	std_dev_density = std_dev_density <= 0 ? 0 : precision::sqrt(std_dev_density);

	// print ratio of graphs
	float ratio = s.symbolic_iteration.num_graphs == 0 ? 1 : (float)s.num_graphs / (float)s.symbolic_iteration.num_graphs;
	std::cout << "{\n\t\t\t\"ratio\": " << ratio;

	// print total proba
	std::cout << ",\n\t\t\t\"total_proba\": " << total_proba;

	// print num graphs
	std::cout << ",\n\t\t\t\"num_graphs\": " << s.num_graphs;

	// print sizes
	std::cout << ",\n\t\t\t\"avg_size\": " << avg_size;
	std::cout << ",\n\t\t\t\"std_dev_size\": " << std_dev_size;

	// print densities
	std::cout << ",\n\t\t\t\"avg_density\": " << avg_density / 2;
	std::cout << ",\n\t\t\t\"std_dev_density\": " << std_dev_density / 2;

	// print separator
	std::cout << "\n\t\t}";

	// print separator
	if (!last)
		std::cout << ", ";
}

void serialize_state_to_json(state_t const &s) {
	serialize_state_to_json(s, false);
}

void end_json(state_t const &s) {
	serialize_state_to_json(s, true);
	std::cout << "\n\t]\n}\n";
}


/*
-----------------------------------------------------------------
for debugging
-----------------------------------------------------------------
*/


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

	std::vector<unsigned int> gids(s.num_graphs);
	std::iota(gids.begin(), gids.end(), 0);

	unsigned int num_graphs = max_num_graph_print > 0 ? std::min(s.num_graphs, (size_t)max_num_graph_print) : s.num_graphs;

	__gnu_parallel::partial_sort(gids.begin(), gids.begin() + num_graphs, gids.end(),
	[&](unsigned int const &gid1, unsigned int const &gid2) {
		auto r1 = s.real[gid1];
		auto i1 = s.imag[gid1];

		auto r2 = s.real[gid2];
		auto i2 = s.imag[gid2];

		return r1*r1 + i1*i1 > r2*r2 + i2*i2;
	});

	for (int i = 0; i < num_graphs; ++i) {
		unsigned int gid = gids[i];

		if (verbose >= PRINT_DEBUG_LEVEL_3) {
			printf("\ngid:%d, n:", gid);
			for (unsigned int j = s.node_begin[gid]; j < s.node_begin[gid + 1]; ++j)
				printf("%d,", s.node_id_c[j]);

			printf("  ");

			for (unsigned int j = s.sub_node_begin[gid]; j < s.sub_node_begin[gid + 1]; ++j) {
				std::string type;

				auto node_type = s.node_type(gid, j - s.sub_node_begin[gid]);

				if (s.is_trash(gid, j - s.sub_node_begin[gid])) {
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

				printf("%d:(l:%d, r:%d, t:%s), ", j - s.sub_node_begin[gid], s.left_idx__or_element__and_has_most_left_zero__or_is_trash_[j],
					s.right_idx__or_type_[j],
					type.c_str());
			}

			std::cout << "\n";
		}

		PROBA_TYPE real = precision::abs(s.real[gid]) < tolerance ? 0 : s.real[gid];
		PROBA_TYPE imag = precision::abs(s.imag[gid]) < tolerance ? 0 : s.imag[gid];

		std::cout << real << (imag >= 0 ? "+" : "") << imag << "i   ";

		if (verbose >= PRINT_DEBUG_LEVEL_1)
			std::cout << s.symbolic_iteration.next_hash[gid] << "   ";

		unsigned int num_nodes_ = s.num_nodes(gid);
		for (unsigned short int node = 0; node < num_nodes_; ++node) {
			std::cout << "-|" << (s.left(gid, node) ? "<" : " ") << "|";

			print_node(gid, s.node_id(gid, node), false);

			if (verbose >= PRINT_DEBUG_LEVEL_2)
				std::cout << "-" << s.hash(gid, s.node_id(gid, node));

			std::cout << "|" << (s.right(gid, node) ? ">" : " ") << "|-";
		}

		std::cout << "\n";
	}
}