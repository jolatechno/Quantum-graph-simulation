#include "../classical/rules.hpp"
#include "../classical/graph.hpp"
#include "../quantum/rules.hpp"
#include "../quantum/state.hpp"
#include "../quantum/checks.hpp"
#include "../utils/quantum_parser.hpp"
#include <ctime>

int main(int argc, char* argv[]) {
    // ------------------------------------------
    // making stdout unbuffered so it can get redirected to file even if there is an error
    
    std::setvbuf(stdout, NULL, _IONBF, 0);

    std::srand(std::time(nullptr)); // use current time as seed for random generator

    // ------------------------------------------
    // initialize state

    cxxopts::Options options("iterate quantum case and print out json statistics");
    auto [s, rule, rule2, n_iter, max_n_graphs, rule_, normalize_] = parse_quantum(options, argc, argv);

    // ------------------------------------------
    // iterations

    start_json(s, rule_.c_str());
    
    for (int i = 1; i < n_iter + 1; ++i) {
        if (i%2) {
            s->step_all(rule);
        } else
            s->step_all(rule2);

        s->reduce_all();

        if (max_n_graphs > 0)
            s->discard_all(max_n_graphs);

        serialize_state_to_json(s);
        
        if (normalize_)
            s->normalize();
   	}

    end_json();
}
