#include "../src/rules.hpp"
#include "../src/utils/parser.hpp"

// debug levels
#define TEST_STEP_DEBUG_LEVEL 0.5
#define GRAPH_SIZE_DEBUG_LEVEL 0.1

#define _USE_MATH_DEFINES
#include <math.h>

int main(int argc, char* argv[]) {
	std::setvbuf(stdout, NULL, _IONBF, 0);

	cxxopts::Options options("check for quantum injectivity");
	auto [size, rule, _, n_iter, max_num_graphs, __, injectivity, ___] = parser(options, argc, argv);

	state s(size);
	s.randomize();

	move_all(s);
	print(s); std::cout << "\n";

	state new_state(1); //10000, 10000, 10000, 10000);

	for (int i = 0; i < n_iter; ++i) {
		//state new_state(1);

		if (verbose >= TEST_STEP_DEBUG_LEVEL)
			std::cout << "move...\n";

		move_all(s);
		
		if (verbose >= TEST_STEP_DEBUG_LEVEL)
			std::cout << "...moved\nstep...\n";
		
		s.step(new_state, *(rule_t*)rule, max_num_graphs);
		
		if (verbose >= TEST_STEP_DEBUG_LEVEL)
			std::cout << "...steped\nswap...\n";
		
		std::swap(s, new_state);
		
		if (verbose >= TEST_STEP_DEBUG_LEVEL)
			std::cout << "...swaped\n";

		if (verbose >= GRAPH_SIZE_DEBUG_LEVEL)
			std::cout << s.num_graphs << " graphs\n";
	}

	if (injectivity)
		for (int i = 0; i < n_iter; ++i) {
			//state new_state(1);

			if (verbose >= TEST_STEP_DEBUG_LEVEL)
				std::cout << "step...\n";
			
			s.step(new_state, *(rule_t*)rule, max_num_graphs);
			
			if (verbose >= TEST_STEP_DEBUG_LEVEL)
				std::cout << "...steped\nswap...\n";
			
			std::swap(s, new_state);
			
			if (verbose >= TEST_STEP_DEBUG_LEVEL)
				std::cout << "...swapped\nmoove...\n";
			
			reversed_move_all(s);
			
			if (verbose >= TEST_STEP_DEBUG_LEVEL)
				std::cout << "...mooved\n";

			if (verbose >= GRAPH_SIZE_DEBUG_LEVEL)
				std::cout << s.num_graphs << " graphs\n";
		}

	//std::cout << "print...\n";
	print(s);
}