#pragma once

#include <vector>
#include <utility>
#include <functional>
#include <complex> //for complex
#include <string>
#include "../classical/graph.hpp"
#include "rules.hpp"
#include <tbb/concurrent_unordered_map.h>
#include <tbb/concurrent_vector.h>

// forward definition of the state type
typedef class state state_t;

class state {
public:
	// hasher
	struct graph_hasher {
		size_t operator()(graph_t* const &g) const {
			return g->hash();
		}
	};

	// comparator
	struct graph_comparator {
		bool operator()(graph_t* const &g1, graph_t* const &g2) const {
			return g1->hash() == g2->hash();
		}
	};

	// type definition
	typedef tbb::concurrent_unordered_multimap<graph_t*, std::complex<long double>, graph_hasher, graph_comparator> graph_map_t;

private:
	// main list 
	graph_map_t graphs_;

public:
	// single graph constructor 
	state(graph_t* g) {
		// add graph 
		graphs_.insert({g, {1, 0}});
	}

	//getter
	auto const &graphs() { return graphs_; }

	// reduce
	void reduce_all();

	// dynamic 
	void step_all(std::function<tbb::concurrent_vector<std::pair<graph_t*, std::complex<long double>>>(graph_t* g)> rule);
};

// insert operators 
void state::reduce_all() {
	#ifdef VERBOSE
		printf("reducing %ld graphs...\n", graphs_.size());
	#endif

    for(auto it = graphs_.begin(); it != graphs_.end();) {
    	// range of similar graphs to delete
    	auto upper = graphs_.equal_range(it->first).second;

    	for(auto jt = std::next(it); jt != upper; ++jt)
        	it->second += jt->second;

    	// if the first graphgs has a zero probability, erase the whole range
    	if (!check_zero(it->second))
    		++it;
    	
    	it = graphs_.unsafe_erase(it, upper);
    }
}

// dynamic 
void state::step_all(std::function<tbb::concurrent_vector<std::pair<graph_t*, std::complex<long double>>>(graph_t* g)> rule) {
	graph_map_t buff;
	buff.swap(graphs_);

	#pragma omp parallel
	#pragma omp single
  	for (auto & [graph, mag] : buff)
	#pragma omp task
  	{
  		auto graphs = rule(graph);
  		for (auto & [graph_, mag_] : graphs)
  		#pragma task
  		{
  			graphs_.insert({graph_, mag_ * mag});
  		}
  	}
}

//---------------------------------------------------------
// non member functions
//---------------------------------------------------------

//reader
void size_stat(state_t* s) {
	long double avg = 0;
	long double var = 0;
	double long correction_factor = 0;
	int numb = s->graphs().size();

	for (auto & [graph, mag] : s->graphs()) {
		long double size = graph->size();
		long double proba = std::norm(mag);
		avg += size*proba;
		var += size*size*proba;
		correction_factor += proba;
	}

	avg /= correction_factor; var /= correction_factor;
	var = var - avg*avg <= 0 ? 0 : std::sqrt(var - avg*avg);

	printf("%Lf±%Lf", avg, var);
}

// for debugging 
void print(state_t* const s) {
	for (auto & [graph, mag] : s->graphs()) {
		if (std::imag(mag) >= 0) {
	  		printf("%Lf + i%Lf   ", std::real(mag), std::imag(mag)); 
	  	} else {
	  		printf("%Lf - i%Lf   ", std::real(mag), -std::imag(mag));
	  	}
	  	print(graph);
	  	printf("\n");
	}
}