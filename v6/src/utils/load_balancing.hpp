template <class UnsignedIntIterator1, class UnsignedIntIterator2>
bool inline probe_load_sharing(unsigned long int wprobe,
	UnsignedIntIterator1 prefixSumLoadBegin, UnsignedIntIterator1 prefixSumLoadEnd,
	UnsignedIntIterator2 workSharingIndexesBegin, UnsignedIntIterator2 workSharingIndexesEnd) {

	unsigned long int num_segments = std::distance(workSharingIndexesBegin, workSharingIndexesEnd) - 1;
	unsigned long int num_elements = std::distance(prefixSumLoadBegin, prefixSumLoadEnd);

	/* separators */
	unsigned long int separators[num_segments - 1] = { 0 };
	unsigned long int last_separator = 0;
	unsigned long int last_separator_value = 0;

	/* find separators */
	for (unsigned long int separator = 0; separator < num_segments - 1; ++separator) {
		if (prefixSumLoadBegin[num_elements - 1] - last_separator_value > wprobe * (num_segments - separator))
			return false;

		/* dichotomie search of separator */
		last_separator = std::distance(prefixSumLoadBegin,
			std::upper_bound(prefixSumLoadBegin + last_separator, prefixSumLoadEnd, wprobe + last_separator_value));

		/* prepare next iteration */
		last_separator_value = last_separator == 0 ? 0 : prefixSumLoadBegin[last_separator - 1];
		separators[separator] = last_separator;
	}

	/* check last segment */
	if (prefixSumLoadBegin[num_elements - 1] - last_separator_value > wprobe)
		return false;

	/* copy separators over */
	for (unsigned long int i = 0; i < num_segments - 1; ++i)
		workSharingIndexesBegin[i + 1] = separators[i];

	return true;
}

template <class UnsignedIntIterator1, class UnsignedIntIterator2>
void inline load_balancing_from_prefix_sum(UnsignedIntIterator1 prefixSumLoadBegin, UnsignedIntIterator1 prefixSumLoadEnd,
	UnsignedIntIterator2 workSharingIndexesBegin, UnsignedIntIterator2 workSharingIndexesEnd) {

	unsigned long int num_segments = std::distance(workSharingIndexesBegin, workSharingIndexesEnd) - 1;
	unsigned long int num_elements = std::distance(prefixSumLoadBegin, prefixSumLoadEnd);

	unsigned long int w_min = prefixSumLoadBegin[num_elements - 1] / num_segments;
	unsigned long int w_max = 2*w_min + 1;

	while(!probe_load_sharing(w_max,
		prefixSumLoadBegin, prefixSumLoadEnd,
		workSharingIndexesBegin, workSharingIndexesEnd)) { w_max *= 2; };

	while (w_max - w_min > 1) {
		unsigned long int middle = (w_min + w_max) / 2;

		bool passed = probe_load_sharing(middle,
			prefixSumLoadBegin, prefixSumLoadEnd,
			workSharingIndexesBegin, workSharingIndexesEnd);

		if (passed) {
			w_max = middle;
		} else
			w_min = middle;
	}

	workSharingIndexesBegin[0] = 0;
	workSharingIndexesBegin[num_segments] = num_elements;
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