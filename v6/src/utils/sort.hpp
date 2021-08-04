#include <algorithm>

// parallel radix sort for the first byte of indexed values
void inline parallel_radix_indexed_sort_0(unsigned int *begin, unsigned int *end,
	unsigned int *outBegin,
	size_t *valueBegin,
	unsigned int *count) {

	// cast value to char
	unsigned char *char_valueBegin = (unsigned char*)valueBegin;

	// counting occurences of each key
	#pragma omp parallel for schedule(static)
	for (unsigned int *it = begin; it != end; ++it) {
		unsigned char key = char_valueBegin[8*(*it) + 7];

		#pragma omp atomic
		++count[key];
	}

	// accumulating occurences to get indexes
	__gnu_parallel::partial_sum(count, count + 256, count);

	// mooving indexes
	size_t size = std::distance(begin, end);
	for (int i = size - 1; i >= 0; --i) {
		unsigned int idx = --count[char_valueBegin[8*begin[i] + 7]];
		outBegin[idx] = begin[i];
	}
}

// second loop of the radix algorithm
void inline radix_secon_loop_indexed_offset(unsigned int *begin, unsigned int *outBegin,
	size_t size,
	unsigned char *char_valueBegin,
	unsigned int *count,
	int offset) {
	
	// mooving indexes
	for (int i = size - 1; i >= 0; --i) {
		unsigned int idx = --count[char_valueBegin[8*begin[i] + offset]];
		outBegin[idx] = begin[i];
	}
}

// sequential contracted radix sort for byte 7 to 1 for indexed values
void inline radix_indexed_sort_1_7(unsigned int *begin, unsigned int *end,
	unsigned int *outBegin,
	size_t *valueBegin) {

	// declaration of the count variable
	unsigned int count1[256] = {0};
	unsigned int count2[256] = {0};
	unsigned int count3[256] = {0};
	unsigned int count4[256] = {0};
	unsigned int count5[256] = {0};
	unsigned int count6[256] = {0};
	unsigned int count7[256] = {0};

	// cast value to char
	unsigned char* char_valueBegin = (unsigned char*)valueBegin;

	// counting occurences of each key
	for (unsigned int *it = begin; it != end; ++it) {
		unsigned char* keys = char_valueBegin + 8*(*it);

		++count1[keys[6]];
		++count2[keys[5]];
		++count3[keys[4]];
		++count4[keys[3]];
		++count5[keys[2]];
		++count6[keys[1]];
		++count7[keys[0]];
	}

	// accumulating occurences to get indexes
	std::partial_sum(count1, count1 + 256, count1);
	std::partial_sum(count2, count2 + 256, count2);
	std::partial_sum(count3, count3 + 256, count3);
	std::partial_sum(count4, count4 + 256, count4);
	std::partial_sum(count5, count5 + 256, count5);
	std::partial_sum(count6, count6 + 256, count6);
	std::partial_sum(count7, count7 + 256, count7);

	// mooving indexes
	size_t size = std::distance(begin, end);
	radix_secon_loop_indexed_offset(begin, outBegin, size, char_valueBegin, count1, 6);
	radix_secon_loop_indexed_offset(outBegin, begin, size, char_valueBegin, count2, 5);
	radix_secon_loop_indexed_offset(begin, outBegin, size, char_valueBegin, count3, 4);
	radix_secon_loop_indexed_offset(outBegin, begin, size, char_valueBegin, count4, 3);
	radix_secon_loop_indexed_offset(begin, outBegin, size, char_valueBegin, count5, 2);
	radix_secon_loop_indexed_offset(outBegin, begin, size, char_valueBegin, count6, 1);
	radix_secon_loop_indexed_offset(begin, outBegin, size, char_valueBegin, count7, 0);
}