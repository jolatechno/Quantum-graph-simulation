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

// debugging options
unsigned short int verbose = 0;

// rule interface definition
class rule {
public:
	rule() {}

	virtual op_type_t operation(state_t const &s, unsigned int gid, unsigned short int node) const { return '\0'; } /* step (1) */

	virtual unsigned short int num_childs(state_t const &s, unsigned int gid) const { return 0; } /* step (2) */

	virtual std::tuple<size_t /* hash */,
		PROBA_TYPE /* real*/, PROBA_TYPE /* imag */,
		unsigned short int /* num_nodes */, unsigned short int /* num sub-nodes */> 
		child_properties(state_t const &s, unsigned int parent_id, unsigned int child_id) const { return {0, 0., 0., 0, 0}; } /* step (4) */

	virtual void populate_new_graph(state_t const &s, state_t &new_state, unsigned int next_gid, unsigned int parent_id, unsigned int child_id) const {} /* step (8) */

	/* parameters of a unitary matrix */
	PROBA_TYPE do_real = 1;
	PROBA_TYPE do_imag = 0;
	PROBA_TYPE do_not = 0;
};

/*
!!!!!
What are called "sub-nodes" are nodes containers that are used to point to other node container (to generate pairs, left or right nodes).

Iteration protocol is:
  - (1): use a rule to put all operation inside of the vector "operations"

  - (2): calculate the numer of child graph generated by each parent graph and populate "num_childs" (*)

  - (3): read the total numer of child graph generated by each parent graph and populate "next_gid", "parent_gid" and "child_id"

  - (4): by iterating on "next_gid" and using the rule, populate "new_hash" and "new_real" and "new_imag" through a symbolic iteration (**),
  	and populate "new_size_b" and "new_size_c" which are respectively the numer of node and of sub-nodes for each new graph

  - (5): using "new_hash" sum the probability ("new_real" and "new_imag") of equal graphs, while setting the probability of graph which are equal to 0

  - (5.1): partition "next_gid" based on probability ("new_real" and "new_imag"), and keep only the max_num_graphs most probable graph

  - (6): reserve all public vector for a new state by iterating over the N first "next_gid" andusing "parent_gid" and "child_id",
  	and assign "node_begin" and "sub_node_begin".

  - (7): generate all new graphs of the new state by iterating over the N first "next_gid" andusing "parent_gid" and "child_id".

(*) to compute the numer of child you just compute "pow(2, num_operations)""
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
	state(unsigned int size) : num_graphs(1) {
		real = {1};
		imag = {0};

		node_begin = {0, size};
		sub_node_begin = {0, size};

		left_.resize(size, false);
		right_.resize(size, false);
		node_id_c.resize(size);
		left_idx__or_element__and_has_most_left_zero_.resize(size);
		right_idx__or_type_.resize(size, element_t);
		node_hash.resize(size);
		operations.resize(size);
		num_childs.resize(size);

		std::iota(node_id_c.begin(), node_id_c.end(), 0);
		std::iota(left_idx__or_element__and_has_most_left_zero_.begin(), left_idx__or_element__and_has_most_left_zero_.end(), 1);
		left_idx__or_element__and_has_most_left_zero_[0] = -1;

		for (unsigned int i = 0; i < size; ++i) {
			node_hash[i] = i;
			boost::hash_combine(node_hash[i], (short int)element_t);
		}
	}

	/* constructor for a temp state */
	state(size_t size_a_symbolic, size_t size_a, size_t size_b, size_t size_c) {
		resize_num_graphs_symbolic(size_a_symbolic);
		resize_num_graphs(size_a);
		resize_num_nodes(size_b);
		resize_num_sub_nodes(size_c);
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
	std::vector<short int> right_idx__or_type_; /* of size (c) */
	std::vector<size_t> node_hash; /* of size (c) */

	/*
	intermediary vectors used to generate the next state :
	*/

	// vector of operations
	std::vector<op_type_t> operations; /* of size (b) */

	// numer of sub-graph for each parent
	std::vector<unsigned short int> num_childs; /* of size (a) */

	// new graphs
	std::vector</*bool*/ char> is_first_index; /* of size (a) for the symbolic iteration */
	std::vector<unsigned int> next_gid; /* of size (a) for the symbolic iteration */
	std::vector<unsigned int> parent_gid; /* of size (a) for the symbolic iteration */
	std::vector<unsigned int> child_id_begin; /* of size (a) + 1 for the symbolic iteration */
	std::vector<unsigned short int> child_id; /* of size (a) for the symbolic iteration */

	// new graph hash
	std::vector<size_t> new_hash; /* of size (a) for the symbolic iteration */

	/* !!!!!!!!!!!!!!!!!!!!!!!!!!!
	peut etre fait in_place
	!!!!!!!!!!!!!!!!!!!!!!!!!!! */
	std::vector<unsigned short int> new_size_b; /* of size (a) for the symbolic iteration */
	std::vector<unsigned short int> new_size_c; /* of size (a) for the symbolic iteration */

	// new graph magnitude
	std::vector<PROBA_TYPE> new_real; /* of size (a) for the symbolic iteration */
	std::vector<PROBA_TYPE> new_imag; /* of size (a) for the symbolic iteration */

	// resize operators
	void resize_num_graphs_symbolic(size_t size) {
		if (next_gid.size() < size) {
			next_gid.resize(resize_policy * size);
			parent_gid.resize(resize_policy * size);
			child_id.resize(resize_policy * size);
			is_first_index.resize(resize_policy * size);
			child_id_begin.resize(resize_policy * size + 1);
			new_hash.resize(resize_policy * size);
			new_real.resize(resize_policy * size);
			new_imag.resize(resize_policy * size);
			new_size_c.resize(resize_policy * size);
			new_size_b.resize(resize_policy * size);
		}

		std::iota(next_gid.begin(), next_gid.begin() + size, 0);
	}

	void resize_num_graphs(size_t size) {
		if (real.size() < size) {
			real.resize(resize_policy * size);
			imag.resize(resize_policy * size);
			node_begin.resize(resize_policy * size + 1);
			sub_node_begin.resize(resize_policy * size + 1);
		}
	}

	void resize_num_nodes(size_t size) {
		if (left_.size() < size) {
			left_.resize(resize_policy * size);
			right_.resize(resize_policy * size);
			node_id_c.resize(resize_policy * size);
			operations.resize(resize_policy * size);
			num_childs.resize(resize_policy * size);
		}
	}

	void resize_num_sub_nodes(size_t size) {
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
	void inline hash_node(unsigned int gid, unsigned short int node) {
		unsigned int id = sub_node_begin[gid] + node;
		node_hash[id] = hash_node_by_value(gid, left_idx__or_element__and_has_most_left_zero_[id], right_idx__or_type_[id]);
	}

	/* getters */
	unsigned short int inline num_nodes(unsigned int gid) const { return node_begin[gid + 1] - node_begin[gid]; }
	unsigned short int inline sub_node_size(unsigned int gid) const { return sub_node_begin[gid + 1] - sub_node_begin[gid]; }

	unsigned short int inline node_id(unsigned int gid, unsigned short int node) const { return node_id_c[node_begin[gid] + node]; }
	bool inline left(unsigned int gid, unsigned short int node) const { return left_[node_begin[gid] + node]; }
	bool inline right(unsigned int gid, unsigned short int node) const { return right_[node_begin[gid] + node]; }
	op_type_t inline operation(unsigned int gid, unsigned short int node) const { return operations[node_begin[gid] + node]; }

	size_t hash(unsigned int gid, unsigned short int node) const { return node_hash[sub_node_begin[gid] + node]; }
	short int inline right_idx(unsigned int gid, unsigned short int node) const { return right_idx__or_type_[sub_node_begin[gid] + node]; }
	short int inline &right_idx(unsigned int gid, unsigned short int node) { return right_idx__or_type_[sub_node_begin[gid] + node]; }
	short int inline raw_left_idx(unsigned int gid, unsigned short int node) const { return left_idx__or_element__and_has_most_left_zero_[sub_node_begin[gid] + node]; }
	short int inline &raw_left_idx(unsigned int gid, unsigned short int node) { return left_idx__or_element__and_has_most_left_zero_[sub_node_begin[gid] + node]; }
	bool inline has_most_left_zero(unsigned int gid, unsigned short int node) const {
		return raw_left_idx(gid, node) < 0;
	}
	unsigned short int inline left_idx(unsigned int gid, unsigned short int node) const { 
		return std::abs(raw_left_idx(gid, node)) - 1;
	}
	unsigned short int inline element(unsigned int gid, unsigned short int node) const { return left_idx(gid, node); }
	node_type_t inline node_type(unsigned int gid, unsigned short int node) const {
		return std::min((node_type_t)right_idx(gid, node), pair_t);
	}

	/* setters */
	void inline set_node_id(unsigned int gid, unsigned short int node, unsigned short int value) { node_id_c[node_begin[gid] + node] = value; }
	void inline set_left(unsigned int gid, unsigned short int node, bool value) { left_[node_begin[gid] + node] = value; }
	void inline set_right(unsigned int gid, unsigned short int node, unsigned short int value) { right_[node_begin[gid] + node] = value; }
	void inline set_operation(unsigned int gid, unsigned short int node, op_type_t value) { operations[node_begin[gid] + node] = value; }

	void inline set_right_idx(unsigned int gid, unsigned short int node, unsigned short int value) { right_idx(gid, node) = value; }
	void inline set_raw_left(unsigned int gid, unsigned short int node, short int value) { raw_left_idx(gid, node) = value; }
	void inline set_type(unsigned int gid, unsigned short int node, node_type_t value) { set_right_idx(gid, node, value); }
	void inline set_left_idx(unsigned int gid, unsigned short int node, unsigned short int value) {
		if (has_most_left_zero(gid, node)) {
			left_idx__or_element__and_has_most_left_zero_[sub_node_begin[gid] + node] = -value - 1;
		} else
			left_idx__or_element__and_has_most_left_zero_[sub_node_begin[gid] + node] = value + 1;
	}
	void inline set_element(unsigned int gid, unsigned short int node, unsigned short int value) { set_left_idx(gid, node, value); }
	void inline set_most_left_zero(unsigned int gid, unsigned short int node, bool has_most_left_zero_) {
		short int &temp = left_idx__or_element__and_has_most_left_zero_[sub_node_begin[gid] + node];

		if (has_most_left_zero_ == (temp > 0))
			temp *= -1;
	}
	

	/* step function */
	void step(state_t &new_state, rule_t &rule) { step(new_state, rule, -1); }
	void step(state_t &new_state, rule_t &rule, unsigned int max_num_graphs) {
		size_t total_num_graphs = 0;

		#pragma omp parallel
		{

			/* !!!!!!!!!!!!!!!!
			step (1) 
			 !!!!!!!!!!!!!!!! */

			#pragma omp single
			if (verbose > 0)
				std::cout << "step 1\n";

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
			if (verbose > 0)
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
		
		if (verbose > 0)
			std::cout << "step 3\n";

		/* resize variables with the right_ numer of elements */
		resize_num_graphs_symbolic(total_num_graphs);

		child_id_begin[0] = 0;
		__gnu_parallel::partial_sum(num_childs.begin(), num_childs.begin() + num_graphs, child_id_begin.begin() + 1);

		#pragma omp parallel
		{
			#pragma omp for
			for (unsigned int gid = 0; gid < num_graphs; ++gid) {
				unsigned int const num_child = num_childs[gid];
				unsigned int const child_begin = child_id_begin[gid];

				/* assign parent ids and child ids for each child */
				for (unsigned int child_id_ = 0; child_id_ < num_child; ++child_id_) {
					parent_gid[child_begin + child_id_] = gid;
					child_id[child_begin + child_id_] = child_id_;
				}
			}

			/* !!!!!!!!!!!!!!!!
			step (4) 
			 !!!!!!!!!!!!!!!! */

			#pragma omp single
			if (verbose > 0)
				std::cout << "step 4\n";

			#pragma omp for
			for (unsigned int gid = 0; gid < total_num_graphs; ++gid) {
				auto [hash_, real_, imag_, size_node_, size_sub_node_] = rule.child_properties(*this, parent_gid[gid], child_id[gid]);

				/* assign propreties for each child */
				new_hash[gid] = hash_;
				new_real[gid] = real_;
				new_imag[gid] = imag_;
				new_size_b[gid] = size_node_;
				new_size_c[gid] = size_sub_node_;
			}
		}

		/* !!!!!!!!!!!!!!!!
		step (5) 
		 !!!!!!!!!!!!!!!! */

		if (verbose > 0)
			std::cout << "step 5\n";
		
		/* sort graphs hash to compute interference */
		__gnu_parallel::sort(next_gid.begin(), next_gid.begin() + total_num_graphs, [&](unsigned int const &gid1, unsigned int const &gid2) {
			return new_hash[gid1] > new_hash[gid2];
		});

		/* compute is_first_index */
		is_first_index[next_gid[0]] = true;

		#pragma omp parallel
		{
			#pragma omp for
			for (unsigned int gid = 1; gid < total_num_graphs; ++gid)
				is_first_index[next_gid[gid]] = new_hash[next_gid[gid]] != new_hash[next_gid[gid - 1]];

			/* compute interferances */
			/* !!!!!!!!!!!!!!!!!!!!!!!!
			potentiel d'optimisation
			!!!!!!!!!!!!!!!!!!!!!!!! */
			#pragma omp for
			for (unsigned int gid = 0; gid < total_num_graphs; ++gid) {
				auto id = next_gid[gid];

				if (is_first_index[id])
					/* sum magnitude of equal graphs */
					for (unsigned int gid_ = gid + 1; gid_ < total_num_graphs && !is_first_index[next_gid[gid_]]; ++gid_) {
						auto id_ = next_gid[gid_];

						new_real[id] += new_real[id_];
						new_imag[id] += new_imag[id_];
					}
			}
		}

		/* get all graphs with a non zero probability */
		auto partitioned_it = __gnu_parallel::partition(next_gid.begin(), next_gid.begin() + total_num_graphs, [&](unsigned int const &gid) {
			/* check if graph is unique */
			if (!is_first_index[gid])
				return false;

			/* check for zero probability */
			auto r = new_real[gid];
			auto i = new_imag[gid];

			return r*r + i*i > tolerance; 
		});

		new_state.num_graphs = std::distance(next_gid.begin(), partitioned_it);
				
		/* !!!!!!!!!!!!!!!!
		step (5.1) 
		 !!!!!!!!!!!!!!!! */

		if (max_num_graphs > 0 && new_state.num_graphs > max_num_graphs) {

			if (verbose > 0)
				std::cout << "step 5.1\n";

			/* sort graphs according to probability */
			__gnu_parallel::nth_element(next_gid.begin(), next_gid.begin() + max_num_graphs, next_gid.begin() + new_state.num_graphs,
			[&](unsigned int const &gid1, unsigned int const &gid2) {
				auto r1 = new_real[gid1];
				auto i1 = new_imag[gid1];

				auto r2 = new_real[gid2];
				auto i2 = new_imag[gid2];

				return r1*r1 + i1*i1 > r2*r2 + i2*i2;
			});

			new_state.num_graphs = max_num_graphs;
		}

		/* !!!!!!!!!!!!!!!!
		step (6) 
		 !!!!!!!!!!!!!!!! */

		if (verbose > 0)
			std::cout << "step 6\n";

		/* sort to make memory access more continuous */
		__gnu_parallel::sort(next_gid.begin(), next_gid.begin() + new_state.num_graphs);

		/* resize new step variables */
		new_state.resize_num_graphs(new_state.num_graphs);

		/* prepare for partial sum */
		#pragma omp parallel for
		for (unsigned int gid = 0; gid <  new_state.num_graphs; ++gid) {
			unsigned int id = next_gid[gid];

			new_state.node_begin[gid + 1] = new_size_b[id];
			new_state.sub_node_begin[gid + 1] = new_size_c[id];

			/* assign magnitude */
			new_state.real[gid] = new_real[id];
			new_state.imag[gid] = new_imag[id];
		}

		/* compute the partial sums to get new node_begin and sub_node_begin */
		new_state.node_begin[0] = 0;
		__gnu_parallel::partial_sum(new_state.node_begin.begin() + 1, new_state.node_begin.begin() + new_state.num_graphs + 1, new_state.node_begin.begin() + 1);

		new_state.sub_node_begin[0] = 0;
		__gnu_parallel::partial_sum(new_state.sub_node_begin.begin() + 1, new_state.sub_node_begin.begin() + new_state.num_graphs + 1, new_state.sub_node_begin.begin() + 1);

		/* resize new step variables */
		new_state.resize_num_nodes(new_state.node_begin[new_state.num_graphs]);
		new_state.resize_num_sub_nodes(new_state.sub_node_begin[new_state.num_graphs]);

		/* !!!!!!!!!!!!!!!!
		step (7) 
		 !!!!!!!!!!!!!!!! */

		if (verbose > 0)
			std::cout << "step 7\n";

		#pragma omp parallel for
		for (unsigned int gid = 0; gid < new_state.num_graphs; ++gid) {
			auto id = next_gid[gid];

			/* populate graphs */
			rule.populate_new_graph(*this, new_state, gid, parent_gid[id], child_id[id]);
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

	for (int gid = 0; gid < s.num_graphs; ++gid) {
		std::cout << s.real[gid] << (s.imag[gid] >= 0 ? "+" : "") << s.imag[gid] << "i   ";

		unsigned int num_nodes_ = s.num_nodes(gid);
		for (unsigned short int node = 0; node < num_nodes_; ++node) {
			std::cout << "-|" << (s.left(gid, node) ? "<" : " ") << "|";

			print_node(gid, s.node_id(gid, node), false);

			std::cout << "|" << (s.right(gid, node) ? ">" : " ") << "|-";
		}

		std::cout << "\n";
	}
}
