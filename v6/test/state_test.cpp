#include "../src/rules.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

int main() {
	tolerance = 1e-18;
	std::setvbuf(stdout, NULL, _IONBF, 0);

	state s(5);
	s.randomize();

	print(s); std::cout << "\n";

	move_all(s);
	print(s); std::cout << "\n";

	reversed_move_all(s);
	print(s);

	auto rule = /*split_merge_rule*/erase_create_rule(M_PI_4, 0);

	state new_state;
	s.step(new_state, rule, -1);
	print(new_state);

	std::swap(s, new_state);

	s.step(new_state, rule, -1);
	print(new_state);
}