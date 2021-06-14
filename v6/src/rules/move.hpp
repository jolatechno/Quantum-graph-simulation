#pragma once

#include "../state.hpp"

void move_all(state_t &s) {
	size_t numb_graphs = s.real.size();
	#pragma omp parallel for
	for (unsigned int gid = 0; gid < numb_graphs; ++gid) {
		unsigned int numb_nodes_ = s.numb_nodes(gid);
		unsigned int begin = s.b_begin[gid];

		/* rotate particules */
		std::rotate(s.left_.begin() + begin, s.left_.begin() + begin + 1, s.left_.begin() + begin + numb_nodes_);
		std::rotate(s.right_.begin() + begin, s.right_.begin() + begin + numb_nodes_ - 1, s.right_.begin() + begin + numb_nodes_);
	}
}

void reversed_move_all(state_t &s) {
	size_t numb_graphs = s.real.size();
	#pragma omp parallel for
	for (unsigned int gid = 0; gid < numb_graphs; ++gid) {
		unsigned int numb_nodes_ = s.numb_nodes(gid);
		unsigned int begin = s.b_begin[gid];

		/* rotate particules */
		std::rotate(s.right_.begin() + begin, s.right_.begin() + begin + 1, s.right_.begin() + begin + numb_nodes_);
		std::rotate(s.left_.begin() + begin, s.left_.begin() + begin + numb_nodes_ - 1, s.left_.begin() + begin + numb_nodes_);
	}
}