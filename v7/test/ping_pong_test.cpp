#include "../IQS/src/iqs.hpp"
#include "../IQS/src/rules/qcgd.hpp"

int main(int argc, char* argv[]) {
	iqs::it_t buffer;
	iqs::sy_it_t sy_it;

	auto [n_iter, state, rule, reversed_rule] = iqs::rules::qcgd::flags::parse_simulation(argv[1]);

	iqs::rules::qcgd::utils::print(state);

	for (int i = 0; i < n_iter; ++i)
		rule(state, buffer, sy_it);
	for (int i = 0; i < n_iter; ++i)
		reversed_rule(state, buffer, sy_it);

	iqs::set_tolerance(1e-10);
	std::cout << "\n"; iqs::rules::qcgd::utils::print(state);
}
