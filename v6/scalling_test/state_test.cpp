#include <time.h>
#include <sys/time.h>

/* number of step */
#define N_STEP 8

/* define a custom "MID_STEP_FUNCTION(n)" that accumulate the time taken in each step */
#define MID_STEP_FUNCTION(n) \
	if (n > 0) { \
		step_wall_duration[n - 1] += get_wall_time(); \
		step_cpu_duration[n - 1] += get_cpu_time(); \
	} \
	if (n < N_STEP) { \
		step_wall_duration[n] -= get_wall_time(); \
		step_cpu_duration[n] -= get_cpu_time(); \
	}	

/* global vector to accumulate */
#define STATE_GLOBAL_HEADER \
	double get_wall_time(){ \
	    struct timeval time; \
	    gettimeofday(&time,NULL); \
	    return (double)time.tv_sec + (double)time.tv_usec * .000001; \
	} \
	double get_cpu_time(){ \
	    return (double)clock() / CLOCKS_PER_SEC; \
	} \
	std::vector<double> step_cpu_duration(N_STEP, 0.0); \
	std::vector<double> step_wall_duration(N_STEP, 0.0);

#include "../src/rules.hpp"
#include "../src/utils/parser.hpp"

// debug levels
#define TEST_STEP_DEBUG_LEVEL 0.2
#define GRAPH_SIZE_DEBUG_LEVEL 0.1

#define _USE_MATH_DEFINES
#include <math.h>

int main(int argc, char* argv[]) {
	std::setvbuf(stdout, NULL, _IONBF, 0);

	cxxopts::Options options("file used for multi-threading test (does not run reversed iterations");
	auto [s, rule, _, n_iter, __, normalize, n_fast] = test_parser(options, argc, argv);

	double total_cpu_time = -get_cpu_time();
	double total_wall_time = -get_wall_time();

	for (int i = 0; i < n_iter; ++i) {
		if (n_fast == 0 || i % (n_fast + 1) == 0) {
			s.step(*rule, normalize);
		} else
			s.fast_step(*rule, normalize);
		
		move_all(s);
	}

	total_cpu_time += get_cpu_time();
	total_wall_time += get_wall_time();

	printf("{ \"wall\" : { \"steps\" : [");
	for (unsigned int i = 0; i < N_STEP; ++i) {
		printf("%f", step_wall_duration[i]);
		if (i < N_STEP - 1) {
			printf(", ");
		} else
			printf("], \"total\" : %f }", total_wall_time);
	}
	printf(", \"cpu\" : { \"steps\" : [");
	for (unsigned int i = 0; i < N_STEP; ++i) {
		printf("%f", step_cpu_duration[i]);
		if (i < N_STEP - 1) {
			printf(", ");
		} else
			printf("], \"total\" : %f }", total_cpu_time);
	}
	printf("}");
}