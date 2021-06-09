#include "../classical/rules.hpp"
#include "../classical/graph.hpp"
#include "../quantum/rules.hpp"
#include "../quantum/state.hpp"
#include "../quantum/checks.hpp"
#include <ctime>

#define _USE_MATH_DEFINES
#include <cmath>

#ifndef N_ITER
    #define N_ITER 20
#endif

#ifndef TETA
    #define TETA M_PI_4
#endif

#ifndef PHI
    #define PHI M_PI_2
#endif

int main() {
    std::srand(std::time(nullptr)); // use current time as seed for random generator

    SET_PRECISION
    
    graph_t g_1 = graph_t({true, false, false, true, true},  {true, false, true, false, true});

    /*graph_t* g_1 = new graph(7);
    g_1->randomize();*/

    state_t* s = new state(g_1);
    auto [non_merge, merge] = unitary(TETA, PHI);
    auto rule = /**/step_split_merge_all/*step_erase_create_all*/(non_merge, merge);

    for (int i = 1; i < N_ITER; ++i) {

        printf("%ld graph", s->graphs().size());
        #ifdef VERBOSE
            printf(" of size ");
            size_stat(s);
        #endif
        printf(" after %d iterations\n", i);

        #ifdef TEST
            printf("checking graphs...\n");
            if (!check(s))
                return -1;
            printf("...OK\n");
        #endif

        printf("\nstep_split_merge_all() ...\n"); s->step_all(rule);

        printf("reduce_all() ...\n\n");
        s->reduce_all();
        //s->discard_all(300);
        printf("\n");
   	}

    printf("%ld graph", s->graphs().size());
    #ifdef VERBOSE
        printf(" of size ");
        size_stat(s);
    #endif
    printf(" at the last iteration\n");
}
