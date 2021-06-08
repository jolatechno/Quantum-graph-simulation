#pragma once

#include "state.hpp"
#include "rules.hpp"
#include "../classical/checks.hpp"

const double probability_tolerance = 1e-5;

bool check(state_t* s) {
	auto data = s->graphs();
	auto end = data.end();
	PROBA_TYPE probability = 0;

	//iterate over all graphs
    for(auto it = data.begin(); it != data.end(); ++it) {
    	//add probability
    	probability += std::norm(it->second);

    	//check if a probability is under zero
		/*if (check_zero(it->second)) {
			printf("probability is zero!!\n");
			return false;
		}*/

		//check graph
		if (!graph_checker(*it->first))
			return false;

    	//iterate over all graphs
    	if (data.count(it->first) != 1) {
			printf("two graphs with the same hash!!\n");
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