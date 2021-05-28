#pragma once

#include <vector>
#include <utility>
#include <complex> //for complex
#include "../classical/graph.hpp"
#include "../classical/rules.hpp"
#include "rules.hpp"

// forward definition of the state type
typedef class state state_t;

class state {
public:
	// type definition 
	typedef std::pair<graph_t*, std::complex<double>> graph_w_proba_t;

private:
	// main list 
	std::vector<graph_w_proba_t> graphs_;

	// parameters 
	std::complex<double> non_split_ = 1;
	std::complex<double> non_merge_ = -1;
	std::complex<double> merge_ = 0;
	std::complex<double> split_ = 0;

	//comparator
	bool quick_greater(graph_t* g1, graph_t* g2) {
		if (g1->left() > g2->left())
			return true;

		if (g1->left() < g2->left())
			return false;

		return g1->right() > g2->right();
	}

	// checker
	friend bool check(state_t* s);

public:
	// single graph constructor 
	state(graph_t* g) {
		// add graph 
		graphs_.push_back(std::pair<graph_t*, std::complex<double>>(g, std::complex<double>(1, 0)));
	}

	// setter 
	void set_params(double teta, double phi) {
		// compute amplitude 
		non_split_ = std::cos(teta);
		non_merge_ = -non_split_;

		double r = std::sin(teta);
		merge_ = std::polar(r, phi);
		split_ = std::polar(r, -phi);
	}

	// getters 
	std::vector<graph_w_proba_t> inline const &graphs() { return graphs_; }

	// insert operators 
	void inline insert_temp(graph_t* g, std::complex<double> proba) {
		std::pair<graph_t*, std::complex<double>> G(g, proba);
		graphs_.push_back(G);
	}
	void sort_and_reduce();

	// dynamic 
	void step_all();
	void split_merge_all();

	// for debugging 
	void print();
};

// insert operators 
void state::sort_and_reduce() {
	auto greater_than = [&] (graph_w_proba_t a, graph_w_proba_t b) {
		return quick_greater(a.first, b.first);
	};

	int blocks = 1;
	int size = graphs_.size();

	#ifdef _OPENMP
		// get the number of threads 
		int num_thread = 0;
		#pragma omp parallel reduction(+:num_thread)
		num_thread += 1;

		// get the number of iterations and blocks 
		while (blocks*2 <= num_thread)
			blocks *= 2;

		// get the size of each blocks 
		size = graphs_.size() / blocks;
		if (size < 1000) {
			blocks = 1;
			size = graphs_.size();
		}
	#endif

	#ifdef VERBOSE
		printf("sorting %d blocks of size %d\n", blocks, size);
	#endif

	auto begin = graphs_.begin();

	// sort per block 
	#ifdef _OPENMP
		#pragma omp parallel for
	#endif
	for (int i = 0; i < blocks; ++i) {
		auto end = (i == blocks - 1) ? graphs_.end() : begin + size*(i + 1);
		std::sort(begin + size*i, end, greater_than);
	}

	// recusively merge 
	while (blocks >= 2) {

		#ifdef VERBOSE
			printf("merging %d blocks of size %d\n", blocks, size);
		#endif

		// merge per block 
		#ifdef _OPENMP
			#pragma omp parallel for
		#endif
		for (int i = 0; i < blocks/2; ++i) {
			auto end = (i == blocks/2 - 1) ? graphs_.end() : begin + size*(2*i + 2);
			std::inplace_merge(begin + size*2*i, begin + size*(2*i + 1), end, greater_than);
		}

		blocks /= 2;
		size *= 2;
	}

	#ifdef VERBOSE
		printf("reducing %ld elements\n", graphs_.size());
	#endif

	// reducing graphs 
	for (auto it = graphs_.end() - 1; it >= begin; --it)
		// check for zero probability 
		if (std::norm((*it).second) <= double_tolerance) {
			graphs_.erase(it);
		} else
			for (auto other_it = it - 1; other_it >= begin; --other_it)
				if (quick_greater((*other_it).first, (*it).first)) {
					break;
				} else if ((*other_it).first->name()->equal((*it).first->name())) {
					// add probabilities 
					(*other_it).second += (*it).second;
					graphs_.erase(it);
					break;
				}
}

// dynamic 
void state::step_all() {
	#ifdef _OPENMP
		#pragma omp parallel for
	#endif
	for (auto it: graphs_)
		it.first->step();
}

void state::split_merge_all() {
	const int max_it = graphs_.size();
	#ifdef _OPENMP
		#pragma omp parallel
		#pragma omp single
		#pragma omp taskloop
	#endif
	for (int i = 0; i < max_it; ++i) {
		auto split_merge = get_split_merge(graphs_[i].first);

		// add all graphs that actually have some split ot merge 
		const int n_max = num_subset(split_merge);
		for (int j = 1; j < n_max; ++j) {
			graph_t* g_ = graphs_[i].first->copy();

			auto inter = subset(split_merge, j, non_merge_, non_split_, merge_, split_);
	  		g_->split_merge(inter.first);

			#ifdef _OPENMP
				#pragma omp critical
			#endif
	  		insert_temp(g_, graphs_[i].second * inter.second);
		}

		// update the probability of the graph without split or merge 
		auto proba = subset(split_merge, 0, non_merge_, non_split_, merge_, split_).second;
		graphs_[i].second *= proba;
	}
}

// for debugging 
void state::print() {
	for (int i = 0; i < graphs_.size(); ++i) {
		auto proba = graphs_[i].second;
		if (imag(proba) >= 0) {
	  		printf("%f + i%f   ", real(proba), imag(proba)); 
	  	} else {
	  		printf("%f - i%f   ", real(proba), -imag(proba));
	  	}
	  	graphs_[i].first->print();
	  	printf("\n");
	}
}