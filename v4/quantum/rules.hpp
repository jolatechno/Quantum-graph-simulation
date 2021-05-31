#pragma once

#include <vector>
#include <math.h>  //pow
#include <utility> //for pairs
#include <complex> //for complex
#include "../classical/graph.hpp"

bool inline check_zero(const std::complex<long double>& mag) {
	const long double double_tolerance = 1e-30;
	return std::norm(mag) <= double_tolerance;
	/*const std::complex<long double> zero = 0;
	return mag == zero;*/
}

/* count the number of subsets */
template<class T>
int num_subset(std::vector<T> const &vect) {
	return pow(2, vect.size());
}

/* create a subset and a probability from the subset indice */
std::pair<std::vector<graph::split_merge_t>, std::complex<long double>> subset(std::vector<graph::split_merge_t>& split_merge, int subset_numb,
	std::complex<long double>& non_merge, std::complex<long double>& merge) {
	
	std::vector<graph::split_merge_t> res;
	std::complex<long double> proba = 1;

	for (int i = 0; i < split_merge.size(); ++i) {
		long double sign = 1 - 2*split_merge[i].second;

		/* i_th bit of subset_numb */
		if (subset_numb%2) {
			/* add opperation to res */
			res.push_back(split_merge[i]);

			/* get proba */
			proba *= std::complex<long double>(merge.real(), sign*merge.imag());
		} else
			proba *= sign*non_merge;


		subset_numb /= 2;
	}

	return {res, proba};
}

std::pair<std::vector<graph::split_merge_t>, std::complex<long double>> subset(std::vector<graph::split_merge_t>& split_merge, int subset_numb,
	long double teta, long double phi) {

	/* compute amplitudes */
	std::complex<long double> non_merge = std::cos(teta);
	std::complex<long double> merge = std::polar(std::sin(teta), phi);

	return subset(split_merge, subset_numb, non_merge, merge);
}