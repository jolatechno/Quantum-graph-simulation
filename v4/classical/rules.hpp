#pragma once

#include "graph.hpp"
#include <stdio.h>
#include <utility> //for pairs
#include <vector>

/* function to get all split and merges */
std::vector<graph::split_merge_t> inline get_split_merge(graph_t* graph) {
	std::vector<graph::split_merge_t> split_merge;

	if (graph->left().size() == 0 ||
	graph->right().size() == 0)
		return split_merge;

	auto last_left = graph->left().end() - 1;
	auto last_right = graph->right().end() - 1;
	unsigned int last_idx = graph->size() - 1;

	auto left_it = graph->left().begin();
	auto right_it = graph->right().begin();
	bool first_or_last_split = *left_it == *right_it && *left_it == 0;
	while (true) {
		/* check if there are any particules left after iterating */
		if (right_it > last_right ||
		left_it > last_left)
			break;

		if (*left_it == *right_it) {
			/* check for split */
			split_merge.push_back({(*left_it), graph->split_t});

			/* check for last split */
			if (*right_it == last_idx)
				first_or_last_split = true;

			++right_it;
			++left_it;
		} else if (*left_it < *right_it) {
			/* chedk for merges */
			if (*left_it == *right_it - 1)
				if (left_it == last_left) {
					split_merge.push_back({(*left_it), graph->merge_t});
				} else if (*(left_it + 1) != *right_it)
					split_merge.push_back({(*left_it), graph->merge_t});

			++left_it;
		} else
			++right_it;
	}
		
	if (!first_or_last_split)
		if (*last_left == last_idx &&
		graph->right()[0] == 0)
			split_merge.push_back({last_idx, graph->merge_t});

	return split_merge;
}

/* for debugging */
void print_split_merge(std::vector<graph::split_merge_t>& split_merge) {
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