#pragma once

#include <vector>
#include <complex> //for complex
#include "state.hpp"
#include "rules.hpp"
#include "../classical/checks.hpp"

const long double probability_tolerance = 1e-5;

bool full_check(state_t* s) {
	auto data = s->graphs()/*.data*/;
	auto end = data.end();
	long double probability = 0;

	//iterate over all graphs
	for (auto it = data.begin(); it != end; ++it) {
		//add probability
		probability += std::norm(it->second.second);

		//check if a probability is under zero
		if (check_zero(it->second.second)) {
			printf("probability is zero!!\n");
			return false;
		}

		//check graph
		if (!graph_checker(it->second.first))
			return false;

		//check for equal graphs
		for (auto jt = data.begin(); jt != it; ++jt)
			if (it->second.first->equal(jt->second.first)) {
				printf("two graphs equal!!\n");
				return false;
			}
	}

	//check for probability
	if (probability > 1 + probability_tolerance || probability < 1 - probability_tolerance) {
		printf("probability not equal 1 (%Lf)!!\n", probability);
		return false;
	}

	return true;
}

bool check(state_t* s) {
	auto data = s->graphs()/*.data*/;
	auto end = data.end();
	long double probability = 0;

	//iterate over all keys
	decltype(data.equal_range("")) range;
    for(auto it = data.begin(); it != data.end(); it = range.second) {

    	//iterate over all graphs
    	range = data.equal_range(it->first);
		for(auto jt = range.first; jt != range.second; ++jt)
        	if (!check_zero(jt->second.second)) {
        		//add probability
				probability += std::norm(jt->second.second);

				//check if a probability is under zero
				if (check_zero(jt->second.second)) {
					printf("probability is zero!!\n");
					return false;
				}

				//check graph
				if (!graph_checker(jt->second.first))
					return false;

        		// next iterator
        		auto kt = jt;
        		++kt;

				//check for equal graphs
				for (; kt != range.second; ++kt)
					if (kt->second.first->equal(jt->second.first)) {
						printf("two graphs equal!!\n");
						return false;
					}
        	}
        }

	//check for probability
	if (probability > 1 + probability_tolerance || probability < 1 - probability_tolerance) {
		printf("probability not equal 1 (%Lf)!!\n", probability);
		return false;
	}

	return true;
}