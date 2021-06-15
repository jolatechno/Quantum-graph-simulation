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

	tolerance = 1e-18;
	std::setvbuf(stdout, NULL, _IONBF, 0);

	state s(5);
	s.randomize();

	print(s); std::cout << "\n";

	auto rule = /*split_merge_rule*/erase_create_rule/**/(M_PI_4, 0);

	
	unsigned int n_iteration = 1;
	state new_state(1);
	
	for (int i = 0; i < n_iteration; ++i) {
		move_all(s);
		s.step(new_state, rule, -1);
		std::swap(s, new_state);
	}

	for (int i = 0; i < n_iteration; ++i) {
		s.step(new_state, rule, -1);
		std::swap(s, new_state);
		reversed_move_all(s);
	}

	print(s);
}