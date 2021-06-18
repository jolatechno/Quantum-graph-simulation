#include "../src/rules.hpp"
#include "../src/utils/parser.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

int main(int argc, char* argv[]) {
	std::setvbuf(stdout, NULL, _IONBF, 0);

	cxxopts::Options options("check for quantum injectivity");
	auto [size, rule, _, n_iter, max_num_graphs, __, ___] = parser(options, argc, argv);

	state s(size);
	s.randomize();

	move_all(s);
	print(s); std::cout << "\n";

	//auto rule = erase_create_rule(M_PI_4, 0);

	state new_state(1); //10000, 10000, 10000, 10000);

	for (int i = 0; i < n_iter; ++i) {
		//state new_state(1);

		std::cout << "move...\n";
		move_all(s);
		std::cout << "...moved\nstep...\n";
		s.step(new_state, *(rule_t*)rule, max_num_graphs);
		std::cout << "...steped\nswap...\n";
		std::swap(s, new_state);
		std::cout << "...swaped\n";

		std::cout << s.num_graphs << " graphs\n";
	}

	for (int i = 0; i < n_iter; ++i) {
		//state new_state(1);

		std::cout << "step...\n";
		s.step(new_state, *(rule_t*)rule, max_num_graphs);
		std::cout << "...steped\nswap...\n";
		std::swap(s, new_state);
		std::cout << "...swapped\nmoove...\n";
		reversed_move_all(s);
		std::cout << "...mooved\n";

		std::cout << s.num_graphs << " graphs\n";
	}

	//std::cout << "print...\n";
	print(s);
}