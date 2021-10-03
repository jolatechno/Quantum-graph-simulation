#include "../../v6/src/utils/sort.hpp"
#include "../../v6/src/utils/load_balancing.hpp"
#include <time.h>
#include <iostream>
#include <chrono>
#include <omp.h>
#include <mutex>
#include <unordered_map>

#ifdef USE_TBB
	#include <tbb/concurrent_hash_map.h> // For concurrent hash map.
#endif

#ifndef N_STEP
	#define N_STEP 5
#endif

#ifndef FIRST_SIZE
	#define FIRST_SIZE 2000
#endif

#ifndef END_SIZE
	#define END_SIZE 20000000
#endif

#ifndef PROPORTION
	#define PROPORTION 0.3
#endif

const size_t num_threads = []() {
	int num_threads;
	#pragma omp parallel
	#pragma omp single
	num_threads = omp_get_num_threads();

	return num_threads;
}();

int g_seed = 0;
inline int fastrand() { 
  g_seed = (214013*g_seed+2531011); 
  return (g_seed>>16)&0x7FFF; 
} 

void random_list_generator(long unsigned int *arr, size_t size) {
	char *char_arr = (char*)arr;

	for (size_t i = 0; i < 8*size; ++i)
		char_arr[i] = fastrand();

	for (size_t i = 0; i < PROPORTION*size/2; ++i) {
		size_t idx = arr[i] % size;
		arr[idx] = arr[i];
	}
}

void radix_sort_elimination(long unsigned int *next_hash, long unsigned int *next_gid, long unsigned int *next_gid_buffer, bool *is_last_index, double *values, size_t size) {
	std::vector<size_t> work_sharing_begin = std::vector<size_t>(num_threads + 1);
	
	#pragma omp parallel
	{
		size_t thread_id = omp_get_thread_num();

		#pragma omp single
		{
			/* share work according to hash */
			size_t count[256] = {0};
			parallel_radix_count_indexed_0(next_gid, next_gid + size, /* 1 size_t write and 1 (index) + 2 size_t and 1 char reads per symbolic graph */
				next_hash,	/* + 1 size_t read and 1 size_t write per symbolic graphs */
				count);

			/* share work according to count */
			indexed_load_balancing_from_prefix_sum(count, count + 256, work_sharing_begin.begin(), work_sharing_begin.end());

			/* finish radix sort */
			radix_secon_loop_indexed_offset(next_gid, next_gid_buffer,
				size,
				(unsigned char*)next_hash,
				count,
				7);
		}
					
		if (work_sharing_begin[thread_id] < work_sharing_begin[thread_id + 1]) {
			/* sort all graphs */
			radix_indexed_sort_1_7(next_gid_buffer + work_sharing_begin[thread_id], /* 7 size_t write and 1 (index) + 7*2 size_t and 7 char reads per symbolic graph */
				next_gid_buffer + work_sharing_begin[thread_id + 1], /* + 7 size_t read and 7 size_t write per symbolic graphs */
				next_gid + work_sharing_begin[thread_id],
				next_hash);
							
			/* set is_last_index of the last graph */
			is_last_index[next_gid[work_sharing_begin[thread_id + 1] - 1]] = true;

			/* compute is_last_index */
			for (size_t gid = work_sharing_begin[thread_id]; gid < work_sharing_begin[thread_id + 1] - 1; ++gid)
				is_last_index[next_gid[gid]] = next_hash[next_gid[gid]] != next_hash[next_gid[gid + 1]]; /* 1 char write and 4 size_t reads per symbolic graph */

			/* partial sum over the interval since we know it starts and end at unique graphs */
			double sign;
			size_t last_id = next_gid[work_sharing_begin[thread_id]];
			for (size_t gid = work_sharing_begin[thread_id] + 1; gid < work_sharing_begin[thread_id + 1]; ++gid) {
				size_t id = next_gid[gid]; /* 1 size_t read per symbolic graphs */
				sign = !is_last_index[last_id]; /* 1 char read per symbolic graph */

				/* add probabilites of graph with equal hashes */
				values[id] += sign*values[last_id];
				last_id = id;
			}
		}

		#pragma omp barrier

		#pragma omp single
		{
			auto partitioned_it = next_gid + size;
			/* get all unique graphs with a non zero probability */
			partitioned_it = __gnu_parallel::partition(next_gid, partitioned_it, /* 1 size_t read and 1 size_t write per symbolic graph */
				[&](size_t const &gid) { return is_last_index[gid]; });
		}
	}
}

void radix_quick_sort_elimination(long unsigned int *next_hash, long unsigned int *next_gid, long unsigned int *next_gid_buffer, bool *is_last_index, double *values, size_t size) {
	std::vector<size_t> work_sharing_begin = std::vector<size_t>(num_threads + 1);
	
	#pragma omp parallel
	{
		size_t thread_id = omp_get_thread_num();

		#pragma omp single
		{
			/* share work according to hash */
			size_t count[256] = {0};
			parallel_radix_count_indexed_0(next_gid, next_gid + size, /* 1 size_t write and 1 (index) + 2 size_t and 1 char reads per symbolic graph */
				next_hash,	/* + 1 size_t read and 1 size_t write per symbolic graphs */
				count);

			/* share work according to count */
			indexed_load_balancing_from_prefix_sum(count, count + 256, work_sharing_begin.begin(), work_sharing_begin.end());

			/* finish radix sort */
			radix_secon_loop_indexed_offset(next_gid, next_gid_buffer,
				size,
				(unsigned char*)next_hash,
				count,
				7);
		}
					
		if (work_sharing_begin[thread_id] < work_sharing_begin[thread_id + 1]) {
			/* sort all graphs */
			std::sort(next_gid_buffer + work_sharing_begin[thread_id], next_gid_buffer + work_sharing_begin[thread_id + 1],
				[&](long unsigned int gid1, long unsigned int gid2){
					return next_hash[gid1] < next_hash[gid2];
				});
							
			/* set is_last_index of the last graph */
			is_last_index[next_gid_buffer[work_sharing_begin[thread_id + 1] - 1]] = true;

			/* compute is_last_index */
			for (size_t gid = work_sharing_begin[thread_id]; gid < work_sharing_begin[thread_id + 1] - 1; ++gid)
				is_last_index[next_gid_buffer[gid]] = next_hash[next_gid_buffer[gid]] != next_hash[next_gid_buffer[gid + 1]]; /* 1 char write and 4 size_t reads per symbolic graph */

			/* partial sum over the interval since we know it starts and end at unique graphs */
			double sign;
			size_t last_id = next_gid_buffer[work_sharing_begin[thread_id]];
			for (size_t gid = work_sharing_begin[thread_id] + 1; gid < work_sharing_begin[thread_id + 1]; ++gid) {
				size_t id = next_gid_buffer[gid]; /* 1 size_t read per symbolic graphs */
				sign = !is_last_index[last_id]; /* 1 char read per symbolic graph */

				/* add probabilites of graph with equal hashes */
				values[id] += sign*values[last_id];
				last_id = id;
			}
		}

		#pragma omp barrier

		#pragma omp single
		{
			auto partitioned_it = next_gid_buffer + size;
			/* get all unique graphs with a non zero probability */
			partitioned_it = __gnu_parallel::partition(next_gid_buffer, partitioned_it, /* 1 size_t read and 1 size_t write per symbolic graph */
				[&](size_t const &gid) { return is_last_index[gid]; });
		}
	}
}

void single_threaded_radix_sort_elimination(long unsigned int *next_hash, long unsigned int *next_gid, long unsigned int *next_gid_buffer, bool *is_last_index, double *values, size_t size) {
	size_t count[256] = {0};
		parallel_radix_count_indexed_0(next_gid, next_gid + size, /* 1 size_t write and 1 (index) + 2 size_t and 1 char reads per symbolic graph */
		next_hash,	/* + 1 size_t read and 1 size_t write per symbolic graphs */
		count);

	radix_indexed_sort_1_7(next_gid_buffer, /* 7 size_t write and 1 (index) + 7*2 size_t and 7 char reads per symbolic graph */
		next_gid_buffer + size, /* + 7 size_t read and 7 size_t write per symbolic graphs */
		next_gid,
		next_hash);

	is_last_index[next_gid[size - 1]] = true;

	/* compute is_last_index */
	for (size_t gid = 0; gid < size - 1; ++gid)
		is_last_index[next_gid[gid]] = next_hash[next_gid[gid]] != next_hash[next_gid[gid + 1]]; /* 1 char write and 4 size_t reads per symbolic graph */

	double sign;
	size_t last_id = next_gid[0];
	for (size_t gid = 1; gid < size; ++gid) {
		size_t id = next_gid[gid]; /* 1 size_t read per symbolic graphs */
		sign = !is_last_index[last_id]; /* 1 char read per symbolic graph */

		/* add probabilites of graph with equal hashes */
		values[id] += sign*values[last_id];
		last_id = id;
	}

	auto partitioned_it = next_gid + size;
	/* get all unique graphs with a non zero probability */
	partitioned_it = std::partition(next_gid, partitioned_it, /* 1 size_t read and 1 size_t write per symbolic graph */
		[&](size_t const &gid) { return is_last_index[gid]; });
}

void quick_sort_elimination(long unsigned int *next_hash, long unsigned int *next_gid, bool *is_last_index, double *values, size_t size) {
	#pragma omp parallel
	{
		size_t thread_id = omp_get_thread_num();

		#pragma omp single
		{
			__gnu_parallel::sort(next_gid, next_gid + size,
				[&](long unsigned int gid1, long unsigned int gid2){
					return next_hash[gid1] < next_hash[gid2];
				});

			is_last_index[next_gid[size - 1]] = true;
		}

		/* compute is_last_index */
		#pragma omp for
		for (size_t gid = 0; gid < size - 1; ++gid)
			is_last_index[next_gid[gid]] = next_hash[next_gid[gid]] != next_hash[next_gid[gid + 1]]; /* 1 char write and 4 size_t reads per symbolic graph */

		#pragma omp barrier

		size_t begin = (thread_id * size) / num_threads;
		size_t end = ((thread_id + 1) * size) / num_threads;

		if (thread_id != num_threads - 1)
			while(!is_last_index[end - 1])
				++end;
		if (thread_id != 0)
			while(!is_last_index[begin - 1])
				++begin; 

		double sign;
		size_t last_id = next_gid[begin];
		for (size_t gid = begin + 1; gid < end; ++gid) {
			size_t id = next_gid[gid]; /* 1 size_t read per symbolic graphs */
			sign = !is_last_index[last_id]; /* 1 char read per symbolic graph */

			/* add probabilites of graph with equal hashes */
			values[id] += sign*values[last_id];
			last_id = id;
		}

		#pragma omp barrier

		#pragma omp single
		{
			auto partitioned_it = next_gid + size;
			/* get all unique graphs with a non zero probability */
			partitioned_it = __gnu_parallel::partition(next_gid, partitioned_it, /* 1 size_t read and 1 size_t write per symbolic graph */
				[&](size_t const &gid) { return is_last_index[gid]; });
		}
	}
}

void single_threaded_quick_sort_elimination(long unsigned int *next_hash, long unsigned int *next_gid, bool *is_last_index, double *values, size_t size) {
	std::sort(next_gid, next_gid + size,
		[&](long unsigned int gid1, long unsigned int gid2){
			return next_hash[gid1] < next_hash[gid2];
		});

	is_last_index[next_gid[size - 1]] = true;

	/* compute is_last_index */
	for (size_t gid = 0; gid < size - 1; ++gid)
		is_last_index[next_gid[gid]] = next_hash[next_gid[gid]] != next_hash[next_gid[gid + 1]]; /* 1 char write and 4 size_t reads per symbolic graph */

	double sign;
	size_t last_id = next_gid[0];
	for (size_t gid = 1; gid < size; ++gid) {
		size_t id = next_gid[gid]; /* 1 size_t read per symbolic graphs */
		sign = !is_last_index[last_id]; /* 1 char read per symbolic graph */

		/* add probabilites of graph with equal hashes */
		values[id] += sign*values[last_id];
		last_id = id;
	}

	auto partitioned_it = next_gid + size;
	/* get all unique graphs with a non zero probability */
	partitioned_it = std::partition(next_gid, partitioned_it, /* 1 size_t read and 1 size_t write per symbolic graph */
		[&](size_t const &gid) { return is_last_index[gid]; });
}

void single_threaded_hashmap_elimination(long unsigned int *next_hash, long unsigned int *next_gid, double *values, size_t size) {
	std::unordered_map<size_t, size_t> map;

	for (unsigned int i = 0; i < size; ++i) {
		size_t gid = next_gid[i];
		size_t hash = next_hash[gid];

		auto it = map.find(hash);
		if (it == map.end()) {
			map.insert({hash, gid});
		} else {
			values[it->second] += values[gid];
		}
	}

	int i = 0;
	for (auto it = map.begin(); it != map.end(); ++it, ++i)
		next_gid[i] = it->second;
}

#ifdef USE_TBB
void hashmap_elimination(long unsigned int *next_hash, long unsigned int *next_gid, bool *is_last_index, double *values, size_t size) {
	tbb::concurrent_hash_map<size_t, size_t> map;

	#pragma omp parallel for
	for (unsigned int i = 0; i < size; ++i) {
		size_t gid = next_gid[i];
		size_t hash = next_hash[gid];

		tbb::concurrent_hash_map<size_t, size_t>::accessor it;
		if (map.insert(it, hash)) {
			values[it->second] += values[gid];
			is_last_index[gid] = false;
		} else {
			it->second = gid;
			is_last_index[gid] = true;
		}
		it.release();
	}

	auto partitioned_it = next_gid + size;
	/* get all unique graphs with a non zero probability */
	partitioned_it = __gnu_parallel::partition(next_gid, partitioned_it, /* 1 size_t read and 1 size_t write per symbolic graph */
		[&](size_t const &gid) { return is_last_index[gid]; });
}
#endif

int main() {
	/* allow nested parallism for __gnu_parallel inside omp single */
	omp_set_nested(3);

	unsigned int n = N_STEP;
	size_t first_size = FIRST_SIZE;
	size_t end_size = END_SIZE;

	float r = (float)end_size / (float)first_size;
	r = std::pow(r, 1/(float)(n - 1)); 

	long unsigned int *arr = (long unsigned int*)malloc(end_size*8);
	random_list_generator(arr, end_size);

	std::cout << "{\n";
	for (int i = 0; i < n; ++i) {
		size_t size = first_size * std::pow(r, i);

		std::cout << "\t\"" << size << "\" : {\n";

		long unsigned int *idxs = (long unsigned int*)malloc(size*8);
		long unsigned int *idxs_buffer = (long unsigned int*)malloc(size*8);
		bool *is_last = (bool*)malloc(size);
		double *values = (double*)malloc(size*sizeof(double));

/* ------------------------------------------ */

		std::iota(idxs, idxs + size, 0);

		auto start = std::chrono::high_resolution_clock::now();
		radix_sort_elimination(arr, idxs, idxs_buffer, is_last, values, size);
		auto stop = std::chrono::high_resolution_clock::now();
		auto duration = duration_cast<std::chrono::microseconds>(stop - start);

		std::cout << "\t\t\"radix\" : " << (float)(duration.count()) * 1e-6 << ",\n";

/* ------------------------------------------ */

		std::iota(idxs, idxs + size, 0);

		start = std::chrono::high_resolution_clock::now();
		single_threaded_radix_sort_elimination(arr, idxs, idxs_buffer, is_last, values, size);
		stop = std::chrono::high_resolution_clock::now();
		duration = duration_cast<std::chrono::microseconds>(stop - start);

		std::cout << "\t\t\"single_threaded_radix\" : " << (float)(duration.count()) * 1e-6 << ",\n";

/* ------------------------------------------ */

		std::iota(idxs, idxs + size, 0);

		start = std::chrono::high_resolution_clock::now();
		radix_quick_sort_elimination(arr, idxs, idxs_buffer, is_last, values, size);
		stop = std::chrono::high_resolution_clock::now();
		duration = duration_cast<std::chrono::microseconds>(stop - start);

		std::cout << "\t\t\"hybrid_radix_quick\" : " << (float)(duration.count()) * 1e-6 << ",\n";

/* ------------------------------------------ */

		std::iota(idxs, idxs + size, 0);

		start = std::chrono::high_resolution_clock::now();
		quick_sort_elimination(arr, idxs, is_last, values, size);
		stop = std::chrono::high_resolution_clock::now();
		duration = duration_cast<std::chrono::microseconds>(stop - start);

		std::cout << "\t\t\"quick_sort\" : " << (float)(duration.count()) * 1e-6 << ",\n";

/* ------------------------------------------ */

		std::iota(idxs, idxs + size, 0);

		start = std::chrono::high_resolution_clock::now();
		single_threaded_quick_sort_elimination(arr, idxs, is_last, values, size);
		stop = std::chrono::high_resolution_clock::now();
		duration = duration_cast<std::chrono::microseconds>(stop - start);

		std::cout << "\t\t\"single_threaded_quick_sort\" : " << (float)(duration.count()) * 1e-6 << ",\n";

/* ------------------------------------------ */

		std::iota(idxs, idxs + size, 0);

		start = std::chrono::high_resolution_clock::now();
		single_threaded_hashmap_elimination(arr, idxs, values, size);
		stop = std::chrono::high_resolution_clock::now();
		duration = duration_cast<std::chrono::microseconds>(stop - start);

		std::cout << "\t\t\"single_threaded_hashmap_elimination\" : " << (float)(duration.count()) * 1e-6 << "\n";

/* ------------------------------------------ */
#ifdef USE_TBB

		std::iota(idxs, idxs + size, 0);

		start = std::chrono::high_resolution_clock::now();
		hashmap_elimination(arr, idxs, is_last, values, size);
		stop = std::chrono::high_resolution_clock::now();
		duration = duration_cast<std::chrono::microseconds>(stop - start);

		std::cout << "\t\t\"hashmap_elimination\" : " << (float)(duration.count()) * 1e-6 << "\n";

#endif
/* ------------------------------------------ */

		std::cout << "\t}" << (i == n - 1 ? "" : ",") << "\n";

		free(idxs);
		free(idxs_buffer);
		free(is_last);
		free(values);
	}
	std::cout << "}\n";
	
	free(arr);
}