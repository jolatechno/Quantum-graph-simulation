#include <vector>

class unfiorm {
private:
	std::vector<std::default_random_engine> generators;
	std::vector<std::uniform_real_distribution<double>> distributions;

public:
	unfiorm() {
		int num_thread;

		#pragma omp parallel
		#pragma omp single
		num_thread = omp_get_num_threads();

		generators = std::vector<std::default_random_engine>(num_thread);
		distributions = std::vector<std::uniform_real_distribution<double>>(num_thread);

		for (int i = 0; i < num_thread; ++i)
			distributions[i] = std::uniform_real_distribution<double>(0, 1);
	}

	double operator()() {
		int thread_id = omp_get_thread_num();
		return distributions[thread_id](generators[thread_id]);
	}
};