#include <vector>
#include <chrono>
#include <ranges>
#include <unistd.h>
#include <time.h>

#include <iostream>

#include "../IQS/src/iqs_mpi.hpp"
#include "../IQS/src/rules/qcgd.hpp"

typedef std::chrono::time_point<std::chrono::high_resolution_clock> time_point;

std::map<std::string, double> max_step_duration;
std::map<std::string, double> min_step_duration;
std::map<std::string, double> avg_cpu_step_duration;

time_point step_start;
clock_t cpu_step_start;

size_t max_num_object = 0;
size_t min_num_object = 0;

size_t max_symbolic_num_object = 0;
size_t min_symbolic_num_object = 0;

size_t max_num_object_after_interference = 0;
size_t min_num_object_after_interference = 0;

size_t max_num_object_after_selection = 0;
size_t min_num_object_after_selection = 0;

std::string last_name = "end";

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

	auto [n_iter, reversed_n_iter, local_state, rules, max_allowed_num_object] = iqs::rules::qcgd::flags::parse_simulation(argv[1]);

	iqs::rule_t *rule = new iqs::rules::qcgd::split_merge(0.25, 0.25);

	if (rank == 0)
		for (int i = 0; i < local_state.num_object; ++i) {
			std::complex<PROBA_TYPE> mag;
			size_t size;
			char const *object_begin;
			local_state.get_object(i, object_begin, size, mag);
			state.append(object_begin, object_begin + size, mag);
		}

	auto const mid_step_function = [&](const char* name) {
		std::string string_name(name);

		if (last_name != "end") {
			time_point stop = std::chrono::high_resolution_clock::now();
			clock_t cpu_stop = clock();

			if (!max_step_duration.count(last_name)) {
				max_step_duration[last_name] = 0.0;
				min_step_duration[last_name] = 0.0;
				avg_cpu_step_duration[last_name] = 0.0;
			}

			double local_step_duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - step_start).count() * 1e-6;
			double local_cpu_step_duration = (double(cpu_stop - cpu_step_start)/CLOCKS_PER_SEC);

			/* collect max time one node 0 */
			double mpi_buffer = local_step_duration;
			//MPI_Allreduce(&local_step_duration, &mpi_buffer, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
			max_step_duration[last_name] += mpi_buffer;

			/* collect min time one node 0 */
			//MPI_Allreduce(&local_step_duration, &mpi_buffer, 1, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD);
			min_step_duration[last_name] += mpi_buffer;

			/* collect min time one node 0 */
			//MPI_Allreduce(&local_cpu_step_duration, &mpi_buffer, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
			avg_cpu_step_duration[last_name] += mpi_buffer;
		}

		last_name = string_name;

		step_start = std::chrono::high_resolution_clock::now();
		cpu_step_start = clock();
	};

	auto start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < n_iter; ++i) {
		for (auto [n_iter, is_rule, modifier, rule, _, __] : rules)
			for (int j = 0; j < n_iter; ++j) {
				if (is_rule) {
					size_t mpi_buffer = 0;
					MPI_Allreduce(&state.num_object, &mpi_buffer, 1, MPI_UNSIGNED_LONG_LONG, MPI_MAX, MPI_COMM_WORLD);
					max_num_object += mpi_buffer;
					MPI_Allreduce(&state.num_object, &mpi_buffer, 1, MPI_UNSIGNED_LONG_LONG, MPI_MIN, MPI_COMM_WORLD);
					min_num_object += mpi_buffer;

					iqs::mpi::simulate(state, rule, buffer, sy_it, MPI_COMM_WORLD, max_allowed_num_object, mid_step_function);

					MPI_Allreduce(&sy_it.num_object, &mpi_buffer, 1, MPI_UNSIGNED_LONG_LONG, MPI_MAX, MPI_COMM_WORLD);
					max_symbolic_num_object += mpi_buffer;
					MPI_Allreduce(&sy_it.num_object, &mpi_buffer, 1, MPI_UNSIGNED_LONG_LONG, MPI_MIN, MPI_COMM_WORLD);
					min_symbolic_num_object += mpi_buffer;

					MPI_Allreduce(&sy_it.num_object_after_interferences, &mpi_buffer, 1, MPI_UNSIGNED_LONG_LONG, MPI_MAX, MPI_COMM_WORLD);
					max_num_object_after_interference += mpi_buffer;
					MPI_Allreduce(&sy_it.num_object_after_interferences, &mpi_buffer, 1, MPI_UNSIGNED_LONG_LONG, MPI_MIN, MPI_COMM_WORLD);
					min_num_object_after_interference += mpi_buffer;

					MPI_Allreduce(&state.num_object, &mpi_buffer, 1, MPI_UNSIGNED_LONG_LONG, MPI_MAX, MPI_COMM_WORLD);
					max_num_object_after_selection += mpi_buffer;
					MPI_Allreduce(&state.num_object, &mpi_buffer, 1, MPI_UNSIGNED_LONG_LONG, MPI_MIN, MPI_COMM_WORLD);
					min_num_object_after_selection += mpi_buffer;

				} else
					iqs::simulate(state, modifier);
			}
	}
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

	/* print results as json */
	size_t total_num_object = state.get_total_num_object(MPI_COMM_WORLD);
	if (rank == 0) {
		printf("\t\"max_step_time\" : {");
		for (auto it = max_step_duration.begin();;) {
			printf("\n\t\t\"%s\" : %f", it->first.c_str(), it->second);
			if (++it != max_step_duration.end()) {
				printf(", ");
			} else
				break;
		}
		
		/*printf("\n\t},\n\n\t\"min_step_time\" : {");
		for (auto it = min_step_duration.begin();;) {
			printf("\n\t\t\"%s\" : %f", it->first.c_str(), it->second);
			if (++it != min_step_duration.end()) {
				printf(", ");
			} else
				break;
		}

		/*int total_num_threads = size;
		#pragma omp parallel
		#pragma omp single
		total_num_threads *= omp_get_num_threads();

		printf("\n\t},\n\n\t\"avg_cpu_step_time\" : {");
		for (auto it = avg_cpu_step_duration.begin();;) {
			printf("\n\t\t\"%s\" : %f", it->first.c_str(), it->second / total_num_threads);
			if (++it != avg_cpu_step_duration.end()) {
				printf(", ");
			} else
				break;
		}*/

		printf("\n\t},\n\n\t\"max_num_object\" : %lu,", max_num_object);
		printf("\n\t\"min_num_object\" : %lu,", min_num_object);

		printf("\n\n\t\"max_symbolic_num_object\" : %lu,", max_symbolic_num_object);
		printf("\n\t\"min_symbolic_num_object\" : %lu,", min_symbolic_num_object);

		printf("\n\n\t\"max_num_object_after_interference\" : %lu,", max_num_object_after_interference);
		printf("\n\t\"min_num_object_after_interference\" : %lu,", min_num_object_after_interference);

		printf("\n\n\t\"max_num_object_after_selection\" : %lu,", max_num_object_after_selection);
		printf("\n\t\"min_num_object_after_selection\" : %lu,", min_num_object_after_selection);

		printf("\n\n\t\"num_object\" : %lu,", total_num_object);
		printf("\n\t\"total\" : %f\n}", duration.count()*1e-6);
	}

	MPI_Finalize();
}
