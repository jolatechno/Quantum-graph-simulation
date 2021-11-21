#include "../IQS/src/iqs.hpp"
#include "../IQS/src/rules/qcgd.hpp"

int main(int argc, char* argv[]) {
	iqs::it_t buffer;
	iqs::sy_it_t sy_it;

	auto [n_iter, _, state, rules] = iqs::rules::qcgd::flags::parse_simulation(argv[1]);

	iqs::rules::qcgd::utils::print(state);

	std::cout << "{\n\t\"command\" : \"" << argv[1] << "\",\n";
	std::cout << "\t\"iterations\" : [\n\t\t";

	iqs::rules::qcgd::utils::serialize(state, sy_it, 2);
	for (int i = 0; i < n_iter; ++i) {
		for (auto [n_iter, is_rule, modifier, rule, _, __] : rules)
			for (int j = 0; j < n_iter; ++j)
				if (is_rule) {
					iqs::simulate(state, rule, buffer, sy_it);
				} else
					iqs::simulate(state, modifier);
		std::cout << ", ";
		iqs::rules::qcgd::utils::serialize(state, sy_it, 2);
	}

	std::cout << "\n\t]\n}\n";
}
