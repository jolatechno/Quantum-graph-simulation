#include <time.h>
#include <vector>
#include <sys/time.h>

/* number of step */
#define N_STEP 9

/* define a custom "MID_STEP_FUNCTION(n)" that accumulate the time taken in each step */
#define MID_STEP_FUNCTION(n) \
	if (n > 0) { \
		step_wall_duration[n - 1] += get_wall_time(); \
		step_cpu_duration[n - 1] += get_cpu_time(); \
	} \
	if (n == N_STEP) { \
		total_reads[0] += (current_iteration.node_begin[current_iteration.num_graphs] \
			+ 2*current_iteration.num_graphs) * sizeof(long long int); \
		total_writes[0] += 2*current_iteration.node_begin[current_iteration.num_graphs] * \
			sizeof(long long int); \
\
		total_reads[1] += current_iteration.node_begin[current_iteration.num_graphs] \
			+ 2*current_iteration.num_graphs * sizeof(long long int); \
		total_writes[1] += current_iteration.num_graphs * sizeof(long long int); \
\
		total_reads[2] += current_iteration.num_graphs * sizeof(long long int); \
		total_writes[2] += (2*current_iteration.num_graphs + 2*symbolic_num_graphs) * \
			sizeof(long long int); \
\
		total_reads[3] += symbolic_num_graphs * \
			current_iteration.node_begin[current_iteration.num_graphs] / \
			current_iteration.num_graphs * 2 * sizeof(long long int); \
		total_writes[3] += symbolic_num_graphs * \
			(3*sizeof(long long int) + 2*sizeof(PROBA_TYPE)); \
\
		total_reads[4] += symbolic_num_graphs * ( \
			11 * sizeof(size_t) + 2 + 4 * sizeof(PROBA_TYPE)); \
		total_writes[4] += symbolic_num_graphs * ( \
			16 * sizeof(size_t) + 1 + 2 * sizeof(PROBA_TYPE)); \
\
		total_reads[5] += symbolic_num_graphs * 2 * sizeof(PROBA_TYPE); \
		total_writes[5] += symbolic_num_graphs * sizeof(float); \
\
		total_reads[6] += std::log(next_iteration.num_graphs) * next_iteration.num_graphs * sizeof(long long int); \
		total_writes[6] += std::log(next_iteration.num_graphs) * next_iteration.num_graphs * sizeof(long long int); \
\
		total_reads[7] += next_iteration.num_graphs * ( \
			4 * sizeof(long long int) + 2 * sizeof(PROBA_TYPE)); \
		total_writes[7] += next_iteration.num_graphs * ( \
			4 * sizeof(long long int) + 2 * sizeof(PROBA_TYPE)); \
\
		total_reads[8] += next_iteration.node_begin[next_iteration.num_graphs] * 3; \
		total_writes[8] += next_iteration.node_begin[next_iteration.num_graphs] * 2; \
\
		total_reads[9] += next_iteration.num_graphs * 2 * sizeof(PROBA_TYPE); \
		total_writes[9] += next_iteration.num_graphs * 2 * sizeof(PROBA_TYPE); \
	} else { \
		step_wall_duration[n] -= get_wall_time(); \
		step_cpu_duration[n] -= get_cpu_time(); \
	}	

/* global vector to accumulate */
#define STATE_GLOBAL_HEADER \
	double get_wall_time() { \
	    struct timeval time; \
	    gettimeofday(&time,NULL); \
	    return (double)time.tv_sec + (double)time.tv_usec * .000001; \
	} \
	double get_cpu_time() { \
	    return (double)clock() / CLOCKS_PER_SEC; \
	} \
	std::vector<double> step_cpu_duration(N_STEP, 0.0); \
	std::vector<double> step_wall_duration(N_STEP, 0.0); \
	std::vector<double> total_reads(N_STEP, 0.0); \
	std::vector<double> total_writes(N_STEP, 0.0);

#include "../src/rules.hpp"
#include "../src/utils/parser.hpp"

// debug levels
#define TEST_STEP_DEBUG_LEVEL 0.2
#define GRAPH_SIZE_DEBUG_LEVEL 0.1

#ifndef NUM_ELMENTS
	#define NUM_ELMENTS 200000
#endif

int main(int argc, char* argv[]) {
	std::setvbuf(stdout, NULL, _IONBF, 0);

	cxxopts::Options options("file used for multi-threading test (does not run reversed iterations");
	auto [s, rule, _, n_iter, __, n_fast] = test_parser(options, argc, argv);

	double total_cpu_time = -get_cpu_time();
	double total_wall_time = -get_wall_time();

	for (int i = 0; i < n_iter; ++i) {
		if (n_fast == 0 || i % (n_fast + 1) == 0) {
			s.step(*rule);
		} else
			s.fast_step(*rule);
		
		move_all(s);
	}

	total_cpu_time += get_cpu_time();
	total_wall_time += get_wall_time();

	/* meusure max memory bandwidth */
	unsigned int num_elements = NUM_ELMENTS;

	size_t in_array[num_elements] = { 1 };
	size_t out_array[num_elements] = { 0 };

	double memory_delay = -get_wall_time();
	for (unsigned int i = 0; i < num_elements; ++i) out_array[i] = in_array[i];
	memory_delay += get_wall_time();

	double memory_speed = num_elements / memory_delay * sizeof(size_t);

	/* print results as json */
	printf("\t\"wall\" : {\n\t\t\"steps\" : [");
	for (unsigned int i = 0; i < N_STEP; ++i) {
		printf("%f", step_wall_duration[i]);
		if (i < N_STEP - 1) {
			printf(", ");
		} else
			printf("],\n\t\t\"total\" : %f\n\t}", total_wall_time);
	}
	printf(",\n\t\"cpu\" : {\n\t\t\"steps\" : [");
	for (unsigned int i = 0; i < N_STEP; ++i) {
		printf("%f", step_cpu_duration[i]);
		if (i < N_STEP - 1) {
			printf(", ");
		} else
			printf("],\n\t\t\"total\" : %f\n\t}", total_cpu_time);
	}
	printf(",\n\t\"read_bandwidth\" : [");
	for (unsigned int i = 0; i < N_STEP; ++i) {
		printf("%f", total_reads[i] / step_wall_duration[i] / memory_speed);
		if (i < N_STEP - 1) {
			printf(", ");
		} else
			printf("]");
	}
	printf(",\n\t\"write_bandwidth\" : [");
	for (unsigned int i = 0; i < N_STEP; ++i) {
		printf("%f", total_writes[i] / step_wall_duration[i] / memory_speed);
		if (i < N_STEP - 1) {
			printf(", ");
		} else
			printf("]");
	}
	printf("\n}\n");
}