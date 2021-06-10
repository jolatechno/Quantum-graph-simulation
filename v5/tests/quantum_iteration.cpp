#include "../classical/rules.hpp"
#include "../classical/graph.hpp"
#include "../quantum/rules.hpp"
#include "../quantum/state.hpp"
#include "../quantum/checks.hpp"
#include "../utils/quantum_test_parser.hpp"

#define _USE_MATH_DEFINES
#include <cmath>

int main(int argc, char* argv[]) {
    cxxopts::Options options("simple quantum iterator");
    auto [rule, _, rule_, __, n_iter, max_n_graphs, size, tol, normalize_] = parse_test_quantum(options, argc, argv);

    graph_t g_1 = graph(size);
    g_1.randomize();

    state_t* s = new state(g_1);
    s->tolerance = tol;

    for (int i = 1; i < n_iter; ++i) {

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

        printf("\n%s() ...\n", rule_.c_str());
        s->step_all(rule);

        printf("reduce_all() ...\n");
        s->reduce_all();
        
        if (max_n_graphs > 0) {
            printf("discard_all() ...\n");
            s->discard_all(max_n_graphs);
        }

        if (normalize_) {
            printf("normalize() ...\n\n");
            s->normalize();
        }

        printf("\n");
   	}

    printf("%ld graph", s->graphs().size());
    #ifdef VERBOSE
        printf(" of size ");
        size_stat(s);
    #endif
    printf(" at the last iteration\n");
}
