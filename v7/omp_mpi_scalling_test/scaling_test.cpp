#include <vector>
#include <chrono>
#include <ranges>
#include <unistd.h>

#include <iostream>

#include "../IQS/src/iqs_mpi.hpp"
#include "../IQS/src/rules/qcgd.hpp"

typedef std::chrono::time_point<std::chrono::high_resolution_clock> time_point;

int main(int argc, char* argv[]) {
	int provided, rank, size;
	MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    if(provided < MPI_THREAD_MULTIPLE) {
        printf("The threading support level is lesser than that demanded.\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	iqs::mpi::mpi_it_t state, buffer;
	iqs::mpi::mpi_sy_it_t sy_it;

	auto [n_iter, reversed_n_iter, local_state, rules, max_num_object] = iqs::rules::qcgd::flags::parse_simulation(argv[1]);

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
		if (rank == 0) {
			if (n > 0) {
				time_point stop = std::chrono::high_resolution_clock::now(); \
				auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - step_start[n - 1]); \
				step_duration[n - 1] += duration.count() * 1e-6; \
			}
			if (n < num_step) {
				step_start[n] = std::chrono::high_resolution_clock::now(); \
			}
		}
	};

	auto start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < n_iter; ++i) {
		for (auto [n_iter, is_rule, modifier, rule, _, __] : rules)
			for (int j = 0; j < n_iter; ++j) {
				if (is_rule) {
					iqs::mpi::simulate(state, rule, buffer, sy_it, MPI_COMM_WORLD, max_num_object, mid_step_function);
				} else
					iqs::simulate(state, modifier);
			}
	}
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

	/* print results as json */
	size_t total_num_object = state.get_total_num_object(MPI_COMM_WORLD);
	if (rank == 0) {
		printf("\t\"steps\" : [");
		for (unsigned int i = 0; i < num_step; ++i) {
			printf("%f", step_duration[i]);
			if (i < num_step - 1) {
				printf(", ");
			} else
				printf("],\n\t\"num_object\" : %lu,\n\t\"total\" : %f\n}", total_num_object, duration.count() * 1e-6);
		}
	}

	MPI_Finalize();
}
