#pragma once

#include <vector>
#include <utility>
#include <complex> //for complex
#include <string>
#include "../classical/graph.hpp"
#include "../classical/rules.hpp"
#include "rules.hpp"
#include <tbb/concurrent_unordered_map.h>

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

	// parameters
	std::complex<long double> non_merge_ = -1;
	std::complex<long double> merge_ = 0;

	// checker
	friend bool check(state_t* s);

public:
	// single graph constructor 
	state(graph_t* g) {
		// add graph 
		graphs_.insert({g, {1, 0}});
	}

	//getter
	auto const &graphs() { return graphs_; }

	//reader
	std::pair<long double, long double> size_stat();

	// setter 
	void set_params(long double teta, long double phi) {
		// compute amplitude 
		non_merge_ = std::cos(teta);
		merge_ = std::polar(std::sin(teta), phi);
	}

	void reduce_all();

	// dynamic 
	void step_split_merge_all(bool step, bool split_merge, bool reversed);
	void inline step_split_merge_all() { step_split_merge_all(true, true, false); }
	void inline step_all() { step_split_merge_all(true, false, false); }
	void inline split_merge_all() { step_split_merge_all(false, true, false); }
	void inline reversed_split_merge_step() {
		split_merge_all();
		step_split_merge_all(false, false, true);
	}

	// for debugging 
	void print();
};

// insert operators 
void state::reduce_all() {
	#ifdef VERBOSE
		printf("reducing %ld graphs...\n", graphs_.size());
	#endif

    for(auto it = graphs_.begin(); it != graphs_.end();) {
    	// range of similar graphs to delete
    	auto range = graphs_.equal_range(it->first);
    	auto start = range.first;

    	for(auto jt = std::next(it); jt != range.second; ++jt)
        	it->second += jt->second;

    	// if the first graphgs has a zero probability, erase the whole range
    	if (!check_zero(it->second))
    		++start;

    	it = graphs_.unsafe_erase(start, range.second);
    }
}

//reader
std::pair<long double, long double> state::size_stat() {
	long double avg = 0;
	long double var = 0;
	double long correction_factor = 0;
	int numb = graphs_.size();

	for (auto & [graph, mag] : graphs_) {
		long double size = graph->size();
		long double proba = std::norm(mag);
		avg += size*proba;
		var += size*size*proba;
		correction_factor += proba;
	}

	avg /= correction_factor; var /= correction_factor;
	var = std::sqrt(var - avg*avg);
	return {avg, var};
}

// dynamic 
void state::step_split_merge_all(bool step, bool split_merge, bool reversed) {
	graph_map_t buff;
	buff.swap(graphs_);

	#pragma omp parallel
	#pragma omp single
  	for (auto & [graph, mag] : buff)
	#pragma omp task
  	{
  		if (step)
  			graph->step();

  		if (reversed)
  			graph->reversed_step();

  		if (split_merge) {
  			auto split_merge = get_split_merge(graph);

  			#pragma omp task
  			{
  				// update the probability of the graph without split or merge 
				auto [_, mag_no_split_merge] = subset(split_merge, 0, non_merge_, merge_);
				graphs_.insert({graph, mag * mag_no_split_merge});
  			}

			// add all graphs that actually have some split ot merge 
			const int n_max = num_subset(split_merge);
			#pragma omp taskloop
			for (int j = 1; j < n_max; ++j) {
				graph_t* g_ = graph->copy();

				auto [split_merge_list, mag_split_merge] = subset(split_merge, j, non_merge_, merge_);
			  	g_->split_merge(split_merge_list);

			  	//add graph
			  	graphs_.insert({g_, mag * mag_split_merge});
			}
			
  		} else
	  		//add graph
			graphs_.insert({graph, mag});
  	}
}

// for debugging 
void state::print() {
	for (auto & [graph, mag] : graphs_) {
		if (std::imag(mag) >= 0) {
	  		printf("%Lf + i%Lf   ", std::real(mag), std::imag(mag)); 
	  	} else {
	  		printf("%Lf - i%Lf   ", std::real(mag), -std::imag(mag));
	  	}
	  	graph->print();
	  	printf("\n");
	}
}