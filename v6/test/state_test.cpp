#include "../src/rules.hpp"
#include "../src/utils/parser.hpp"

// debug levels
#define TEST_STEP_DEBUG_LEVEL 0.2
#define GRAPH_SIZE_DEBUG_LEVEL 0.1

#define _USE_MATH_DEFINES
#include <math.h>

int main(int argc, char* argv[]) {
	std::setvbuf(stdout, NULL, _IONBF, 0);

	cxxopts::Options options("check for quantum injectivity");
	auto [s, rule, reversed_rule, n_iter, n_reversed_iteration, normalize] = test_parser(options, argc, argv);

	print(s); std::cout << "\n";

	for (int i = 0; i < n_iter; ++i) {
		if (verbose >= TEST_STEP_DEBUG_LEVEL)
			std::cout << "step...\n";
		
		s.step(*rule, normalize);
		
		if (verbose >= TEST_STEP_DEBUG_LEVEL)
			std::cout << "...steped\nswap...\n";
		
		if (verbose >= TEST_STEP_DEBUG_LEVEL)
			std::cout << "...swaped\nmove...\n";

		move_all(s);

		if (verbose >= TEST_STEP_DEBUG_LEVEL)
			std::cout << "...moved\n";

		if (verbose >= GRAPH_SIZE_DEBUG_LEVEL)
			std::cout << s.symbolic_num_graphs << " -> " << s.current_iteration.num_graphs << " graphs\n";
	}

	for (int i = 0; i < n_reversed_iteration; ++i) {
		if (verbose >= TEST_STEP_DEBUG_LEVEL)
			std::cout << "reversed move...\n";
		
		reversed_move_all(s);

		if (verbose >= TEST_STEP_DEBUG_LEVEL)
			std::cout << "...revresed moved\nstep...\n";
			
		s.step(*reversed_rule, normalize);
		
		if (verbose >= TEST_STEP_DEBUG_LEVEL)
			std::cout << "...steped\nswap...\n";
		
		if (verbose >= TEST_STEP_DEBUG_LEVEL)
			std::cout << "...swapped\n";

		if (verbose >= GRAPH_SIZE_DEBUG_LEVEL)
			std::cout << s.symbolic_num_graphs << " -> " << s.current_iteration.num_graphs << " graphs\n";
	}

	print(s);
}