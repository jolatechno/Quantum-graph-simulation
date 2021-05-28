#include "../classical/rules.hpp"
#include "../classical/graph.hpp"
#include "../quantum/rules.hpp"
#include "../quantum/state.hpp"
#include "../quantum/checks.hpp"
#include <stdio.h>

#define _USE_MATH_DEFINES
#include <cmath>

int main() {
    //graph_t* g_1 = new graph(6, {3},  {3});

    graph_t* g_1 = new graph(30);
    //g_1->randomize();
    g_1->randomize(3);

    state_t* s = new state(g_1);
	s->set_params(M_PI_4, M_PI_2);

    for (int i = 1;; ++i) {

        auto [avg, std_dev] = s->size_stat();
        printf("%ld graph of size %Lf±%Lf at the %dth iteration\n", s->graphs().data.size(), avg, std_dev, i);

        #ifdef TEST
            #ifdef FULL
                    printf("checking graphs in fulls...\n");
                    if (!full_check(s))
                        return -1;
                    printf("...OK\n");
            #else
                printf("checking graphs...\n");
                if (!check(s))
                    return -1;
                printf("...OK\n");
            #endif
        #endif

        printf("\nstep_split_merge_all() ...\n"); s->step_split_merge_all();
		printf("reduce_all() ...\n\n"); s->reduce_all(); printf("\n");
   	}
}
