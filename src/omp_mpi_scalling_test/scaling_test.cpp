#include <chrono>
#include <ranges>
#include <unistd.h>
#include <time.h>
#include <map>

#include <iostream>

#include "../QuIDS/src/quids_mpi.hpp"
#include "../QuIDS/src/rules/qcgd.hpp"


#ifdef __CYGWIN__ // windows systems

#include <windows.h>

size_t getTotalSystemMemory() {
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return status.ullTotalPhys;
}

#elif defined(__linux__) // linux systems

#include <unistd.h>

size_t getTotalSystemMemory() {
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    return pages * page_size;
}

#elif defined(__unix__) // other unix systems
	#error "UNIX system other than LINUX aren't supported for now"
#elif defined(__MACH__) // mac os systems
	#error "macos isn't supported for now !"
#else // other systems
	#error "system isn't supported"
#endif


std::vector<float> min_memory_usage;
std::vector<float> max_memory_usage;
std::vector<float> avg_memory_usage;
std::vector<float> step_accuracy;

typedef std::chrono::time_point<std::chrono::high_resolution_clock> time_point;

std::map<std::string, double> max_step_duration;
std::map<std::string, double> avg_step_duration;
//std::map<std::string, double> avg_cpu_step_duration;

time_point step_start;
clock_t cpu_step_start;

size_t max_num_object = 0;
size_t avg_num_object = 0;

size_t max_symbolic_num_object = 0;
size_t avg_symbolic_num_object = 0;

size_t max_num_object_after_interference = 0;
size_t avg_num_object_after_interference = 0;

size_t max_num_object_after_selection = 0;
size_t avg_num_object_after_selection = 0;

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


	quids::mpi::mpi_it_t buffer1, buffer2;
	quids::mpi::mpi_sy_it_t sy_it;
	quids::it_t local_state;

	quids::mpi::mpi_it_t *state = new quids::mpi::mpi_it_t, *buffer = new quids::mpi::mpi_it_t;

	auto [n_iter, reversed_n_iter, rules, max_allowed_num_object] = quids::rules::qcgd::flags::parse_simulation(argv[1], local_state);

	if (rank == 0)
		for (int i = 0; i < local_state.num_object; ++i) {
			std::complex<PROBA_TYPE> mag;
			size_t size;
			char const *object_begin;
			local_state.get_object(i, object_begin, size, mag);
			state->append(object_begin, object_begin + size, mag);
		}

	auto const debug_mid_step_function = [&](const char* name) {
		std::string string_name(name);

		if (rank == 0)
			if (name != "end") {
				std::cerr << name << "\n";
			} else
				std::cerr << "\n\n";
	};

	float used_memory = 0;
	auto const mid_step_function = [&](const char* name) {
		std::string string_name(name);

		if (last_name != "end") {
			time_point stop = std::chrono::high_resolution_clock::now();
			//clock_t cpu_stop = clock();

			if (!max_step_duration.count(last_name)) {
				max_step_duration[last_name] = 0.0;
				avg_step_duration[last_name] = 0.0;
				//avg_cpu_step_duration[last_name] = 0.0;
			}

			double local_step_duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - step_start).count() * 1e-6;
			//double local_cpu_step_duration = (double(cpu_stop - cpu_step_start)/CLOCKS_PER_SEC);

			float this_step_used_memory = 1. - (float)quids::utils::get_free_mem() / (float)getTotalSystemMemory();
			if (this_step_used_memory > used_memory)
				used_memory = this_step_used_memory;

			/* collect max time one node 0 */
			double mpi_buffer;
			MPI_Allreduce(&local_step_duration, &mpi_buffer, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
			max_step_duration[last_name] += mpi_buffer;

			/* collect min time one node 0 */
			MPI_Allreduce(&local_step_duration, &mpi_buffer, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
			avg_step_duration[last_name] += mpi_buffer / size;

			/* collect min time one node 0 */
			//MPI_Allreduce(&local_cpu_step_duration, &mpi_buffer, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
			//avg_cpu_step_duration[last_name] += mpi_buffer;
		}

		MPI_Barrier(MPI_COMM_WORLD);

		debug_mid_step_function(name);

		last_name = string_name;

		step_start = std::chrono::high_resolution_clock::now();
		//cpu_step_start = clock();
	};

	double total_proba = 1;
	size_t total_num_object = 0;

	time_point start;
	for (int i = 0; i < n_iter; ++i) {
		if (i == reversed_n_iter) {
			total_num_object = state->get_total_num_object(MPI_COMM_WORLD);
			start = std::chrono::high_resolution_clock::now();
		}
		for (auto [local_n_iter, is_rule, modifier, rule, _, __] : rules)
			for (int j = 0; j < local_n_iter; ++j) {
				if (is_rule) {
					used_memory = 0;

					if (i >= reversed_n_iter) {
						size_t mpi_buffer = 0;
						MPI_Allreduce(&state->num_object, &mpi_buffer, 1, MPI_UNSIGNED_LONG_LONG, MPI_MAX, MPI_COMM_WORLD);
						max_num_object += mpi_buffer;
						MPI_Allreduce(&state->num_object, &mpi_buffer, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, MPI_COMM_WORLD);
						avg_num_object += mpi_buffer/size;

						quids::mpi::simulate(*state, rule, *buffer, sy_it, MPI_COMM_WORLD, max_allowed_num_object, mid_step_function);
					} else
						quids::mpi::simulate(*state, rule, *buffer, sy_it, MPI_COMM_WORLD, max_allowed_num_object, debug_mid_step_function);

					std::swap(state, buffer);

					if (i >= reversed_n_iter) {
						float min_mem, max_mem, avg_mem;

						MPI_Allreduce(&used_memory, &min_mem, 1, MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD);
						MPI_Allreduce(&used_memory, &max_mem, 1, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD);
						MPI_Allreduce(&used_memory, &avg_mem, 1, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);
						avg_mem /= size;

						min_memory_usage.push_back(min_mem);
						max_memory_usage.push_back(max_mem);
						avg_memory_usage.push_back(avg_mem);
						step_accuracy.push_back(state->total_proba);

						size_t mpi_buffer = 0;
						MPI_Allreduce(&sy_it.num_object, &mpi_buffer, 1, MPI_UNSIGNED_LONG_LONG, MPI_MAX, MPI_COMM_WORLD);
						max_symbolic_num_object += mpi_buffer;
						MPI_Allreduce(&sy_it.num_object, &mpi_buffer, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, MPI_COMM_WORLD);
						avg_symbolic_num_object += mpi_buffer/size;

						MPI_Allreduce(&sy_it.num_object_after_interferences, &mpi_buffer, 1, MPI_UNSIGNED_LONG_LONG, MPI_MAX, MPI_COMM_WORLD);
						max_num_object_after_interference += mpi_buffer;
						MPI_Allreduce(&sy_it.num_object_after_interferences, &mpi_buffer, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, MPI_COMM_WORLD);
						avg_num_object_after_interference += mpi_buffer/size;

						MPI_Allreduce(&state->num_object, &mpi_buffer, 1, MPI_UNSIGNED_LONG_LONG, MPI_MAX, MPI_COMM_WORLD);
						max_num_object_after_selection += mpi_buffer;
						MPI_Allreduce(&state->num_object, &mpi_buffer, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, MPI_COMM_WORLD);
						avg_num_object_after_selection += mpi_buffer/size;

						total_num_object += state->get_total_num_object(MPI_COMM_WORLD);
						total_proba *= std::pow(state->total_proba, 1/(double)(n_iter - reversed_n_iter));
					}

				} else
					quids::simulate(*state, modifier);
			}
	}
	time_point stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

	/* print results as json */
	if (rank == 0) {
		printf("\t\"avg_step_time\" : {");
		for (auto it = avg_step_duration.begin();;) {
			printf("\n\t\t\"%s\" : %f", it->first.c_str(), it->second);
			if (++it != avg_step_duration.end()) {
				printf(", ");
			} else
				break;
		}

		printf("\n\t},\n\n\t\"relative_inbalance\" : {");
		for (auto it = max_step_duration.begin();;) {
			double relative_inbalance = (it->second - avg_step_duration[it->first])/it->second*100;
			printf("\n\t\t\"%s\" : %f", it->first.c_str(), relative_inbalance);
			if (++it != max_step_duration.end()) {
				printf(", ");
			} else
				break;
		}

		double total_max = 0, total_avg = 0;
		for (auto it = max_step_duration.begin(); it != max_step_duration.end(); ++it) {
			total_max += it->second;
			total_avg += avg_step_duration[it->first];
		}
		double total_imbalance = (total_max - total_avg)/total_max*100;
		printf("\n\t},\n\t\"total_relative_inbalance\" : %f,", total_imbalance);



		printf("\n\n\t\"min_memory_usage\" : [");
		for (int i = 0; i < min_memory_usage.size(); ++i) {
			if (i > 0)
				printf(", ");
			printf("%f", min_memory_usage[i]);
		}

		printf("],\n\t\"max_memory_usage\" : [");
		for (int i = 0; i < max_memory_usage.size(); ++i) {
			if (i > 0)
				printf(", ");
			printf("%f", max_memory_usage[i]);
		}

		printf("],\n\t\"avg_memory_usage\" : [");
		for (int i = 0; i < avg_memory_usage.size(); ++i) {
			if (i > 0)
				printf(", ");
			printf("%f", avg_memory_usage[i]);
		}

		printf("],\n\n\t\"step_accuracy\" : [");
		for (int i = 0; i < step_accuracy.size(); ++i) {
			if (i > 0)
				printf(", ");
			printf("%f", step_accuracy[i]);
		}



		printf("],\n\n\t\"max_num_object\" : %lu,", max_num_object);
		printf("\n\t\"avg_num_object\" : %lu,", avg_num_object);

		printf("\n\n\t\"max_symbolic_num_object\" : %lu,", max_symbolic_num_object);
		printf("\n\t\"avg_symbolic_num_object\" : %lu,", avg_symbolic_num_object);

		printf("\n\n\t\"max_num_object_after_interference\" : %lu,", max_num_object_after_interference);
		printf("\n\t\"avg_num_object_after_interference\" : %lu,", avg_num_object_after_interference);

		printf("\n\n\t\"max_num_object_after_selection\" : %lu,", max_num_object_after_selection);
		printf("\n\t\"avg_num_object_after_selection\" : %lu,", avg_num_object_after_selection);

		printf("\n\n\t\"num_object\" : %lu,", total_num_object);
		std::cout << "\n\t\"total_proba\" : " << total_proba << ",";
		printf("\n\t\"total\" : %f\n}", duration.count()*1e-6);
	}

	MPI_Finalize();
}
