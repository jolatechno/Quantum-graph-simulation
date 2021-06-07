#pragma once

#include <complex> //for complex
#include "../classical/graph.hpp"
#include <tbb/concurrent_unordered_map.h>
#include <tbb/concurrent_vector.h>
#include <boost/range/combine.hpp>
#include <map>

// forward definition of the state type
typedef class state state_t;

class state {
public:
	// tolerance
	long double tolerance = 1e-7;

	// hasher
	struct graph_hasher {
		size_t operator()(std::shared_ptr<graph_t> const &g) const {
			return g.get()->hash();
		}
	};

	// comparator
	struct graph_comparator {
		bool operator()(std::shared_ptr<graph_t> const &g1, std::shared_ptr<graph_t> const &g2) const {
			#ifndef SLOW_COMPARE
				return g1->hash() == g2->hash();
			#else
				if (g1->left != g2->left)
					return false;

				if (g1->right != g2->right)
					return false;

				for (auto [n1, n2] : boost::combine(g1->name().nodes(), g2->name().nodes()))
					if (n1.hash() != n2.hash())
						return false;

				return true;
			#endif
		}
	};

	// type definition
	typedef tbb::concurrent_unordered_multimap<std::shared_ptr<graph_t>, std::complex<long double>, graph_hasher, graph_comparator> graph_map_t;

private:
	// main list 
	graph_map_t graphs_;

	// check zero probas
	bool inline check_zero(const std::complex<long double>& mag) { return std::norm(mag) <= tolerance; }

public:
	// single graph constructor 
	state(graph_t* g) {
		// add graph 
		graphs_.insert({std::shared_ptr<graph_t>(g), {1, 0}});
	}

	//getter
	auto const &graphs() { return graphs_; }

	// reduce
	void reduce_all();

	// dynamic 
	void step_all(std::function<tbb::concurrent_vector<std::pair<std::shared_ptr<graph_t>, std::complex<long double>>>(std::shared_ptr<graph_t> const &g)> rule);

	// limit graphs
	void discard_all(size_t n_graphs);

	// normalize
	void normalize();
};

// insert operators 
void state::reduce_all() {
	#ifdef VERBOSE
		printf("reducing %ld graphs...\n", graphs_.size());
	#endif

    #ifndef SLOW_REDUCE
		graph_map_t buff; // faster, parallel reduce that uses WAY more ram
		buff.swap(graphs_);

		#pragma omp parallel
		#pragma omp single
	    for(auto it = buff.begin(); it != buff.end();) {
	    	// range of similar graphs to delete
	    	auto const graph = it->first;
	    	auto const range = buff.equal_range(graph);

	    	// next iteration
	    	it = range.second;

	    	#pragma omp task
	    	{
	    		std::complex<long double> acc = 0;
	    		for(auto jt = range.first; jt != range.second; ++jt)
	        		acc += jt->second;

		    	// if the first graphgs has a zero probability, erase the whole range
		    	if (!check_zero(acc))
		    		graphs_.insert({graph, acc});
	    	}
	    }
    #else
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
    #endif
}

void state::discard_all(size_t n_graphs) {
	if (graphs_.size() < n_graphs)
		return;

	/* compute sure threshold proba */
	long double threshold_proba = 0;

	std::map<long double, std::pair<std::shared_ptr<graph_t>, std::complex<long double>>> map;
	for (auto & [graph, mag] : graphs_) {
		long double proba = std::norm(mag);

		if (proba > threshold_proba) {
			map.insert({proba, {graph, mag}});

			if (map.size() == n_graphs + 1) {
				map.erase(map.begin());
				threshold_proba = map.begin()->first;
			}
		}
	}

	graphs_.clear();
	for (auto & [_, key] : map)
		graphs_.insert(key);
}

void state::normalize() {
	long double proba = 0;

	for (auto & [_, mag] : graphs_)
		proba += std::norm(mag);

	#pragma omp parallel
	#pragma omp single
	for (auto it = graphs_.begin(); it != graphs_.end(); ++it)
		#pragma omp task
		it->second /= proba;
}

// dynamic 
void state::step_all(std::function<tbb::concurrent_vector<std::pair<std::shared_ptr<graph_t>, std::complex<long double>>>(std::shared_ptr<graph_t> const &g)> rule) {
	graph_map_t buff;
	buff.swap(graphs_);

	#pragma omp parallel
	#pragma omp single
  	for (auto & [graph, mag] : buff)
	#pragma omp task
  	{
  		auto const graphs = rule(graph);

  		#pragma omp barrier
  		if (graph.hash() == buff.begin()->first.hash())
  			printf("created graphs..\n"); // suspecting error here

  		for (auto & [graph_, mag_] : graphs)
  		#pragma task
  		{
  			graphs_.insert({graph_, mag_ * mag});
  		}
  		#pragma omp barrier
  		if (graph.hash() == buff.begin()->first.hash())
  			printf("..OK\n"); // suspecting error here
  	}
}

//---------------------------------------------------------
// non member functions
//---------------------------------------------------------

//reader
void size_stat(state_t* s) {
	long double avg = 0;
	long double var = 0;
	long double correction_factor = 0;
	int numb = s->graphs().size();

	for (auto [graph, mag] : s->graphs()) {
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

void start_json(graph_t const* initial, char const* rule) {
	// print rule
	printf("{\n\t\"rule\" : \"%s\",", rule);

	// print initial state staitisic
	printf("\n\t\"initial state\" : {\n");
	printf("\t\t\"size\" : %ld\n", initial->size());
	printf("\t},\n\t\"iterations\" : [");

	// print first iteration statistic
	printf("\n\t\t{\n\t\t\t\"nums\" : [");
	for (int i = 0; i < initial->size(); ++i)
		printf("0, ");

	printf("1],\n\t\t\t\"probas\" : [");
	for (int i = 0; i < initial->size(); ++i)
		printf("0.0, ");

	printf("1.0]\n\t\t}");
}

void serialize_state_to_json(state_t* s) {
	// final vectors
	std::vector<unsigned short int> nums;
	std::vector<long double> probas;

	auto const graphs = s->graphs();
	for (auto & [graph, mag] : graphs) {
		size_t size = graph.get()->size();
		long double proba = std::norm(mag);

		if (size >= nums.size()) {
			nums.resize(size + 1, 0);
			probas.resize(size + 1, 0);
		}

		nums[size] += 1;
		probas[size] += proba;
	}

	// print number of graphs
	printf(",\n\t\t{\n\t\t\t\"nums\" : [%d", nums[0]);
	for (auto it = nums.begin() + 1; it < nums.end(); ++it)
		printf(", %d", *it);

	// print total probability
	printf("],\n\t\t\t\"probas\" : [%Lf", probas[0]);
	for (auto it = probas.begin() + 1; it < probas.end(); ++it)
		printf(", %Lf", *it);

	// print separator
	printf("]\n\t\t}");
}

void end_json() {
	printf("\n\t]\n}\n");
}

// for debugging 
void print(state_t *s) {
	for (auto & [graph, mag] : s->graphs()) {
		if (std::imag(mag) >= 0) {
	  		printf("%Lf + i%Lf   ", std::real(mag), std::imag(mag)); 
	  	} else {
	  		printf("%Lf - i%Lf   ", std::real(mag), -std::imag(mag));
	  	}
	  	print(graph.get());
	  	printf("\n");
	}
}