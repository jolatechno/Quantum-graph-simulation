#include "../src/rules.hpp"
#include "../src/utils/parser.hpp"

#include "../src/utils/debuging.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

int main(int argc, char* argv[]) {
	std::setvbuf(stdout, NULL, _IONBF, 0);

	cxxopts::Options options("running quantum iterations for grapher");
	auto [s, rule_1, n_iter_1, moove_1, rule_2, n_iter_2, moove_2, n_iter, max_num_graphs, rule_name, normalize] = iteration_parser(options, argc, argv);

	state new_state(1000, 1000, 1000);

	start_json(s, rule_name.c_str());

	for (int i = 0; i < n_iter; ++i) {
		for (int j = 0; j < n_iter_1; ++j) {
			s.step(new_state, *rule_1, max_num_graphs, normalize);
			std::swap(s, new_state);

			if (moove_1)
				move_all(s);
		}

		for (int j = 0; j < n_iter_2; ++j) {
			s.step(new_state, *rule_2, max_num_graphs, normalize);
			std::swap(s, new_state);

			if (moove_1)
				move_all(s);
		}

		serialize_state_to_json(s);
	}

	end_json();
}