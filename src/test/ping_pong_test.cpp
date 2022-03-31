#include <iostream>
#include <ranges>

#include "../IQDS/src/iqds.hpp"
#include "../IQDS/src/rules/qcgd.hpp"

int main(int argc, char* argv[]) {
	iqds::it_t *state = new iqds::it_t(), *buffer = new iqds::it_t();
	iqds::sy_it_t sy_it;
	
	iqds::rules::qcgd::utils::max_print_num_graphs = 10;

	auto [n_iter, reversed_n_iter, rules, max_num_object] = iqds::rules::qcgd::flags::parse_simulation(argv[1], *state);

	iqds::rules::qcgd::utils::print(*state);

	for (int i = 0; i < n_iter; ++i)
		for (auto [local_n_iter, is_rule, modifier, rule, _, __] : rules)
			for (int j = 0; j < local_n_iter; ++j)
				if (is_rule) {
					iqds::simulate(*state, rule, *buffer, sy_it, max_num_object);

					std::swap(state, buffer);
				} else
					iqds::simulate(*state, modifier);

	for (int i = 0; i < reversed_n_iter; ++i)
		for (auto [local_n_iter, is_rule, _, __, modifier, rule] : rules | std::views::reverse)
			for (int j = 0; j < local_n_iter; ++j)
				if (is_rule) {
					iqds::simulate(*state, rule, *buffer, sy_it, max_num_object);

					std::swap(state, buffer);
				} else
					iqds::simulate(*state, modifier);

	std::cout << "\n"; iqds::rules::qcgd::utils::print(*state);
}
