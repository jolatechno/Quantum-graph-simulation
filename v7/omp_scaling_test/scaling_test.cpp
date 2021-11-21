#include <vector>
#include <chrono>

#include <iostream>

#include "../IQS/src/iqs.hpp"
#include "../IQS/src/rules/qcgd.hpp"

typedef std::chrono::time_point<std::chrono::high_resolution_clock> time_point;

int main(int argc, char* argv[]) {
	iqs::it_t buffer;
	iqs::sy_it_t sy_it;

	const int num_step = 8;
	std::vector<double> step_duration(num_step, 0.0);
	std::vector<time_point> step_start(num_step);

	auto const mid_step_function = [&](int n) {
		//std::cout << "step (" << n << ")\n";
		if (n > 0) {
			time_point stop = std::chrono::high_resolution_clock::now(); \
			auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - step_start[n - 1]); \
			step_duration[n - 1] += duration.count() * 1e-6; \
		}
		if (n < num_step) {
			step_start[n] = std::chrono::high_resolution_clock::now(); \
		}
	};

	auto [n_iter, reversed_n_iter, state, rules] = iqs::rules::qcgd::flags::parse_simulation(argv[1]);

	auto start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < n_iter; ++i)
		for (auto [n_iter, is_rule, modifier, rule, _, __] : rules)
			for (int j = 0; j < n_iter; ++j)
				if (is_rule) {
					iqs::simulate(state, rule, buffer, sy_it, 0, mid_step_function);
				} else
					iqs::simulate(state, modifier);
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

	/* print results as json */
	printf("\t\"steps\" : [");
	for (unsigned int i = 0; i < num_step; ++i) {
		printf("%f", step_duration[i]);
		if (i < num_step - 1) {
			printf(", ");
		} else
			printf("],\n\t\"total\" : %f\n}", duration.count() * 1e-6);
	}
}
