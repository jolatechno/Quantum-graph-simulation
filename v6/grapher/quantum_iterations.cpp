#include "../src/rules.hpp"
#include "../src/utils/parser.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

int main(int argc, char* argv[]) {
	std::setvbuf(stdout, NULL, _IONBF, 0);

	cxxopts::Options options("running quantum iterations for grapher");
	auto [s, rule_1, rule_2, n_iter, first_serialize_iteration, normalize, n_fast] = iteration_parser(options, argc, argv);
	

	start_json(*rule_1, *rule_2, n_iter);

	int total_iteration = 0;
	for (int i = 0; i < n_iter; ++i, ++total_iteration) {
		if (i >= first_serialize_iteration)
			serialize_state_to_json(s);

		for (int j = 0; j < rule_1->n_iter; ++j, ++total_iteration) {
			if (n_fast == 0 || total_iteration % (n_fast + 1) == 0) {
				s.step(*rule_1, normalize);
			} else
				s.fast_step(*rule_1, normalize);
			

			if (rule_1->move)
				move_all(s);
		}

		for (int j = 0; j < rule_2->n_iter; ++j, ++total_iteration) {
			if (n_fast == 0 || total_iteration % (n_fast + 1) == 0) {
				s.step(*rule_2, normalize);
			} else
				s.fast_step(*rule_2, normalize);

			if (rule_2->move)
				move_all(s);
		}
	}

	end_json(s);
}