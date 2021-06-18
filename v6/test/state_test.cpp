#include "../src/rules.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

int main() {
	/*std::vector<int> test(1000000000, 2);
	__gnu_parallel::partial_sum(test.begin(), test.end(), test.begin());

	std::cout << "finished\n";

	#pragma omp parallel master
	__gnu_parallel::partial_sum(test.begin(), test.end(), test.begin());

	std::cout << "finished\n";*/

	tolerance = 1e-18; //0;
	verbose = 0;

	std::setvbuf(stdout, NULL, _IONBF, 0);

	state s(5 /*10*/);
	s.randomize();

	move_all(s);
	print(s); std::cout << "\n";

	//auto rule = erase_create_rule(M_PI_4, 0);
	auto rule = split_merge_rule(M_PI_4, 0);
	
	unsigned int n_iteration = 1; //20;

	state new_state(1); //10000, 10000, 10000, 10000);

	for (int i = 0; i < n_iteration; ++i) {
		//state new_state(1);

		std::cout << "move...\n";
		move_all(s);
		std::cout << "...moved\nstep...\n";
		s.step(new_state, rule);
		std::cout << "...steped\nswap...\n";
		std::swap(s, new_state);
		std::cout << "...swaped\n";

		s.step(new_state, rule);
		new_state.step(s, rule);

		std::cout << s.num_graphs << " graphs\n";
	}

	for (int i = 0; i < n_iteration; ++i) {
		//state new_state(1);

		std::cout << "step...\n";
		s.step(new_state, rule);
		std::cout << "...steped\nswap...\n";
		std::swap(s, new_state);
		std::cout << "...swapped\nmoove...\n";
		reversed_move_all(s);
		std::cout << "...mooved\n";

		s.step(new_state, rule);
		new_state.step(s, rule);

		std::cout << s.num_graphs << " graphs\n";
	}

	//std::cout << "print...\n";
	print(s);
}