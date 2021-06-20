#include "../src/rules.hpp"
#include "../src/utils/parser.hpp"

#include "../src/utils/debuging.hpp"

// debug levels
#define TEST_STEP_DEBUG_LEVEL 0.2
#define GRAPH_SIZE_DEBUG_LEVEL 0.1

#define _USE_MATH_DEFINES
#include <math.h>

int main(int argc, char* argv[]) {
	std::setvbuf(stdout, NULL, _IONBF, 0);

	cxxopts::Options options("check for quantum injectivity");
	auto [size, rule, _, n_iter, max_num_graphs, __, injectivity, ___] = parser(options, argc, argv);

	state s(size);
	s.randomize();

	print(s); std::cout << "\n";

	state new_state(1);

	for (int i = 0; i < n_iter; ++i) {
		if (verbose >= TEST_STEP_DEBUG_LEVEL)
			std::cout << "step...\n";
		
		s.step(new_state, *(rule_t*)rule, max_num_graphs);
		
		if (verbose >= TEST_STEP_DEBUG_LEVEL)
			std::cout << "...steped\nswap...\n";
		
		std::swap(s, new_state);
		
		if (verbose >= TEST_STEP_DEBUG_LEVEL)
			std::cout << "...swaped\nmove...\n";

		move_all(s);

		if (verbose >= TEST_STEP_DEBUG_LEVEL)
			std::cout << "...moved\n";
		if (verbose >= GRAPH_SIZE_DEBUG_LEVEL)
			std::cout << s.symbolic_iteration.num_graphs << " -> " << s.num_graphs << " graphs\n";
	}

	if (injectivity)
		for (int i = 0; i < n_iter; ++i) {
			if (verbose >= TEST_STEP_DEBUG_LEVEL)
				std::cout << "reversed move...\n";

			reversed_move_all(s);

			if (verbose >= TEST_STEP_DEBUG_LEVEL)
				std::cout << "...revresed moved\nstep...\n";
			
			s.step(new_state, *(rule_t*)rule, max_num_graphs);
			
			if (verbose >= TEST_STEP_DEBUG_LEVEL)
				std::cout << "...steped\nswap...\n";
			
			std::swap(s, new_state);
			
			if (verbose >= TEST_STEP_DEBUG_LEVEL)
				std::cout << "...swapped\n";

			if (verbose >= GRAPH_SIZE_DEBUG_LEVEL)
				std::cout << s.symbolic_iteration.num_graphs << " -> " << s.num_graphs << " graphs\n";
		}

	/* !!!!!!!!!!!!!!!
	!!!!!!!!!!!!!!!
	debuging
	!!!!!!!!!!!!!!!
	!!!!!!!!!!!!!!! */
	//print_vector(s.node_begin);
	//print_vector(s.sub_node_begin);
	/* !!!!!!!!!!!!!!!
	!!!!!!!!!!!!!!!
	debuging
	!!!!!!!!!!!!!!!
	!!!!!!!!!!!!!!! */

	print(s);
}