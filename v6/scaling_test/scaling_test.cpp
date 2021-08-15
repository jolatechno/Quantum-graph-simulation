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
		int n_rule = 0; \
		if (rule.name == "erase_create") { \
			n_rule = 2; \
		} else if (rule.name == "split_merge") { \
			n_rule = 3; \
		} else if (rule.name == "coin") { \
			n_rule = 2; \
		} \
\
		size_t current_num_nodes = current_iteration.node_begin[current_iteration.num_graphs]; \
		size_t next_num_nodes = next_iteration.node_begin[next_iteration.num_graphs]; \
\
		total_reads[0] += 2*current_iteration.num_graphs*sizeof(size_t) + n_rule*current_num_nodes; \
		total_writes[0] += current_num_nodes; \
\
		total_reads[1] += 2*current_iteration.num_graphs*sizeof(size_t) + current_num_nodes; \
		total_writes[1] += current_iteration.num_graphs*sizeof(size_t); \
\
		total_reads[2] += 2*current_iteration.num_graphs*sizeof(size_t); \
		total_writes[2] += symbolic_num_graphs*(sizeof(size_t) + sizeof(unsigned short int)) + 2*current_iteration.num_graphs*sizeof(size_t); \
\
		total_reads[3] += symbolic_num_graphs*(6*sizeof(size_t) + 2*sizeof(PROBA_TYPE) + sizeof(short)) + 2*next_num_nodes; \
		total_writes[3] += symbolic_num_graphs*(3*sizeof(size_t) + 2*sizeof(PROBA_TYPE)); \
\
		total_reads[4] += symbolic_num_graphs*(sizeof(size_t)*32 + sizeof(PROBA_TYPE)*4 + 10); \
		total_writes[4] += symbolic_num_graphs*(sizeof(size_t)*17 + sizeof(PROBA_TYPE)*2 + 1); \
\
		total_reads[5] += symbolic_num_graphs_after_interferences*(2*sizeof(PROBA_TYPE) + 2*sizeof(float) + sizeof(size_t)); \
		total_writes[5] += max_num_graphs*sizeof(size_t) + symbolic_num_graphs_after_interferences*sizeof(float); \
\
		total_reads[6] += next_iteration.num_graphs*((6 + std::log2(next_iteration.num_graphs))*sizeof(size_t) + 2*sizeof(PROBA_TYPE)); \
		total_writes[6] = total_reads[6]; \
\
		total_reads[7] += n_rule*next_iteration.num_graphs + 3*next_iteration.num_graphs; \
		total_writes[7] += 2*next_iteration.num_graphs*2; \
\
		total_reads[8] += 2*next_iteration.num_graphs*sizeof(PROBA_TYPE); \
		total_writes[8] = total_reads[8]; \
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

int main(int argc, char* argv[]) {
	/* test scalability */
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
		printf("%f", total_reads[i] / step_wall_duration[i] / 1e9);
		if (i < N_STEP - 1) {
			printf(", ");
		} else
			printf("]");
	}
	printf(",\n\t\"write_bandwidth\" : [");
	for (unsigned int i = 0; i < N_STEP; ++i) {
		printf("%f", total_writes[i] / step_wall_duration[i] / 1e9);
		if (i < N_STEP - 1) {
			printf(", ");
		} else
			printf("]");
	}
	printf("\n}\n");
}