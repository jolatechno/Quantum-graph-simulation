template <class UnsignedIntIterator1, class UnsignedIntIterator2>
void inline load_balancing_from_prefix_sum(UnsignedIntIterator1 prefixSumLoadBegin, UnsignedIntIterator1 prefixSumLoadEnd,
	UnsignedIntIterator2 workSharingIndexesBegin, UnsignedIntIterator2 workSharingIndexesEnd) {

	unsigned int num_segments = std::distance(workSharingIndexesBegin, workSharingIndexesEnd);
	unsigned int num_elements = std::distance(prefixSumLoadBegin, prefixSumLoadEnd);

	workSharingIndexesBegin[0] = 0;
	workSharingIndexesBegin[num_segments - 1] = num_elements;

	/* !!!!!!
	TODO:
		Fix load balancing
	 !!!!!! */
	for (unsigned int segment = 1; segment < num_segments - 1; ++segment)
		workSharingIndexesBegin[segment] = (segment * num_elements) / (num_segments - 1);

	/*unsigned int total_sum = prefixSumLoadBegin[num_elements - 1];

	unsigned int i = 0;
	for (unsigned int segment = 1; segment < num_segments - 1; ++segment) {

		unsigned int segment_limit = (total_sum * segment) / (num_segments - 1);

		++i;
		for (; i < num_elements; ++i)
			if (prefixSumLoadBegin[i] > segment_limit) {
				if (segment_limit - prefixSumLoadBegin[i] > prefixSumLoadBegin[i - 1] - segment_limit) {
					workSharingIndexesBegin[segment] = i - 1;
				} else
					workSharingIndexesBegin[segment] = i;

				break;
			}

		if (i >= num_elements)
			workSharingIndexesBegin[segment] = num_elements;
	}*/
		
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