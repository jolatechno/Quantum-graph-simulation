template <class UnsignedIntIterator1, class UnsignedIntIterator2>
void inline load_balancing_from_prefix_sum(UnsignedIntIterator1 prefixSumLoadBegin, UnsignedIntIterator1 prefixSumLoadEnd,
	UnsignedIntIterator2 workSharingIndexesBegin, UnsignedIntIterator2 workSharingIndexesEnd) {

	unsigned int num_segments = std::distance(workSharingIndexesBegin, workSharingIndexesEnd) - 1;
	unsigned int num_elements = std::distance(prefixSumLoadBegin, prefixSumLoadEnd);

	float ideal_segment = (float)num_elements / (float)num_segments;

	workSharingIndexesBegin[0] = 0;
	workSharingIndexesBegin[num_segments] = num_elements;

	/* initial guess */
	for (unsigned int segment = 1; segment < num_segments; ++segment)
		workSharingIndexesBegin[segment] = (segment * num_elements) / num_segments;

	/* !!!
	TODO:
		implement improving the guess
	 !!! */
}

template <class UnsignedIntIterator1, class UnsignedIntIterator2>
void inline indexed_load_balancing_from_prefix_sum(UnsignedIntIterator1 prefixSumLoadBegin, UnsignedIntIterator1 prefixSumLoadEnd,
	UnsignedIntIterator2 workSharingIndexesBegin, UnsignedIntIterator2 workSharingIndexesEnd) {

	/* load balance */
	load_balancing_from_prefix_sum(prefixSumLoadBegin, prefixSumLoadEnd, workSharingIndexesBegin, workSharingIndexesEnd);

	/* de-index limits from prefix sum */
	for (auto workSharingIt = workSharingIndexesBegin; workSharingIt != workSharingIndexesEnd; ++workSharingIt)
		*workSharingIt = *workSharingIt == 0 ? 0 : prefixSumLoadBegin[*workSharingIt - 1];
}