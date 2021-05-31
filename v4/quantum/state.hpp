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
	// type definition 
	typedef std::pair<graph_t*, std::complex<long double>> graph_w_proba_t;

private:
	// main list 
	tbb::concurrent_unordered_multimap<std::string, graph_w_proba_t> graphs_;

	// parameters
	std::complex<long double> non_merge_ = -1;
	std::complex<long double> merge_ = 0;

	// checker
	friend bool check(state_t* s);
	friend bool full_check(state_t* s);

public:
	// single graph constructor 
	state(graph_t* g) {
		// add graph 
		insert_temp(g, {1, 0});
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

	// insert operators 
	void inline insert_temp(graph_t* g, std::complex<long double> proba) {
		auto k = g->hash();
		std::pair<graph_t*, std::complex<long double>> v(g, proba);
		graphs_.insert({k, v});
	}
	void reduce_all();

	// dynamic 
	void step_split_merge_all(bool step, bool split_merge);
	void step_split_merge_all() { step_split_merge_all(true, true); }
	void step_all() { step_split_merge_all(true, false); }
	void split_merge_all() { step_split_merge_all(false, true); }

	// for debugging 
	void print();
};

// insert operators 
void state::reduce_all() {
	#ifdef VERBOSE
		printf("reducing %ld graphs...\n", graphs_.size());
	#endif

	// Select the correct type for calling the equal_range function
    decltype(graphs_.equal_range("")) range;

    // iterate through multimap's elements (by key)
	#pragma omp parallel
	#pragma omp single
    for(auto it = graphs_.begin(); it != graphs_.end(); it = range.second) {
        // Get the range of the current key
        range = graphs_.equal_range(it->first);

        // save the range
        auto const jt = range.first;
        const auto local_end = range.second;
        
        // next iterator
        auto kt = jt;
        ++kt;

        // Now sum out that whole range
		#pragma omp task
        for(; kt != local_end; ++kt)
        	jt->second.second += kt->second.second;
    }

    #ifdef VERBOSE
		printf("erasing graphs...\n");
	#endif

    /* erase zero probability graphs */
    for(auto it = graphs_.begin(); it != graphs_.end();) {
    	/* range of similar graphs to delete */
    	range = graphs_.equal_range(it->first);
    	auto start = range.first;

    	/* if the first graphgs has a zero probability, erase the whole rang */
    	if (!check_zero(it->second.second))
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

	for (auto & [_, it] : graphs_) {
		long double size = it.first->size();
		long double proba = std::norm(it.second);
		avg += size*proba;
		var += size*size*proba;
		correction_factor += proba;
	}

	avg /= correction_factor; var /= correction_factor;
	var = std::sqrt(var - avg*avg);
	return {avg, var};
}

// dynamic 
void state::step_split_merge_all(bool step, bool split_merge) {
	tbb::concurrent_unordered_multimap<std::string, graph_w_proba_t> buff;
	buff.swap(graphs_);

	#pragma omp parallel
	#pragma omp single
  	for (auto & [_, it] : buff)
	#pragma omp task
  	{
  		if (step)
  			it.first->step();

  		if (split_merge) {
  			auto split_merge = get_split_merge(it.first);

			// add all graphs that actually have some split ot merge 
			const int n_max = num_subset(split_merge);
			for (int j = 1; j < n_max; ++j) {
				graph_t* g_ = it.first->copy();

				auto inter = subset(split_merge, j, non_merge_, merge_);
			  	g_->split_merge(inter.first);

			  	//add graph
			  	insert_temp(g_, it.second * inter.second);
			}
			// update the probability of the graph without split or merge 
			auto proba = subset(split_merge, 0, non_merge_, merge_).second;
			it.second *= proba;
  		}
  		
  		//add graph
		insert_temp(it.first, it.second);
  	}
}

// for debugging 
void state::print() {
	for (auto & [_, it] : graphs_) {
		auto proba = it.second;
		if (std::imag(proba) >= 0) {
	  		printf("%Lf + i%Lf   ", std::real(proba), std::imag(proba)); 
	  	} else {
	  		printf("%Lf - i%Lf   ", std::real(proba), -std::imag(proba));
	  	}
	  	it.first->print();
	  	printf("\n");
	}
}