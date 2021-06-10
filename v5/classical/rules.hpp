#pragma once

#include "graph.hpp"

// function to get all split and merges 
std::vector<graph::op_t> inline get_split_merge(graph_t const &graph) {
	std::vector<graph::op_t> split_merges;
	
	auto const last_idx_ = graph.size() - 1;
	for (int i = 0; i <= last_idx_; ++i)
		if (graph.left[i] && graph.right[i]) {
			split_merges.push_back({i, graph.split_t});
		} else {
			// next pos
			unsigned short int next_pos = i == last_idx_ ? 0 : i + 1; 

			if (graph.left[i] && graph.right[next_pos] && !graph.left[next_pos])
				split_merges.push_back({i, graph.merge_t});
		}

	return split_merges;
}

// split merge a graph
void split_merge(graph_t &graph) {
	auto split_merges = get_split_merge((graph_t const &)graph);
	graph.split_merge(split_merges);
}

//function to get all void and split
std::vector<graph::op_t> inline get_erase_create(graph_t const &graph) {
	std::vector<graph::op_t> create_erases;

	for (int i = 0; i < graph.size(); ++i)
		if (graph.left[i] && graph.right[i]) {
			create_erases.push_back({i, graph.erase_t});
		} else if (!graph.left[i] && !graph.right[i])
			create_erases.push_back({i, graph.create_t});

	return create_erases;
}

// erase create a graph
void erase_create(graph_t &graph) {
	auto erase_creates = get_erase_create((graph_t const &)graph);
	graph.erase_create(erase_creates);
}



//---------------------------------------------------------
// debugging functions
//---------------------------------------------------------

void print_split_merge(std::vector<graph::op_t>& split_merge) {
	graph_t* graph;

	printf("merge at indexes ");
	for (auto & it : split_merge)
		if (it.second == graph->merge_t)
			printf("%u, ", it.first);

	printf("\nsplit at indexes ");
	for (auto & it : split_merge)
		if (it.second == graph->split_t)
			printf("%u, ", it.first);
}

/* for debugging */
void print_erase_create(std::vector<graph::op_t>& create_erase) {
	graph_t* graph;

	printf("create at indexes ");
	for (auto & it : create_erase)
		if (it.second == graph->create_t)
			printf("%u, ", it.first);

	printf("\nerase at indexes ");
	for (auto & it : create_erase)
		if (it.second == graph->erase_t)
			printf("%u, ", it.first);
}