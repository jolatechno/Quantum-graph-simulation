#include <algorithm>

template <class RandomReadIt, class Value, class RandomWriteIt>
void inline radix_indexed_sort_offset(RandomReadIt begin, RandomReadIt end, Value valueBegin, RandomWriteIt outBegin, int offset) {
	// declaration of the count variable
	unsigned int count[256] = {0};

	// counting occurences of each key
	for (RandomReadIt it = begin; it != end; ++it) {
		unsigned char key = valueBegin[*it] >> offset;
		++count[key];
	}

	// accumulating occurences to get indexes
	std::partial_sum(count, count + 256, count);

	// mooving indexes
	size_t size = std::distance(begin, end);
	for (int i = size - 1; i >= 0; --i) {
		unsigned char key = valueBegin[begin[i]] >> offset;
		unsigned int idx = --count[key];
		outBegin[idx] = begin[i];
	}
}

template <class RandomReadIt, class Value, class RandomWriteIt>
void inline parallel_radix_indexed_sort_offset(RandomReadIt begin, RandomReadIt end, Value valueBegin, RandomWriteIt outBegin, unsigned int *count, int offset) {
	// counting occurences of each key
	#pragma omp parallel for schedule(static)
	for (RandomReadIt it = begin; it != end; ++it) {
		unsigned char key = valueBegin[*it] >> offset;;

		#pragma omp atomic
		++count[key];
	}

	// accumulating occurences to get indexes
	__gnu_parallel::partial_sum(count, count + 256, count);

	// mooving indexes
	size_t size = std::distance(begin, end);
	for (int i = size - 1; i >= 0; --i) {
		unsigned char key = valueBegin[begin[i]] >> offset;
		unsigned int idx = --count[key];
		outBegin[idx] = begin[i];
	}
}