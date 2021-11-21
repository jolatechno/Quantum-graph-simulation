#include <vector>
#include <chrono>
#include <ranges>

#include <iostream>

#include "../IQS/src/iqs_mpi.hpp"
#include "../IQS/src/rules/qcgd.hpp"

typedef std::chrono::time_point<std::chrono::high_resolution_clock> time_point;

int main(int argc, char* argv[]) {
	int rank;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	iqs::mpi::mpi_it_t state, buffer;
	iqs::mpi::mpi_sy_it_t sy_it;

	auto [n_iter, reversed_n_iter, local_state, rules] = iqs::rules::qcgd::flags::parse_simulation(argv[1]);

	iqs::rule_t *rule = new iqs::rules::qcgd::split_merge(0.25, 0.25);

	if (rank == 0)
		for (int i = 0; i < local_state.num_object; ++i) {
			std::complex<PROBA_TYPE> mag;
			size_t size;
			char const *object_begin;
			local_state.get_object(i, object_begin, size, mag);
			state.append(object_begin, object_begin + size, mag);
		}

	const int num_step = 9;
	std::vector<double> step_duration(num_step, 0.0);
	std::vector<time_point> step_start(num_step);

	auto const mid_step_function = [&](int n) {
		//std::cout << "step (" << n << ")\n";
		//MPI_Barrier(MPI_COMM_WORLD);
		if (n > 0) {
			time_point stop = std::chrono::high_resolution_clock::now(); \
			auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - step_start[n - 1]); \
			step_duration[n - 1] += duration.count() * 1e-6; \
		}
		if (n < num_step) {
			step_start[n] = std::chrono::high_resolution_clock::now(); \
		}
	};

	auto start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < n_iter; ++i) {
		for (auto [n_iter, is_rule, modifier, rule, _, __] : rules)
			for (int j = 0; j < n_iter; ++j)
				if (is_rule) {
					iqs::mpi::simulate(state, rule, buffer, sy_it, MPI_COMM_WORLD, 0, mid_step_function);
				} else
					iqs::simulate(state, modifier);
	}
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

	/* print results as json */
	if (rank == 0) {
		printf("\t\"steps\" : [");
		for (unsigned int i = 0; i < num_step; ++i) {
			printf("%f", step_duration[i]);
			if (i < num_step - 1) {
				printf(", ");
			} else
				printf("],\n\t\"total\" : %f\n}", duration.count() * 1e-6);
		}
	}

	MPI_Finalize();
}
