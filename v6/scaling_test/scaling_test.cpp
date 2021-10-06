#include <time.h>
#include <vector>
#include <sys/time.h>

/* global vector to accumulate */
#define STATE_GLOBAL_HEADER \
	double get_time() { \
	    struct timeval time; \
	    gettimeofday(&time,NULL); \
	    return (double)time.tv_sec + (double)time.tv_usec * .000001; \
	} \
	std::vector<double> step_duration(N_STEP, 0.0);

/* define a custom "MID_STEP_FUNCTION(n)" that accumulate the time taken in each step */
#define MID_STEP_FUNCTION(n) \
	if (n > 0) { \
		step_duration[n - 1] += get_time(); \
	}

/* number of step */
#define N_STEP 8

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
	double total_time = -get_time();

	for (int i = 0; i < n_iter; ++i) {
		if (n_fast == 0 || i % (n_fast + 1) == 0) {
			s.step(*rule);
		} else
			s.fast_step(*rule);
		
		move_all(s);
	}

	total_time += get_time();

	/* print results as json */
	printf("\t\"steps\" : [");
	for (unsigned int i = 0; i < N_STEP; ++i) {
		printf("%f", step_duration[i]);
		if (i < N_STEP - 1) {
			printf(", ");
		} else
			printf("],\n\t\"total\" : %f\n}", total_time);
	}
}
