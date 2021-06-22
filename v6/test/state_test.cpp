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
	auto [size, rule, _, n_iter, n_reversed_iteration, max_num_graphs, __, ___] = parser(options, argc, argv);

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

	for (int i = 0; i < n_reversed_iteration; ++i) {
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

	if (verbose >= .05) {
		printf("\n");

		for (unsigned int i = 0; i < s.num_graphs; ++i) {
			printf("gid:%d, n:", i);
			for (unsigned int j = s.node_begin[i]; j < s.node_begin[i + 1]; ++j)
				printf("%d,", s.node_id_c[j]);

			printf("  ");

			for (unsigned int j = s.sub_node_begin[i]; j < s.sub_node_begin[i + 1]; ++j) {
				std::string type;

				auto node_type = s.node_type(i, j - s.sub_node_begin[i]);
				switch (node_type) {
					case state_t::left_t:
						type = "l";
						break;

					case state_t::right_t:
						type = "r";
						break;

					case state_t::element_t:
						type = "e";
						break;

					case state_t::pair_t:
						type = "p";
						break;

					case state_t::trash_t:
						type = "x";
						break;

					default:
						type = "!";
						break;
				}

				printf("(l:%d, r:%d, t:%s, h:%ld), ", s.left_idx__or_element__and_has_most_left_zero_[j],
					s.right_idx__or_type__and_is_trash_[j],
					type.c_str(),
					s.node_hash[j]);
			}

			std::cout << "\n";
		}

		std::cout << "\n";
	}
	/* !!!!!!!!!!!!!!!
	!!!!!!!!!!!!!!!
	debuging
	!!!!!!!!!!!!!!!
	!!!!!!!!!!!!!!! */

	print(s);
}