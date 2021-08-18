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
	auto [s, rule, reversed_rule, n_iter, n_reversed_iteration, n_fast] = test_parser(options, argc, argv);

	print(s);

	PROBA_TYPE total_proba = 1;

	for (int i = 0; i < n_iter; ++i) {
		if (verbose >= TEST_STEP_DEBUG_LEVEL)
			std::cerr << "step...\n";
		
		if (n_fast == 0 || i % (n_fast + 1)) {
			s.step(*rule);
		} else
			s.fast_step(*rule);
		
		if (verbose >= TEST_STEP_DEBUG_LEVEL)
			std::cerr << "...steped\nswap...\n";
		
		if (verbose >= TEST_STEP_DEBUG_LEVEL)
			std::cerr << "...swaped\nmove...\n";
		
		move_all(s);

		if (verbose >= TEST_STEP_DEBUG_LEVEL)
			std::cerr << "...moved\n";

		if (verbose >= GRAPH_SIZE_DEBUG_LEVEL)
			std::cerr << s.symbolic_num_graphs << " -> " << s.current_iteration.num_graphs << " graphs\n";

		total_proba *= s.total_proba;
	}

	for (int i = n_reversed_iteration - 1; i >= 0; --i) {
		if (verbose >= TEST_STEP_DEBUG_LEVEL)
			std::cerr << "reversed move...\n";
		
		reversed_move_all(s);

		if (verbose >= TEST_STEP_DEBUG_LEVEL)
			std::cerr << "...revresed moved\nstep...\n";
		
		if (n_fast == 0 || i % (n_fast + 1)) {
			s.step(*reversed_rule);
		} else
			s.fast_step(*reversed_rule);
		
		if (verbose >= TEST_STEP_DEBUG_LEVEL)
			std::cerr << "...steped\nswap...\n";
		
		if (verbose >= TEST_STEP_DEBUG_LEVEL)
			std::cerr << "...swapped\n";

		if (verbose >= GRAPH_SIZE_DEBUG_LEVEL)
			std::cerr << s.symbolic_num_graphs << " -> " << s.current_iteration.num_graphs << " graphs\n";

		total_proba *= s.total_proba;
	}

	std::cout << total_proba << "\n";

	print(s);
}