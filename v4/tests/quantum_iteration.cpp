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

    graph_t* g_1 = new graph(4);
    //g_1->randomize();
    g_1->randomize(3);

    state_t* s = new state(g_1);
    auto [non_merge, merge] = unitary(M_PI_4, M_PI_2);
    auto rule = step_split_merge_all(non_merge, merge);

    for (int i = 1; i < 11; ++i) {

        auto [avg, std_dev] = s->size_stat();
        printf("%ld graph of size %Lf±%Lf at the %dth iteration\n", s->graphs().size(), avg, std_dev, i);

        #ifdef TEST
            printf("checking graphs...\n");
            if (!check(s))
                return -1;
            printf("...OK\n");
        #endif

        printf("\nstep_split_merge_all() ...\n"); s->step_all(rule);

        //if (i % 3 == 0) {
            printf("reduce_all() ...\n\n"); s->reduce_all(); printf("\n");
        //}	
   	}
    
    auto [avg, std_dev] = s->size_stat();
    printf("%ld graph of size %Lf±%Lf at the last iteration\n", s->graphs().size(), avg, std_dev);
}
