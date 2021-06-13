#pragma once

#include <complex> //for complex
#include "../classical/graph.hpp"
#include <tbb/concurrent_unordered_set.h>
#include <tbb/concurrent_vector.h>
#include <boost/range/combine.hpp>
#include <map>
#include <random>
#include <iostream>

// forward definition of the state type
typedef class state state_t;

bool slow_comparisons = false;

class state {
public:
	// hasher
	struct graph_hasher {
		size_t operator()(std::shared_ptr<graph_t> const &g) const {
			return g->hash();
		}
	};

	// comparator
	struct graph_comparator {
		bool operator()(std::shared_ptr<graph_t> const &g1, std::shared_ptr<graph_t> const &g2) const {
			if (slow_comparisons) {
				if (g1->size() != g2->size())
					return false;

				if (g1->name().hash() != g2->name().hash())
					return false;

				if (g1->left != g2->left)
					return false;

				return g1->right == g2->right;

			} else 
				return g1->hash() == g2->hash();
		}
	};

	// type definition
	typedef tbb::concurrent_unordered_multiset<std::shared_ptr<graph_t>, graph_hasher, graph_comparator> graph_map_t;
	typedef std::function<std::vector<std::shared_ptr<graph_t>>(std::shared_ptr<graph_t> const &g)> rule_t;

private:
	// main list 
	graph_map_t graphs_;

public:
	// constructor
	state() {}

	state(graph_t &g) {
		// add graph 
		graphs_.insert(std::make_shared<graph_t>(g));
	}

	//getter
	auto const &graphs() const { return graphs_; }

	// reduce
	void reduce_all();

	// dynamic 
	void step_all(rule_t rule);

	// limit graphs
	void discard_all(size_t n_graphs);

	// normalize
	void normalize();

	// randomize
	void randomize(unsigned short int min_graph_size, unsigned short int max_graph_size, unsigned short int num_graphs);
	void zero_randomize(unsigned short int min_graph_size, unsigned short int max_graph_size);
};

// insert operators 
void state::reduce_all() {
	if (verbose)
		printf("reducing %ld graphs...\n", graphs_.size());

    graph_map_t buff; // faster, parallel reduce that uses WAY more ram
	buff.swap(graphs_);

	#pragma omp parallel
	#pragma omp master
	for(auto it = buff.begin(); it != buff.end();) {
	    // range of similar graphs to delete
	    auto graph = *it;
	    auto const range = buff.equal_range(graph);

	    // next iteration
	    it = range.second;

	    #pragma omp task
	    {
	    	for(auto jt = std::next(range.first); jt != range.second; ++jt)
	        	graph->add_magnitude((*jt)->mag);

		    // if the first graphgs has a zero probability, erase the whole range
		    if (!graph->check_zero())
		    	graphs_.insert(graph);
	    }
	}

	if (verbose)
		printf("...to %ld graphs\n", graphs_.size());
}

// use std::partition !!
void state::discard_all(size_t n_graphs) {
	if (graphs_.size() <= n_graphs)
		return;

	// create map full of vector element
	std::vector<std::shared_ptr<graph_t>> vect(graphs_.begin(), graphs_.end());

	// find nth largest element
	std::ranges::nth_element(vect.begin(), vect.begin() + n_graphs, vect.end(),
		[](std::shared_ptr<graph_t> const &it1, std::shared_ptr<graph_t> const &it2) {
			return  it1->norm() > it2->norm();
		});

	// insert into graphs_
	graphs_.clear();
	graphs_.insert(vect.begin(), vect.begin() + n_graphs);
}

void state::normalize() {
	PROBA_TYPE proba = 0;

	for (auto & graph : graphs_)
		proba += graph->norm();

	#ifndef PROBABILIST
		proba = precision::sqrt(proba);
	#endif

	#pragma omp parallel
	#pragma omp master
	for (auto it = graphs_.begin(); it != graphs_.end(); ++it)
		#pragma omp task
		(*it)->normalize_magnitude(proba);
}

// dynamic
void state::step_all(rule_t rule) {
	graph_map_t buff;
	buff.swap(graphs_);

	#pragma omp parallel
	#pragma omp master
  	for (auto & graph : buff)
	#pragma omp task
  	{
  		auto const graphs = rule(graph);
		graphs_.insert(graphs.begin(), graphs.end());
  			
  	}
}

// randomize
void state::randomize(unsigned short int min_graph_size, unsigned short int max_graph_size, unsigned short int num_graphs) {
	for (int size = min_graph_size; size < max_graph_size; ++size)
		for (int j = 0; j < num_graphs; ++j) {
			graph_t g(size);
			g.randomize();
			g.mag_randomize();
			graphs_.insert(std::make_shared<graph_t>(g));
		}

	reduce_all();
	normalize();
}

void state::zero_randomize(unsigned short int min_graph_size, unsigned short int max_graph_size) {
	for (int size = min_graph_size; size < max_graph_size; ++size) {
		graph_t g(size);
		g.mag_randomize();
		graphs_.insert(std::make_shared<graph_t>(g));
	}

	normalize();
}

//---------------------------------------------------------
// non member functions
//---------------------------------------------------------

//reader
void size_stat(state_t* s) {
	PROBA_TYPE avg = 0;
	PROBA_TYPE var = 0;
	PROBA_TYPE correction_factor = 0;
	int numb = s->graphs().size();

	for (auto graph : s->graphs()) {
		PROBA_TYPE size = graph->size();
		PROBA_TYPE proba = graph->norm();
		avg += size*proba;
		var += size*size*proba;
		correction_factor += proba;
	}

	avg /= correction_factor; var /= correction_factor;
	var = var - avg*avg <= (PROBA_TYPE)0 ? (PROBA_TYPE)0 : precision::sqrt(var - avg*avg);

	std::cout << avg << "±" << var;
}

void serialize_state_to_json(state_t const *s, bool first) {
	if (!first)
		std::cout << ",";

	// final vectors
	std::vector<unsigned short int> nums{0};
	std::vector<PROBA_TYPE> probas{0.};

	auto const graphs = s->graphs();
	for (auto &  graph : graphs) {
		size_t size = graph.get()->size();
		PROBA_TYPE proba = graph->norm();

		if (size >= nums.size()) {
			nums.resize(size + 1, 0);
			probas.resize(size + 1, 0);
		}

		nums[size] += 1;
		probas[size] += proba;
	}

	// print number of graphs
	std::cout << "\n\t\t{\n\t\t\t\"nums\" : [" << nums[0];
	for (auto it = nums.begin() + 1; it < nums.end(); ++it)
		std::cout << ", " << *it;

	// print total probability
	std::cout << "],\n\t\t\t\"probas\" : [" << probas[0];
	for (auto it = probas.begin() + 1; it < probas.end(); ++it)
		std::cout << ", " << *it;

	// print separator
	printf("]\n\t\t}");
}

void start_json(state_t const *s, char const* rule) {
	// print rule
	std::cout << "{\n\t\"rule\" : \"" << rule << "\",";

	std::cout << "\n\t\"iterations\" : [";
	serialize_state_to_json(s, true);
}

void serialize_state_to_json(state_t const *s) {
	serialize_state_to_json(s, false);
}

void end_json() {
	std::cout << "\n\t]\n}\n";
}

// for debugging 
void print(state_t *s) {
	for (auto & graph : s->graphs()) {
		#ifndef PROBABILIST
			if (graph->mag.imag() >= 0) {
		  		std::cout << graph->mag.real() << " + i" << graph->mag.imag() << "  "; 
		  	} else
		  		std::cout << graph->mag.real() << " - i" << graph->mag.imag() << "  "; 
		#else
		  	std::cout << graph->mag << "  ";
		#endif

	  	print(*graph);
	  	std::cout << "\n";
	}
}