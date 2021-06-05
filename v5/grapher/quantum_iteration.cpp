#include "../classical/rules.hpp"
#include "../classical/graph.hpp"
#include "../quantum/rules.hpp"
#include "../quantum/state.hpp"
#include "../quantum/checks.hpp"

#define _USE_MATH_DEFINES
#include <cmath>

#ifndef RULE
    #define RULE "erase_create_step_split_merge_all"; //"step_split_merge_all" //"step_erase_create_all" //"split_merge_step_erase_create_all"
#endif

#ifndef N_ITER
    #define N_ITER 20
#endif

#ifndef MAX_NUM_GRAPHS
    #define MAX_NUM_GRAPHS 20000
#endif

#ifndef TOLERANCE
    #define TOLERANCE 5e-7
#endif

int main() {
    graph_t* g_1 = new graph({true, false, false, true, false},  {true, false, true, false, true});
    
    /*graph_t* g_1 = new graph(SIZE);
    g_1->randomize();*/

    state_t* s = new state(g_1);
    s->tolerance = TOLERANCE;
    auto [non_merge, merge] = unitary(M_PI_4, M_PI_2);

    std::function<tbb::concurrent_vector<std::pair<std::shared_ptr<graph_t>, std::complex<double>>>(std::shared_ptr<graph_t> g)> rule;

    auto rule_ = RULE;
    if (rule_ == "step_split_merge_all") {
        rule = step_split_merge_all(non_merge, merge);
    } else if (rule_ == "step_erase_create_all") {
        rule = step_erase_create_all(non_merge, merge);
    } else if (rule_ == "split_merge_step_erase_create_all") {
        rule = split_merge_step_erase_create_all(non_merge, merge);
    } else if (rule_ == "erase_create_step_split_merge_all") {
        rule = erase_create_step_split_merge_all(non_merge, merge);
    } else
        return -1;

    start_json(g_1, rule_);
    
    for (int i = 1; i < N_ITER + 1; ++i) {
        s->step_all(rule);
        s->reduce_all();
        s->discard_all(MAX_NUM_GRAPHS);

        serialize_state_to_json(s);
        
        s->normalize();
   	}

    end_json();

}
