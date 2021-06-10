#pragma once

#include <complex> //for complex
#include "../classical/graph.hpp"
#include <tbb/concurrent_unordered_map.h>
#include <tbb/concurrent_vector.h>
#include <boost/range/combine.hpp>
#include <map>
#include <random>
#include <iostream>

// forward definition of the state type
typedef class state state_t;

// mpfr
#ifdef USE_MPRF
	//import
	#include "../utils/mpreal.h"
	
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

class state {
public:
	// tolerance
	PROBA_TYPE tolerance = 0;

	// hasher
	struct graph_hasher {
		size_t operator()(std::shared_ptr<graph_t> const &g) const {
			return g.get()->hash();
		}
	};

	// comparator
	struct graph_comparator {
		bool operator()(std::shared_ptr<graph_t> const &g1, std::shared_ptr<graph_t> const &g2) const {
			return g1->hash() == g2->hash();
		}
	};

	// type definition
	typedef std::complex<PROBA_TYPE> mag_t;
	typedef tbb::concurrent_unordered_multimap<std::shared_ptr<graph_t>, mag_t, graph_hasher, graph_comparator> graph_map_t;
	typedef std::function<tbb::concurrent_vector<std::pair<std::shared_ptr<graph_t>, mag_t>>(state_t const *s, std::shared_ptr<graph_t> const &g)> rule_t;

	// check zero probas
	bool inline check_zero(const mag_t& mag) const { return std::norm(mag) <= tolerance; }

private:
	// main list 
	graph_map_t graphs_;

public:
	// constructor
	state() {}

	state(graph_t &g) {
		// add graph 
		graphs_.insert({std::make_shared<graph_t>(g), {1, 0}});
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
	#ifdef VERBOSE
		printf("reducing %ld graphs...\n", graphs_.size());
	#endif

    graph_map_t buff; // faster, parallel reduce that uses WAY more ram
	buff.swap(graphs_);

	#pragma omp parallel
	#pragma omp master
	for(auto it = buff.begin(); it != buff.end();) {
	    // range of similar graphs to delete
	    auto const graph = it->first;
	    auto const range = buff.equal_range(graph);

	    // next iteration
	    it = range.second;

	    #pragma omp task
	    {
	    	mag_t acc = {0, 0};
	    	for(auto jt = range.first; jt != range.second; ++jt)
	        	acc += jt->second;

		    // if the first graphgs has a zero probability, erase the whole range
		    if (!check_zero(acc))
		    	graphs_.insert({graph, acc});
	    }
	}
}

// use std::partition !!
void state::discard_all(size_t n_graphs) {
	if (graphs_.size() <= n_graphs)
		return;

	// create map full of vector element
	std::vector<std::pair<std::shared_ptr<graph_t>, mag_t>> vect(graphs_.begin(), graphs_.end());

	// find nth largest element
	std::ranges::nth_element(vect.begin(), vect.begin() + n_graphs, vect.end(),
		[](std::pair<std::shared_ptr<graph_t>, mag_t> const &it1,
			std::pair<std::shared_ptr<graph_t>, mag_t> const &it2) {
			return  std::norm(it1.second) > std::norm(it2.second);
		});

	// insert into graphs_
	graphs_.clear();
	graphs_.insert(vect.begin(), vect.begin() + n_graphs);
}

void state::normalize() {
	PROBA_TYPE proba = 0;

	for (auto & [_, mag] : graphs_)
		proba += std::norm(mag);

	proba = precision::sqrt(proba);

	#pragma omp parallel
	#pragma omp single
	for (auto it = graphs_.begin(); it != graphs_.end(); ++it)
		#pragma omp task
		it->second /= proba;
}

// dynamic
void state::step_all(rule_t rule) {
	graph_map_t buff;
	buff.swap(graphs_);

	#pragma omp parallel
	#pragma omp master
  	for (auto & [graph, mag] : buff)
	#pragma omp task
  	{
  		auto const graphs = rule(this, graph);

  		for (auto & [graph_, mag_] : graphs)
  		//#pragma task
  			graphs_.insert({graph_, mag_ * mag});
  	}
}

// randomize
void state::randomize(unsigned short int min_graph_size, unsigned short int max_graph_size, unsigned short int num_graphs) {
	auto const random_complex = [&]() {
		PROBA_TYPE real = static_cast <PROBA_TYPE> (rand()) / static_cast <PROBA_TYPE> (RAND_MAX) - 0.5;
		PROBA_TYPE imag = static_cast <PROBA_TYPE> (rand()) / static_cast <PROBA_TYPE> (RAND_MAX) - 0.5;
		return mag_t(real, imag);
	};

	for (int size = min_graph_size; size < max_graph_size; ++size)
		for (int j = 0; j < num_graphs; ++j) {
			graph_t g(size);
			g.randomize();
			graphs_.insert({std::make_shared<graph_t>(g), random_complex()});
		}

	reduce_all();
	normalize();
}

void state::zero_randomize(unsigned short int min_graph_size, unsigned short int max_graph_size) {
	auto const random_complex = [&]() {
		PROBA_TYPE real = static_cast <PROBA_TYPE> (rand()) / static_cast <PROBA_TYPE> (RAND_MAX) - 0.5;
		PROBA_TYPE imag = static_cast <PROBA_TYPE> (rand()) / static_cast <PROBA_TYPE> (RAND_MAX) - 0.5;
		return mag_t(real, imag);
	};

	for (int size = min_graph_size; size < max_graph_size; ++size) {
		graph_t g(size);
		graphs_.insert({std::make_shared<graph_t>(g), random_complex()});
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

	for (auto [graph, mag] : s->graphs()) {
		PROBA_TYPE size = graph->size();
		PROBA_TYPE proba = std::norm(mag);
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
	for (auto & [graph, mag] : graphs) {
		size_t size = graph.get()->size();
		PROBA_TYPE proba = std::norm(mag);

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
	for (auto & [graph, mag] : s->graphs()) {
		if (std::imag(mag) >= 0) {
	  		std::cout << std::real(mag) << " + i" << std::imag(mag) << "  "; 
	  	} else
	  		std::cout << std::real(mag) << " - i" << std::imag(mag) << "  "; 

	  	print(*graph);
	  	std::cout << "\n";
	}
}