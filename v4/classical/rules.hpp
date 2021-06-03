#pragma once

#include "graph.hpp"

// function to get all split and merges 
std::vector<graph::op_t> inline get_split_merge(graph_t const* graph) {
	std::vector<graph::op_t> split_merge;
	
	if (graph->right.empty() || graph->left.empty())
		return split_merge;

	unsigned int const last_idx = graph->size() - 1;

	auto left_it = graph->left.begin();
	auto right_it = graph->right.begin();
	bool first_or_last_split = *left_it == *right_it && *left_it == 0;
	first_or_last_split |= graph->left.back() == last_idx && graph->right.back() == last_idx;

	auto const left_end = graph->left.end();
	auto const right_end = graph->right.end();
	while (right_it < right_end && left_it < left_end)
		if (*left_it == *right_it) {
			// check for split
			split_merge.push_back({*left_it, graph->split_t});

			++right_it;
			++left_it;
		} else if (*left_it < *right_it) {
			// chedk for merges
			if (*left_it == *right_it - 1)
				if (left_it == left_end - 1) {
					split_merge.push_back({*left_it, graph->merge_t});
				} else if (*(left_it + 1) != *right_it)
					split_merge.push_back({*left_it, graph->merge_t});

			++left_it;
		} else
			++right_it;
		
	if (!first_or_last_split)
		if (graph->left.back() == last_idx && graph->right[0] == 0)
			split_merge.push_back({last_idx, graph->merge_t});

	return split_merge;
}

// split merge a graph
void split_merge(graph_t* graph) {
	auto split_merges = get_split_merge(graph);
	graph->split_merge(split_merges);
}

//function to get all void and split
std::vector<graph::op_t> inline get_erase_create(graph_t const* graph) {
	std::vector<graph::op_t> create_erase;

	unsigned int const last_idx = graph->size() - 1;

	auto left_it = graph->left.begin();
	auto right_it = graph->right.begin();

	unsigned int last_pos = 0;

	auto const left_end = graph->left.end();
	auto const right_end = graph->right.end();
	while (right_it < right_end && left_it < left_end) {
		// add create
		unsigned int next_last_pos = std::min(*left_it, *right_it);
		for (unsigned int i = last_pos; i < next_last_pos; ++i)
			create_erase.push_back({i, graph->create_t});

		// update lest_pos
		last_pos = next_last_pos + 1;

		if (*left_it == *right_it) {
			// check for erase
			create_erase.push_back({*left_it, graph->erase_t});

			++right_it;
			++left_it;
		} else {
			if (*left_it < *right_it) {
				++left_it;
			} else
				++right_it;
		}
	}
		
	// complete the unfinished list
	auto it = left_it == left_end ? right_it : left_it;
	auto const end = left_it == left_end ? right_end : left_end;
	for (;it < end; ++it) {
		// add create
		unsigned int next_last_pos = *it;
		for (unsigned int i = last_pos; i < next_last_pos; ++i)
			create_erase.push_back({i, graph->create_t});

		// update lest_pos
		last_pos = next_last_pos + 1;
	}
	
	// add create at the end
	unsigned int const size = graph->size();
	for (unsigned int i = last_pos; i < size; ++i)
		create_erase.push_back({i, graph->create_t});

	return create_erase;
}

// erase create a graph
void erase_create(graph_t* graph) {
	auto erase_creates = get_erase_create(graph);
	graph->erase_create(erase_creates);
}

/* for debugging */
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