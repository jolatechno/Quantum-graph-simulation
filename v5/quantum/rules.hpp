#pragma once

#include <math.h>  //pow
#include <complex> //for complex
#include "../classical/graph.hpp"
#include "../classical/rules.hpp"
#include <tbb/concurrent_vector.h>
#include "state.hpp"

/* count the number of subsets */
template<class T>
int static num_subset(std::vector<T> const &vect) {
	return std::pow(2, vect.size());
}

/* create a subset and a probability from the subset indice */
std::pair<std::vector<graph::op_t>, std::complex<long double>> static subset(std::vector<graph::op_t>& split_merge, int subset_numb,
	std::complex<long double>& non_merge, std::complex<long double>& merge) {
	
	std::vector<graph::op_t> res;
	std::complex<long double> proba = 1;

	for (int i = 0; i < split_merge.size(); ++i) {
		long double sign = 1 - 2*(split_merge[i].second % 2);

		/* i_th bit of subset_numb */
		if (subset_numb%2) {
			/* add opperation to res */
			res.push_back(split_merge[i]);

			/* get proba */
			proba *= std::complex<long double>(merge.real(), sign*merge.imag());
		} else
			proba *= sign*non_merge;


		subset_numb /= 2;
	}

	return {res, proba};
}

//to create a reversible dynamic
std::pair<std::complex<long double>, std::complex<long double>> unitary(long double teta, long double phi) {
	std::complex<long double> non_merge_ = std::cos(teta);
	std::complex<long double> merge_ = std::polar(std::sin(teta), phi);
	return {non_merge_, merge_};
} 

// split merge a graph
auto split_merge_all(std::complex<long double>& non_merge, std::complex<long double>& merge) {
	return [&](state_t const *s, std::shared_ptr<graph_t> const &g) {
		tbb::concurrent_vector<std::pair<std::shared_ptr<graph_t>, std::complex<long double>>> graphs;

		// check for identity
		if (s->check_zero(merge)) {
			graphs.push_back({g, {1, 0}});
			return graphs;
		}

		// check for classical case
		if (s->check_zero(non_merge)) {
			split_merge(*g);
			graphs.push_back({g, {1, 0}});
			return graphs;
		}

		auto split_merge = get_split_merge(*g);
		#pragma omp parallel
		{
			#pragma omp task
		  	{
		  		// update the probability of the graph without split or merge 
				auto [_, mag_no_split_merge] = subset(split_merge, 0, non_merge, merge);
				graphs.push_back({g, mag_no_split_merge});
		  	}

			// add all graphs that actually have some split ot merge 
			const int n_max = num_subset(split_merge);
			for (int j = 1; j < n_max; ++j)
			#pragma omp task
			{
				std::shared_ptr<graph_t> g_ = std::make_shared<graph_t>(*g);

				auto [split_merge_list, mag_split_merge] = subset(split_merge, j, non_merge, merge);
				g_->split_merge(split_merge_list);
				graphs.push_back({g_, mag_split_merge});
			}
		}

		return graphs;
	};
}

// split merge a graph
auto erase_create_all(std::complex<long double>& non_create, std::complex<long double>& create) {
	return [&](state_t const *s, std::shared_ptr<graph_t> const &g) {
		tbb::concurrent_vector<std::pair<std::shared_ptr<graph_t>, std::complex<long double>>> graphs;

		// check for identity
		if (s->check_zero(create)) {
			graphs.push_back({g, {1, 0}});
			return graphs;
		}

		// check for classical case
		if (s->check_zero(non_create)) {
			erase_create(*g);
			graphs.push_back({g, {1, 0}});
			return graphs;
		}

		auto erase_create = get_erase_create(*g);
		#pragma omp parallel
		{
			#pragma omp task
		  	{
		  		// update the probability of the graph without split or merge 
				auto [_, mag_no_erase_create] = subset(erase_create, 0, non_create, create);
				graphs.push_back({g, mag_no_erase_create});
		  	}

			// add all graphs that actually have some split ot merge 
			const int n_max = num_subset(erase_create);
			for (int j = 1; j < n_max; ++j)
			#pragma omp task
			{
				std::shared_ptr<graph_t> g_ = std::make_shared<graph_t>(*g);

				auto [split_merge_list, mag_erase_create] = subset(erase_create, j, non_create, create);
				g_->erase_create(split_merge_list);
				graphs.push_back({g_, mag_erase_create});
			}
		}

		return graphs;
	};
}

//step and split merge
auto step_split_merge_all(std::complex<long double>& non_merge, std::complex<long double>& merge) {
	return [&](state_t const *s, std::shared_ptr<graph_t> const &g) {
		auto const split_merge_all_ = split_merge_all(non_merge, merge);
		g->step();
		return split_merge_all_(s, g);
	};
}

auto reversed_step_split_merge_all(std::complex<long double>& non_merge, std::complex<long double>& merge) {
	return [&](state_t const *s, std::shared_ptr<graph_t> const &g) {
		auto const split_merge_all_ = split_merge_all(non_merge, merge);
		auto graphs_ = split_merge_all_(s, g);

		#pragma parallel
		for (auto & [graph, _] : graphs_)
		#pragma task
			graph->reversed_step();
		
		return graphs_;
	};
}

//step and erase create
auto step_erase_create_all(std::complex<long double>& non_merge, std::complex<long double>& merge) {
	return [&](state_t const *s, std::shared_ptr<graph_t> const &g) {
		auto const erase_create_all_ = erase_create_all(non_merge, merge);
		g->step();
		return erase_create_all_(s, g);
	};
}

auto reversed_step_erase_create_all(std::complex<long double>& non_merge, std::complex<long double>& merge) {
	return [&](state_t const *s, std::shared_ptr<graph_t> const &g) {
		auto const erase_create_all_ = erase_create_all(non_merge, merge);
		auto graphs_ = erase_create_all_(s, g);

		#pragma parallel
		for (auto & [graph, _] : graphs_)
		#pragma task
			graph->reversed_step();
		
		return graphs_;
	};
}