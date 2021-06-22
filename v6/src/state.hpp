#pragma once

#include <vector>
#include <parallel/algorithm>
#include <parallel/numeric>
#include <boost/functional/hash.hpp>
#include <random>
#include <iostream>

/* !!!!!!!!!!!!!!!
		!!!!!!!!!!!!!!!
		debuging
		!!!!!!!!!!!!!!!
		!!!!!!!!!!!!!!! */
#include "utils/debuging.hpp"

// debug levels
#define STEP_DEBUG_LEVEL 1
#define PRINT_DEBUG_LEVEL_1 0.5
#define PRINT_DEBUG_LEVEL_2 0.75

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

// type definition
typedef class state state_t;
typedef class rule rule_t;
typedef char op_type_t;

// global variable definition
PROBA_TYPE tolerance = 0;
float resize_policy = 1.3;

// debugging options
float verbose = 0;
unsigned int max_num_graph_print = 20;

// rule interface definition
class rule {
public:
	/* parameters of a unitary matrix */
	PROBA_TYPE do_real = 1;
	PROBA_TYPE do_imag = 0;
	PROBA_TYPE do_not = 0;

	/* constructor for a unitary matrix */
	rule(PROBA_TYPE teta, PROBA_TYPE phi) {
		do_real = precision::cos(teta)*precision::cos(phi);
		do_imag = precision::cos(teta)*precision::sin(phi);
		do_not = precision::sin(teta);
	}

	/* step (1) */
	virtual op_type_t operation(state_t const &s, unsigned int gid, unsigned short int node) const { return '\0'; }

	/* step (2) */
	virtual unsigned short int num_childs(state_t const &s, unsigned int gid) const { return 0; }

	/* step (4) */
	virtual std::tuple<size_t /* hash */,
		PROBA_TYPE /* real*/, PROBA_TYPE /* imag */,
		unsigned short int /* num_nodes */, unsigned short int /* num sub-nodes */> 
		child_properties(state_t &s, unsigned int parent_id, unsigned int child_id) const { return {0, 0., 0., 0, 0}; }

	/* step (8) */
	virtual void populate_new_graph(state_t const &s, state_t &next_state, unsigned int next_gid, unsigned int parent_id, unsigned int child_id) const {}
};

/*
!!!!!
What are called "sub-nodes" are nodes containers that are used to point to other node container (to generate pairs, left or right nodes).

Iteration protocol is:
  - (1): use a rule to put all operation inside of the vector "operations"

  - (2): calculate the numer of child graph generated by each parent graph and populate "num_childs" (*)

  - (3): read the total numer of child graph generated by each parent graph and populate "next_gid", "next_state.symbolic_iteration.parent_gid" and "next_state.symbolic_iteration.child_id"

  - (4): by iterating on "next_gid" and using the rule, populate "next_hash" and "next_real" and "next_imag" through a symbolic iteration (**),
  	and populate "node_begin" and "sub_node_begin" which are respectively the numer of node and of sub-nodes for each new graph

  - (5): using "next_hash" sum their probability ("next_real" and "next_imag") of equal graphs, while setting the probability of graph which are equal to 0.
  	when sorting graphs according to "next_hash" to sum their probability, we also sort accroding to "num_childs" so the graphs with the smallest number of child
  	are the one for which we end up computing the dynamic since they also tend to be simpler to compute (since they have less interactions).  

  - (5.1): partition "next_gid" based on probability ("next_real" and "next_imag"), and keep only the max_num_graphs most probable graph

  - (6): reserve all public vector for a new state by iterating over the N first "next_gid" andusing "next_state.symbolic_iteration.parent_gid" and "next_state.symbolic_iteration.child_id",
  	and assign "node_begin" and "sub_node_begin".

  - (7): generate all new graphs of the new state by iterating over the N first "next_gid" andusing "next_state.symbolic_iteration.parent_gid" and "next_state.symbolic_iteration.child_id".

(*) to compute the numer of child you just compute "pow(2, num_operations)""
(**) it's possible to compute the hash of a child graph without actually computing it for most rules
*/

class state {
public:
	// type definition of a node
	typedef enum node_type {
		trash_t = -4,
		left_t,
		right_t,
		element_t,
		pair_t,
	} node_type_t;

	/* empty constructor */
	state() {}

	/* constructor for a single graph of a given size */
	state(unsigned int size) : num_graphs(1) {
		resize_num_graphs(1);
		resize_num_graphs_symbolic(1);
		resize_num_nodes(size);
		resize_num_sub_nodes(size);

		real[0] = 1; imag[0] = 0;
		node_begin[1] = size;
		sub_node_begin[1] = size;

		std::iota(node_id_c.begin(), node_id_c.begin() + size, 0);
		std::fill(right_idx__or_type__and_is_trash_.begin(), right_idx__or_type__and_is_trash_.begin() + size, element_t);
		std::iota(left_idx__or_element__and_has_most_left_zero_.begin(), left_idx__or_element__and_has_most_left_zero_.end(), 1);
		left_idx__or_element__and_has_most_left_zero_[0] = -1;

		for (unsigned int i = 0; i < size; ++i)
			hash_node(0, i);
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
	std::vector<PROBA_TYPE> real; /* of size (a) */
	std::vector<PROBA_TYPE> imag; /* of size (a) */

	// begin for each size
	std::vector<unsigned int> node_begin; /* of size (a + 1), refers to vector of of size (b) */
	std::vector <unsigned int> sub_node_begin; /* of size (a + 1), refers to vectors of of size (c) */

	// graph properties
	std::vector</*bool*/ char> left_; /* of size (b) */
	std::vector</*bool*/ char> right_; /* of size (b) */
	std::vector<short int> node_id_c; /* of size (b), points to the sunode_node (c) namming the ith node (b) */

	// node properties
	std::vector<short int> left_idx__or_element__and_has_most_left_zero_; /* of size (c) */
	std::vector<short int> right_idx__or_type__and_is_trash_; /* of size (c) */
	std::vector<size_t> node_hash; /* of size (c) */

	/*
	intermediary vectors used to generate the next state :
	*/

	// vector of operations
	std::vector<op_type_t> operations; /* of size (b) */

	// numer of sub-graph for each parent
	std::vector<unsigned short int> num_childs; /* of size (a) */

	/* symbolic iteration */
	struct symbolic_t {
		size_t num_graphs = 0;

		// new graphs
		std::vector</*bool*/ char> is_first_index; /* of size (a) for the symbolic iteration */
		std::vector<unsigned int> next_gid; /* of size (a) for the symbolic iteration */
		std::vector<unsigned int> parent_gid; /* of size (a) for the symbolic iteration */
		std::vector<unsigned short int> child_id; /* of size (a) for the symbolic iteration */

		// new graph hash
		std::vector<size_t> next_hash; /* of size (a) for the symbolic iteration */

		// new graph magnitude
		std::vector<PROBA_TYPE> next_real; /* of size (a) for the symbolic iteration */
		std::vector<PROBA_TYPE> next_imag; /* of size (a) for the symbolic iteration */

		// resize operator
		void resize_num_graphs(size_t size) {
			if (next_gid.size() < size) {
				next_gid.resize(resize_policy * size);
				parent_gid.resize(resize_policy * size);
				child_id.resize(resize_policy * size);
				is_first_index.resize(resize_policy * size);
				next_hash.resize(resize_policy * size);
				next_real.resize(resize_policy * size);
				next_imag.resize(resize_policy * size);
			}

			std::iota(next_gid.begin(), next_gid.begin() + size, 0);
		}
	} symbolic_iteration;

	// resize operators
	void resize_num_graphs_symbolic(size_t size) {
		if (node_begin.size() < size + 1) {
			node_begin.resize(resize_policy * size + 1);
			sub_node_begin.resize(resize_policy * size + 1);
		}

		symbolic_iteration.resize_num_graphs(size);
	}

	void resize_num_graphs(size_t size) {
		if (real.size() < size) {
			real.resize(resize_policy * size);
			imag.resize(resize_policy * size);
			node_begin.resize(resize_policy * size + 1);
			sub_node_begin.resize(resize_policy * size + 1);
			num_childs.resize(resize_policy * size);
		}
	}

	void resize_num_nodes(size_t size) {
		if (left_.size() < size) {
			left_.resize(resize_policy * size);
			right_.resize(resize_policy * size);
			node_id_c.resize(resize_policy * size);
			operations.resize(resize_policy * size);
		}
	}

	void resize_num_sub_nodes(size_t size) {
		if (node_hash.size() < size) {
			left_idx__or_element__and_has_most_left_zero_.resize(resize_policy * size);
			right_idx__or_type__and_is_trash_.resize(resize_policy * size);
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

		if (right_idx__or_type < pair_t) {
			boost::hash_combine(hash_, right_idx__or_type);
		} else
			boost::hash_combine(hash_, hash(gid, right_idx__or_type));

		return hash_;
	}
	void inline hash_node(unsigned int gid, unsigned short int node) {
		unsigned int id = sub_node_begin[gid] + node;
		node_hash[id] = hash_node_by_value(gid, left_idx__or_element__and_has_most_left_zero_[id], right_idx__or_type__and_is_trash_[id]);
	}

	/* getters */
	// sizes
	unsigned short int inline num_nodes(unsigned int gid) const { return node_begin[gid + 1] - node_begin[gid]; }
	unsigned short int inline num_sub_node(unsigned int gid) const { return sub_node_begin[gid + 1] - sub_node_begin[gid]; }

	// getter for nodes
	unsigned short int inline node_id(unsigned int gid, unsigned short int node) const { return node_id_c[node_begin[gid] + node]; }
	bool inline left(unsigned int gid, unsigned short int node) const { return left_[node_begin[gid] + node]; }
	bool inline right(unsigned int gid, unsigned short int node) const { return right_[node_begin[gid] + node]; }
	op_type_t inline operation(unsigned int gid, unsigned short int node) const { return operations[node_begin[gid] + node]; }

	// raw getters for sub-nodes
	size_t hash(unsigned int gid, unsigned short int node) const { return node_hash[sub_node_begin[gid] + node]; }
	short int inline right_idx(unsigned int gid, unsigned short int node) const { return right_idx__or_type__and_is_trash_[sub_node_begin[gid] + node]; }
	short int inline &right_idx(unsigned int gid, unsigned short int node) { return right_idx__or_type__and_is_trash_[sub_node_begin[gid] + node]; }
	short int inline raw_left_idx(unsigned int gid, unsigned short int node) const { return left_idx__or_element__and_has_most_left_zero_[sub_node_begin[gid] + node]; }
	short int inline &raw_left_idx(unsigned int gid, unsigned short int node) { return left_idx__or_element__and_has_most_left_zero_[sub_node_begin[gid] + node]; }
	
	// composits getters for raw_left_idx
	unsigned short int inline element(unsigned int gid, unsigned short int node) const { return left_idx(gid, node); }
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
	bool inline is_trash(unsigned int gid, unsigned short int node) const { return right_idx(gid, node) == trash_t; }

	/* setters */
	// setters for nodes
	void inline set_node_id(unsigned int gid, unsigned short int node, unsigned short int value) { node_id_c[node_begin[gid] + node] = value; }
	void inline set_left(unsigned int gid, unsigned short int node, bool value) { left_[node_begin[gid] + node] = value; }
	void inline set_right(unsigned int gid, unsigned short int node, unsigned short int value) { right_[node_begin[gid] + node] = value; }
	void inline set_operation(unsigned int gid, unsigned short int node, op_type_t value) { operations[node_begin[gid] + node] = value; }

	// raw setters for sub-nodes
	void inline set_raw_left(unsigned int gid, unsigned short int node, short int value) { raw_left_idx(gid, node) = value; }
	
	// composite setters for raw_left_idx
	void inline set_left_idx(unsigned int gid, unsigned short int node, unsigned short int value) {
		if (has_most_left_zero(gid, node)) {
			raw_left_idx(gid, node) = -value - 1;
		} else
			raw_left_idx(gid, node) = value + 1;
	}
	void inline set_element(unsigned int gid, unsigned short int node, unsigned short int value) { set_left_idx(gid, node, value); }
	void inline set_has_most_left_zero(unsigned int gid, unsigned short int node, bool has_most_left_zero_) {
		short int &temp = raw_left_idx(gid, node);

		if (has_most_left_zero_ == (temp > 0))
			temp *= -1;
	}

	// composite setters for raw_right_idx
	void inline set_right_idx(unsigned int gid, unsigned short int node, unsigned short int value) { right_idx(gid, node) = value; }
	void inline set_type(unsigned int gid, unsigned short int node, node_type_t value) { right_idx(gid, node) = value; }
	void inline set_is_trash(unsigned int gid, unsigned short int node) { set_type(gid, node, trash_t); }
	
	/* step function */
	void step(state_t &next_state, rule_t &rule) { step(next_state, rule, -1); }
	void step(state_t &next_state, rule_t &rule, unsigned int max_num_graphs) {
		symbolic_iteration.num_graphs = 0;

		/* for memory management */
		next_state.symbolic_iteration = std::move(symbolic_iteration);

		/* for reduction */
		size_t &total_num_graphs = next_state.symbolic_iteration.num_graphs;

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

			#pragma omp for reduction(+:total_num_graphs)
			for (unsigned int gid = 0; gid < num_graphs; ++gid) {
				/* get the numer of child for each graph */
				num_childs[gid] = rule.num_childs(*this, gid);

				/* compute the total numer of child in parallel */
				total_num_graphs += num_childs[gid];
			}
		}

		/* !!!!!!!!!!!!!!!!
		step (3) 
		 !!!!!!!!!!!!!!!! */
		
		if (verbose >= STEP_DEBUG_LEVEL)
			std::cout << "step 3\n";

		/* resize variables with the right_ numer of elements */
		next_state.resize_num_graphs_symbolic(next_state.symbolic_iteration.num_graphs);
		
		unsigned int id = 0;
		for (unsigned int gid = 0; gid < num_graphs; ++gid) {
			unsigned int const num_child = num_childs[gid];
			
			/* assign parent ids and child ids for each child */
			for (unsigned int child_id_ = 0; child_id_ < num_child; ++child_id_, ++id) {
				next_state.symbolic_iteration.parent_gid[id] = gid;
				next_state.symbolic_iteration.child_id[id] = child_id_;
			}
		}

		/* !!!!!!!!!!!!!!!!
		step (4) 
		 !!!!!!!!!!!!!!!! */

		if (verbose >= STEP_DEBUG_LEVEL)
			std::cout << "step 4\n";

		#pragma omp parallel for
		for (unsigned int gid = 0; gid < next_state.symbolic_iteration.num_graphs; ++gid) {
			auto [hash_, real_, imag_, num_node_, num_sub_node_] = rule.child_properties(*this, next_state.symbolic_iteration.parent_gid[gid], next_state.symbolic_iteration.child_id[gid]);

			/* assign propreties for each child */
			next_state.symbolic_iteration.next_hash[gid] = hash_;
			next_state.symbolic_iteration.next_real[gid] = real_;
			next_state.symbolic_iteration.next_imag[gid] = imag_;
			next_state.node_begin[gid + 1] = num_node_;
			next_state.sub_node_begin[gid + 1] = num_sub_node_;
		}

		/* !!!!!!!!!!!!!!!!
		step (5) 
		 !!!!!!!!!!!!!!!! */

		if (verbose >= STEP_DEBUG_LEVEL)
			std::cout << "step 5\n";
		
		/* sort graphs hash to compute interference */
		__gnu_parallel::sort(next_state.symbolic_iteration.next_gid.begin(), next_state.symbolic_iteration.next_gid.begin() + next_state.symbolic_iteration.num_graphs, [&](unsigned int const &gid1, unsigned int const &gid2) {
			size_t hash1 = next_state.symbolic_iteration.next_hash[gid1];
			size_t hash2 = next_state.symbolic_iteration.next_hash[gid2];

			if (hash1 != hash2)
				return hash1 > hash2;

			/* graph with less child are less costly to compute */
			return num_childs[next_state.symbolic_iteration.parent_gid[gid1]] < num_childs[next_state.symbolic_iteration.parent_gid[gid2]];
		});

		/* compute is_first_index */
		next_state.symbolic_iteration.is_first_index[next_state.symbolic_iteration.next_gid[0]] = true;

		#pragma omp 
		{
			#pragma omp for
			for (unsigned int gid = 1; gid < next_state.symbolic_iteration.num_graphs; ++gid)
				next_state.symbolic_iteration.is_first_index[next_state.symbolic_iteration.next_gid[gid]] = next_state.symbolic_iteration.next_hash[next_state.symbolic_iteration.next_gid[gid]] != next_state.symbolic_iteration.next_hash[next_state.symbolic_iteration.next_gid[gid - 1]];

			/* compute interferances */
			/* !!!!!!!!!!!!!!!!!!!!!!!!
			potentiel d'optimisation
			!!!!!!!!!!!!!!!!!!!!!!!! */
			#pragma omp for
			for (unsigned int gid = 0; gid < next_state.symbolic_iteration.num_graphs; ++gid) {
				auto id = next_state.symbolic_iteration.next_gid[gid];

				if (next_state.symbolic_iteration.is_first_index[id])
					/* sum magnitude of equal graphs */
					for (unsigned int gid_ = gid + 1; gid_ < next_state.symbolic_iteration.num_graphs && !next_state.symbolic_iteration.is_first_index[next_state.symbolic_iteration.next_gid[gid_]]; ++gid_) {
						auto id_ = next_state.symbolic_iteration.next_gid[gid_];

						next_state.symbolic_iteration.next_real[id] += next_state.symbolic_iteration.next_real[id_];
						next_state.symbolic_iteration.next_imag[id] += next_state.symbolic_iteration.next_imag[id_];
					}
			}
		}

		/* get all graphs with a non zero probability */
		auto partitioned_it = __gnu_parallel::partition(next_state.symbolic_iteration.next_gid.begin(), next_state.symbolic_iteration.next_gid.begin() + next_state.symbolic_iteration.num_graphs, [&](unsigned int const &gid) {
			/* check if graph is unique */
			if (!next_state.symbolic_iteration.is_first_index[gid])
				return false;

			/* check for zero probability */
			auto r = next_state.symbolic_iteration.next_real[gid];
			auto i = next_state.symbolic_iteration.next_imag[gid];

			return r*r + i*i > tolerance; 
		});

		next_state.num_graphs = std::distance(next_state.symbolic_iteration.next_gid.begin(), partitioned_it);
				
		/* !!!!!!!!!!!!!!!!
		step (5.1) 
		 !!!!!!!!!!!!!!!! */

		if (max_num_graphs > 0 && next_state.num_graphs > max_num_graphs) {

			if (verbose >= STEP_DEBUG_LEVEL)
				std::cout << "step 5.1\n";

			/* sort graphs according to probability */
			__gnu_parallel::nth_element(next_state.symbolic_iteration.next_gid.begin(), next_state.symbolic_iteration.next_gid.begin() + max_num_graphs, next_state.symbolic_iteration.next_gid.begin() + next_state.num_graphs,
			[&](unsigned int const &gid1, unsigned int const &gid2) {
				auto r1 = next_state.symbolic_iteration.next_real[gid1];
				auto i1 = next_state.symbolic_iteration.next_imag[gid1];

				auto r2 = next_state.symbolic_iteration.next_real[gid2];
				auto i2 = next_state.symbolic_iteration.next_imag[gid2];

				return r1*r1 + i1*i1 > r2*r2 + i2*i2;
			});

			next_state.num_graphs = max_num_graphs;
		}

		/* !!!!!!!!!!!!!!!!
		step (6) 
		 !!!!!!!!!!!!!!!! */

		if (verbose >= STEP_DEBUG_LEVEL)
			std::cout << "step 6\n";

		/* sort to make memory access more continuous */
		__gnu_parallel::sort(next_state.symbolic_iteration.next_gid.begin(), next_state.symbolic_iteration.next_gid.begin() + next_state.num_graphs);

		/* resize new step variables */
		next_state.resize_num_graphs(next_state.num_graphs);

		/* prepare for partial sum */
		for (unsigned int gid = 0; gid <  next_state.num_graphs; ++gid) {
			unsigned int id = next_state.symbolic_iteration.next_gid[gid];

			next_state.node_begin[gid + 1] = next_state.node_begin[id + 1];
			next_state.sub_node_begin[gid + 1] = next_state.sub_node_begin[id + 1];

			/* assign magnitude */
			next_state.real[gid] = next_state.symbolic_iteration.next_real[id];
			next_state.imag[gid] = next_state.symbolic_iteration.next_imag[id];
		}

		/* compute the partial sums to get new node_begin and sub_node_begin */
		next_state.node_begin[0] = 0;
		__gnu_parallel::partial_sum(next_state.node_begin.begin() + 1, next_state.node_begin.begin() + next_state.num_graphs + 1, next_state.node_begin.begin() + 1);

		next_state.sub_node_begin[0] = 0;
		__gnu_parallel::partial_sum(next_state.sub_node_begin.begin() + 1, next_state.sub_node_begin.begin() + next_state.num_graphs + 1, next_state.sub_node_begin.begin() + 1);

		/* resize new step variables */
		next_state.resize_num_nodes(next_state.node_begin[next_state.num_graphs]);
		next_state.resize_num_sub_nodes(next_state.sub_node_begin[next_state.num_graphs]);

		/* !!!!!!!!!!!!!!!!
		step (7) 
		 !!!!!!!!!!!!!!!! */

		if (verbose >= STEP_DEBUG_LEVEL)
			std::cout << "step 7\n";

		//#pragma omp parallel for
		printf("\n-----------------\n");
		for (unsigned int gid = 0; gid < next_state.num_graphs; ++gid) {
			auto id = next_state.symbolic_iteration.next_gid[gid];

			printf("\n");

			/* populate graphs */
			rule.populate_new_graph(*this, next_state, gid, next_state.symbolic_iteration.parent_gid[id], next_state.symbolic_iteration.child_id[id]);
		}
	}

	void randomize() {
		// random genearator
		size_t size = left_.size();
		for (int i = 0; i < size; ++i) {
			left_[i] = std::rand() & 1;
			right_[i] = std::rand() & 1;
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

	std::vector<unsigned int> gids(s.num_graphs);
	std::iota(gids.begin(), gids.end(), 0);

	unsigned int num_graphs = s.num_graphs; /*std::min(s.num_graphs, (size_t)max_num_graph_print);

	__gnu_parallel::partial_sort(gids.begin(), gids.begin() + num_graphs, gids.end(),
	[&](unsigned int const &gid1, unsigned int const &gid2) {
		auto r1 = s.real[gid1];
		auto i1 = s.imag[gid1];

		auto r2 = s.real[gid2];
		auto i2 = s.imag[gid2];

		return r1*r1 + i1*i1 > r2*r2 + i2*i2;
	});*/

	for (int i = 0; i < num_graphs; ++i) {
		unsigned int gid = gids[i];

		std::cout << s.real[gid] << (s.imag[gid] >= 0 ? "+" : "") << s.imag[gid] << "i   ";

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
