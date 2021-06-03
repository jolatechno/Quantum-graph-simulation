#include "../classical/rules.hpp"
#include "../classical/graph.hpp"
#include "../quantum/rules.hpp"
#include "../quantum/state.hpp"
#include "../quantum/checks.hpp"

#define _USE_MATH_DEFINES
#include <cmath>

int main() {
    graph_t* g_1 = new graph({true, false, false, true, true},  {true, false, true, false, true});

    /*graph_t* g_1 = new graph(7);
    g_1->randomize();*/

    print(g_1);

    state_t* s = new state(g_1);
    auto [non_merge, merge] = unitary(M_PI_4, M_PI_2);
    auto rule = /*step_split_merge_all*/split_merge_step_erase_create_all/**/(non_merge, merge);

    for (int i = 1; i < 5; ++i) {

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

        //if (i % 3 == 0) {
            printf("reduce_all() ...\n\n"); s->reduce_all(); printf("\n");
        //}	
   	}

    printf("%ld graph", s->graphs().size());
    #ifdef VERBOSE
        printf(" of size ");
        size_stat(s);
    #endif
    printf(" at the last iteration\n");
}
