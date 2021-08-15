#include <sys/time.h>
#include <time.h>
#include <cstdio>
#include <cstring>
#include <functional>

#ifndef NUM_ELMENTS
	#define NUM_ELMENTS 100000000
#endif
#ifndef NUM_TESTS
	#define NUM_TESTS 10
#endif

double get_wall_time() {
	struct timeval time;
	gettimeofday(&time, NULL);
	return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

double get_memory_bandwidth(std::function<void(size_t*&, size_t*&)> work) {
	size_t *in_array = new size_t[NUM_ELMENTS];
	size_t *out_array = new size_t[NUM_ELMENTS];

	double memory_delay = -get_wall_time();
	for (int i = 0; i < NUM_TESTS; ++i)
		work(in_array, out_array);
	memory_delay += get_wall_time();

	return  (double)sizeof(size_t) * NUM_TESTS * NUM_ELMENTS / memory_delay / 1e9;
}

int main(int argc, char* argv[]) {
	printf("{\n");
	
	printf("\t\"memcpy\" : %lf,\n",get_memory_bandwidth(
		[](size_t*& in, size_t*& out) {
			memcpy(in, out, sizeof(size_t) * NUM_ELMENTS);
		}));

	printf("\t\"multithreaded\" : %lf,\n", get_memory_bandwidth(
		[](size_t*& in, size_t*& out) {
			#pragma omp parallel for schedule(static)
			for (unsigned int i = 0; i != NUM_ELMENTS; ++i) out[i] = in[i];
		}));

	printf("\t\"singlethreaded\" : %lf\n",get_memory_bandwidth(
		[](size_t*& in, size_t*& out) {
			for (unsigned int i = 0; i != NUM_ELMENTS; ++i) out[i] = in[i];
		}));

	printf("}");
}