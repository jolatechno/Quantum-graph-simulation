#pragma once

#include "graph.hpp"

/* to check */
bool inline graph_checker(graph_t* graph) {
	int size = graph->size();

	if (graph->left[0] < 0)  {
		printf("\nparticules going left 0 > %d\n", graph->left[0]);
		return false;
	}

	if (graph->left[0] >= size) {
		printf("\nparticules going left %d >= %d\n", graph->left[0], size);
		return false;
	}

	for (auto it = graph->left.begin() + 1; it < graph->left.end(); ++it) {
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

	if (graph->right[0] < 0)  {
		printf("\nparticules going right 0 > %d\n", graph->right[0]);
		return false;
	}

	if (graph->right[0] >= size) {
		printf("\nparticules going right %d > %d\n", graph->right[0], size);
		return false;
	}

	for (auto it = graph->right.begin() + 1; it < graph->right.end(); ++it)
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