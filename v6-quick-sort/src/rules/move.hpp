#pragma once

#include "../state.hpp"

void move_all(state_t &s) {
	#pragma omp parallel for schedule(static)
	for (size_t gid = 0; gid < s.current_iteration.num_graphs; ++gid) {
		size_t end = s.current_iteration.node_begin[gid + 1];
		size_t begin = s.current_iteration.node_begin[gid];

		/* rotate particules */
		std::rotate(s.current_iteration.left_.begin() + begin, s.current_iteration.left_.begin() + begin + 1, s.current_iteration.left_.begin() + end);
		std::rotate(s.current_iteration.right_.begin() + begin, s.current_iteration.right_.begin() + end - 1, s.current_iteration.right_.begin() + end);
	}
}

void reversed_move_all(state_t &s) {
	#pragma omp parallel for schedule(static)
	for (size_t gid = 0; gid < s.current_iteration.num_graphs; ++gid) {
		size_t end = s.current_iteration.node_begin[gid + 1];
		size_t begin = s.current_iteration.node_begin[gid];

		/* rotate particules */
		std::rotate(s.current_iteration.right_.begin() + begin, s.current_iteration.right_.begin() + begin + 1, s.current_iteration.right_.begin() + end);
		std::rotate(s.current_iteration.left_.begin() + begin, s.current_iteration.left_.begin() + end - 1, s.current_iteration.left_.begin() + end);
	}
}