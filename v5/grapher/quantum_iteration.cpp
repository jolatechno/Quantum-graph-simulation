#include "../classical/rules.hpp"
#include "../classical/graph.hpp"
#include "../quantum/rules.hpp"
#include "../quantum/state.hpp"
#include "../quantum/checks.hpp"

#define _USE_MATH_DEFINES
#include <cmath>

#define RULE "erase_create_step_split_merge_all"; //"step_split_merge_all" //"step_erase_create_all" //"split_merge_step_erase_create_all"
#define SIZE 5
#define N_ITER 7
#define MAX_NUM_GRAPHS 1000000

int main() {
    auto rule_ = RULE;

    graph_t* g_1 = new graph({true, false, false, true, false},  {true, false, true, false, true});
    
    /*graph_t* g_1 = new graph(SIZE);
    g_1->randomize();*/

    state_t* s = new state(g_1);
    auto [non_merge, merge] = unitary(M_PI_4, M_PI_2);

    std::function<tbb::concurrent_vector<std::pair<graph_t*, std::complex<double>>>(graph_t* g)> rule;

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
        s->discard_graphs(MAX_NUM_GRAPHS);

        serialize_state_to_json(s);
   	}

    end_json();

}
