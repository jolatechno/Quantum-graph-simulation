#include "../src/rules.hpp"
#include "../src/utils/parser.hpp"

#include "../src/utils/debuging.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

int main(int argc, char* argv[]) {
	std::setvbuf(stdout, NULL, _IONBF, 0);

	cxxopts::Options options("running quantum iterations for grapher");
	auto [size, rule, rule2, n_iter, n_reversed_iteration, max_num_graphs, rule_name, normalize] = parser(options, argc, argv);

	state s(size);
	s.randomize();

	state new_state(1000, 1000, 1000);

	start_json(s, rule_name.c_str());

	for (int i = 0; i < n_iter; ++i) {
		if (i & 1) {
			s.step(new_state, *(rule_t*)rule, max_num_graphs, normalize);
		} else
			s.step(new_state, *(rule_t*)rule2, max_num_graphs, normalize);
		
		std::swap(s, new_state);
		move_all(s);

		serialize_state_to_json(s);
	}

	end_json();
}