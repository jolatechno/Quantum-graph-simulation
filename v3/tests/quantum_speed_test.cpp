#include "../classical/rules.hpp"
#include "../classical/graph.hpp"
#include "../quantum/rules.hpp"
#include "../quantum/state.hpp"
#include "../quantum/checks.hpp"
#include <stdio.h>

#define _USE_MATH_DEFINES
#include <cmath>

int main() {
    graph_t* g_1 = new graph(6);
    g_1->randomize();

    state_t* s = new state(g_1);
	s->set_params(M_PI_4, 0);

    for (int i = 1;; ++i) {
    	#ifdef TEST
    	if (check(s))
    		return -1;
    	#endif

    	printf("%ld graphs at the %dth iteration\n", s->graphs().size(), i);

    	printf("\nsplit_merge_all() ...\n"); s->split_merge_all();
    	printf("step_all() ...\n"); s->step_all();
		  printf("sort_and_reduce() ...\n\n"); s->sort_and_reduce(); printf("\n");
   	}
}
