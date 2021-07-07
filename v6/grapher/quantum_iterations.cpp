#include "../src/rules.hpp"
#include "../src/utils/parser.hpp"

#include "../src/utils/debuging.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

int main(int argc, char* argv[]) {
	std::setvbuf(stdout, NULL, _IONBF, 0);

	cxxopts::Options options("running quantum iterations for grapher");
	auto [s, rule_1, rule_2, n_iter, max_num_graphs, normalize, only_last_iter] = iteration_parser(options, argc, argv);

	state new_state(1000, 1000, 1000);

	start_json(*rule_1, *rule_2, n_iter, only_last_iter);

	for (int i = 0; i < n_iter; ++i) {
		if (!only_last_iter)
			serialize_state_to_json(s, only_last_iter);

		for (int j = 0; j < rule_1->n_iter; ++j) {
			s.step(new_state, *rule_1, max_num_graphs, normalize);
			std::swap(s, new_state);

			if (rule_1->move)
				move_all(s);
		}

		for (int j = 0; j < rule_2->n_iter; ++j) {
			s.step(new_state, *rule_2, max_num_graphs, normalize);
			std::swap(s, new_state);

			if (rule_2->move)
				move_all(s);
		}
	}

	end_json(s, only_last_iter);
}