#pragma once

#include "../state.hpp"

void move_all(state_t &s) {
	//#pragma omp parallel for
	////#pragma omp parallel master
	for (unsigned int gid = 0; gid < s.numb_graphs; ++gid)
	////#pragma omp task
	{
		unsigned int end = s.b_begin[gid + 1];
		unsigned int begin = s.b_begin[gid];

		/* rotate particules */
		std::rotate(s.left_.begin() + begin, s.left_.begin() + begin + 1, s.left_.begin() + end);
		std::rotate(s.right_.begin() + begin, s.right_.begin() + end - 1, s.right_.begin() + end);
	}
}

void reversed_move_all(state_t &s) {
	//#pragma omp parallel for
	////#pragma omp parallel master
	for (unsigned int gid = 0; gid < s.numb_graphs; ++gid)
	////#pragma omp task
	{
		unsigned int end = s.b_begin[gid + 1];
		unsigned int begin = s.b_begin[gid];

		/* rotate particules */
		std::rotate(s.right_.begin() + begin, s.right_.begin() + begin + 1, s.right_.begin() + end);
		std::rotate(s.left_.begin() + begin, s.left_.begin() + end - 1, s.left_.begin() + end);
	}
}