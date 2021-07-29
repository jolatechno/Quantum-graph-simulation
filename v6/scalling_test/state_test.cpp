#include <chrono>

/* number of step */
#define N_STEP 9

/* define a custom "MID_STEP_FUNCTION(n)" that accumulate the time taken in each step */
#define MID_STEP_FUNCTION(n) \
	if (n > 0 && n < N_STEP) \
		step_duration[n - 1] += std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - step_time[n - 1]).count() / 1e3; \
	if (n < N_STEP - 1) \
		step_time[n] = std::chrono::steady_clock::now();

/* global vector to accumulate */
#define STATE_GLOBAL_HEADER \
	std::vector<std::chrono::steady_clock::time_point> step_time(N_STEP); \
	std::vector<double> step_duration(N_STEP, 0.0);

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
	auto [s, rule, _, n_iter, __, normalize] = test_parser(options, argc, argv);

	auto begin = std::chrono::steady_clock::now();

	for (int i = 0; i < n_iter; ++i) {
		s.step(*rule, normalize);
		move_all(s);
	}

	auto end = std::chrono::steady_clock::now();

	printf("{ \"steps\" : [");
	for (unsigned int i = 0; i < N_STEP; ++i) {
		printf("%f", step_duration[i]);
		if (i < N_STEP - 1) {
			printf(", ");
		} else
			printf("], \"total\" : %f }", std::chrono::duration<double, std::milli>(end - begin).count() / 1e3);
	}
}