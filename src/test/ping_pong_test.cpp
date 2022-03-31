#include <iostream>
#include <ranges>

#include "../QuIDS/src/quids.hpp"
#include "../QuIDS/src/rules/qcgd.hpp"

int main(int argc, char* argv[]) {
	quids::it_t *state = new quids::it_t(), *buffer = new quids::it_t();
	quids::sy_it_t sy_it;
	
	quids::rules::qcgd::utils::max_print_num_graphs = 10;

	auto [n_iter, reversed_n_iter, rules, max_num_object] = quids::rules::qcgd::flags::parse_simulation(argv[1], *state);

	quids::rules::qcgd::utils::print(*state);

	for (int i = 0; i < n_iter; ++i)
		for (auto [local_n_iter, is_rule, modifier, rule, _, __] : rules)
			for (int j = 0; j < local_n_iter; ++j)
				if (is_rule) {
					quids::simulate(*state, rule, *buffer, sy_it, max_num_object);

					std::swap(state, buffer);
				} else
					quids::simulate(*state, modifier);

	for (int i = 0; i < reversed_n_iter; ++i)
		for (auto [local_n_iter, is_rule, _, __, modifier, rule] : rules | std::views::reverse)
			for (int j = 0; j < local_n_iter; ++j)
				if (is_rule) {
					quids::simulate(*state, rule, *buffer, sy_it, max_num_object);

					std::swap(state, buffer);
				} else
					quids::simulate(*state, modifier);

	std::cout << "\n"; quids::rules::qcgd::utils::print(*state);
}
