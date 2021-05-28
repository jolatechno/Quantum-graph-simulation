#pragma once

#include <vector>
#include <math.h>  //pow
#include <utility> //for pairs
#include <complex> //for complex
#include "../classical/graph.hpp"

const double double_tolerance = 0; //1e-32;

/* count the number of subsets */
template<class T>
int num_subset(std::vector<T> const &vect) {
	return pow(2, vect.size());
}

/* create a subset and a probability from the subset indice */
std::pair<std::vector<graph::split_merge_t>, std::complex<double>> subset(std::vector<graph::split_merge_t>& split_merge, int subset_numb,
	std::complex<double> non_merge, std::complex<double> non_split, std::complex<double> merge, std::complex<double> split) {
	
	std::vector<graph::split_merge_t> res;
	std::complex<double> proba = 1;

	for (int i = 0; i < split_merge.size(); ++i) {
		/* i_th bit of subset_numb */
		if (subset_numb%2) {
			/* add opperation to res */
			res.push_back(split_merge[i]);

			/* get proba */
			if (split_merge[i].second) {
				proba *= split;
			} else
				proba *= merge;
		} else
			if (split_merge[i].second) {
				proba *= non_split;
			} else
				proba *= non_merge;


		subset_numb /= 2;
	}

	return std::pair<std::vector<graph::split_merge_t>, std::complex<double>>(res, proba);
}

std::pair<std::vector<graph::split_merge_t>, std::complex<double>> subset(std::vector<graph::split_merge_t>& split_merge, int subset_numb,
	double teta, double phi) {

	/* compute amplitudes */
	std::complex<double> non_split = std::cos(teta);
	std::complex<double> non_merge = -std::cos(teta);

	double r = std::sin(teta);
	std::complex<double> merge = std::polar(r, phi);
	std::complex<double> split = std::polar(r, -phi);

	return subset(split_merge, subset_numb, non_merge, non_split, merge, split);
}