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
			split_merge.push_back(std::pair<unsigned int, graph::split_merge_type_t>((*left_it), graph->split_t));

			/* check for last split */
			if (*right_it == last_idx)
				first_or_last_split = true;

			++right_it;
			++left_it;
		} else if (*left_it < *right_it) {
			/* chedk for merges */
			if (*left_it == *right_it - 1)
				if (left_it == last_left) {
					split_merge.push_back(std::pair<unsigned int, graph::split_merge_type_t>((*left_it), graph->merge_t));
				} else if (*(left_it + 1) != *right_it)
					split_merge.push_back(std::pair<unsigned int, graph::split_merge_type_t>((*left_it), graph->merge_t));

			++left_it;
		} else
			++right_it;
	}
		
	if (!first_or_last_split)
		if (*last_left == last_idx &&
		graph->right()[0] == 0)
			split_merge.push_back(std::pair<unsigned int, graph::split_merge_type_t>(last_idx, graph->merge_t));

	return split_merge;
}

/* to check */
bool inline graph_checker(graph_t* graph) {
	int size = graph->size();

	if (graph->left()[0] < 0)  {
		printf("\nparticules going left 0 > %d\n", graph->left()[0]);
		return false;
	}

	if (graph->left()[0] >= size) {
		printf("\nparticules going left %d >= %d\n", graph->left()[0], size);
		return false;
	}

	for (auto it = graph->left().begin() + 1; it < graph->left().end(); ++it) {
		if (*it <= *(it - 1)) {
			printf("\nparticules going left unsorted %d <= %d\n", *it, *(it - 1));
			return false;
		}

		if (*it < 0)  {
			printf("\nparticules going left 0 >= %d\n", *it);
			return false;
		}

		if (*it >= size) {
			printf("\nparticules going left %d > %d\n", *it, size);
			return false;
		}
	}

	if (graph->right()[0] < 0)  {
		printf("\nparticules going right 0 > %d\n", graph->right()[0]);
		return false;
	}

	if (graph->right()[0] >= size) {
		printf("\nparticules going right %d > %d\n", graph->right()[0], size);
		return false;
	}

	for (auto it = graph->right().begin() + 1; it < graph->right().end(); ++it)
		if (*it <= *(it - 1) || *it < 0 || *it >= size) {
			if (*it <= *(it - 1)) {
				printf("\nparticules going right unsorted %d <= %d\n", *it, *(it - 1));
				return false;
			}

			if (*it < 0)  {
				printf("\nparticules going right 0 > %d\n", *it);
				return false;
			}

			if (*it >= size) {
				printf("\nparticules going right %d > %d\n", *it, size);
				return false;
			}
		}

	return true;
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