#include <time.h>
#include <vector>
#include <chrono>

/* number of step */
#define N_STEP 8

/* global vector to accumulate */
#define STATE_GLOBAL_HEADER \
	typedef std::chrono::time_point<std::chrono::high_resolution_clock> time_point; \
	std::vector<double> step_duration(N_STEP, 0.0); \
	std::vector<time_point> step_start(N_STEP);

/* define a custom "MID_STEP_FUNCTION(n)" that accumulate the time taken in each step */
#define MID_STEP_FUNCTION(n) \
	if (n > 0) { \
		time_point stop = std::chrono::high_resolution_clock::now(); \
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - step_start[n - 1]); \
		step_duration[n - 1] = duration.count() * 1e-6; \
	} \
	if (n < N_STEP) { \
		step_start[n] = std::chrono::high_resolution_clock::now(); \
	}

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

	auto static runner = [&]() {
		for (int i = 0; i < n_iter; ++i) {
			if (n_fast == 0 || i % (n_fast + 1) == 0) {
				s.step(*rule);
			} else
				s.fast_step(*rule);
				
			move_all(s);
		}
	};

	auto start = std::chrono::high_resolution_clock::now();
	runner();
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

	/* print results as json */
	printf("\t\"steps\" : [");
	for (unsigned int i = 0; i < N_STEP; ++i) {
		printf("%f", step_duration[i]);
		if (i < N_STEP - 1) {
			printf(", ");
		} else
			printf("],\n\t\"total\" : %f\n}", duration.count() * 1e-6);
	}
}
