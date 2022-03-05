#include <iostream>
#include <ranges>

#include "../IQS/src/iqs.hpp"
#include "../IQS/src/rules/qcgd.hpp"

int main(int argc, char* argv[]) {
	iqs::it_t *state = new iqs::it_t(), *buffer = new iqs::it_t();
	iqs::sy_it_t sy_it;
	
	iqs::rules::qcgd::utils::max_print_num_graphs = 10;

	auto [n_iter, reversed_n_iter, rules, max_num_object] = iqs::rules::qcgd::flags::parse_simulation(argv[1], *state);

	iqs::rules::qcgd::utils::print(*state);

	for (int i = 0; i < n_iter; ++i)
		for (auto [n_iter, is_rule, modifier, rule, _, __] : rules)
			for (int j = 0; j < n_iter; ++j)
				if (is_rule) {
					iqs::simulate(*state, rule, *buffer, sy_it, max_num_object);

					std::swap(state, buffer);
				} else
					iqs::simulate(*state, modifier);

	for (int i = 0; i < reversed_n_iter; ++i)
		for (auto [n_iter, is_rule, _, __, modifier, rule] : rules | std::views::reverse)
			for (int j = 0; j < n_iter; ++j)
				if (is_rule) {
					iqs::simulate(*state, rule, *buffer, sy_it, max_num_object);

					std::swap(state, buffer);
				} else
					iqs::simulate(*state, modifier);

	std::cout << "\n"; iqs::rules::qcgd::utils::print(*state);
}