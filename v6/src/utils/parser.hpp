#include <cxxopts.hpp>
#include "../state.hpp"
#include "../rules.hpp"
#include <ctime>

std::tuple<state_t,
    state_t::rule_t*, state_t::rule_t*, 
    unsigned int, int, unsigned int> iteration_parser(
    cxxopts::Options &options, int argc, char* argv[]) {

    options.add_options() ("h,help", "Print help")
        ("r,rule", "dynamic's rule", cxxopts::value<std::string>()->default_value("erase_create"))
        ("rule2", "dynamic's rule", cxxopts::value<std::string>()->default_value(""))

        ("n-graph", "number of random graphs to start with", cxxopts::value<unsigned int>()->default_value("1"))

        ("n-fast", "number of fast iteration in-between normal iterations", cxxopts::value<unsigned int>()->default_value("0"))
        ("start-serializing", "iteration for which to start serializing", cxxopts::value<unsigned int>()->default_value("0"))

        ("n,n-iter", "number of iteration", cxxopts::value<unsigned int>()->default_value("3"))

        ("safety-margin", "safety margin on the maximum number of graphs", cxxopts::value<float>()->default_value("0.2"))
        ("T,tol", "probability tolerance", cxxopts::value<PROBA_TYPE>()->default_value("0"))
        ("P,precision", "number of bits of precision", cxxopts::value<unsigned int>()->default_value("128"))

        ("t,theta", "theta for the rule (as a multiple of pi)", cxxopts::value<PROBA_TYPE>()->default_value("0.25"))
        ("p,phi", "phi for the rule (as a multiple of pi)", cxxopts::value<PROBA_TYPE>()->default_value("0"))

        ("different-params", "allow setting different parameters for the second rule", cxxopts::value<bool>())

        ("theta2", "theta for the second rule (as a multiple of pi)", cxxopts::value<PROBA_TYPE>()->default_value("0.25"))
        ("phi2", "phi for the second rule (as a multiple of pi)", cxxopts::value<PROBA_TYPE>()->default_value("0"))

        ("niter-1", "number of application of the first rule per iteration", cxxopts::value<unsigned int>()->default_value("1"))
        ("niter-2", "number of application of the second rule per iteration", cxxopts::value<unsigned int>()->default_value("1"))

        ("no-move-1", "remove the move after the first rule (and set niter-1 to 1)")
        ("no-move-2", "remove the move after the second rule (and set niter-2 to 1)")

        ("s,size", "starting size", cxxopts::value<unsigned int>()->default_value("8"))

        ("seed", "random engine seed", cxxopts::value<unsigned>())
        ("z,zero", "start with and empty state");

    // parse
    auto result = options.parse(argc, argv);

    if (result.count("help")) {
      std::cout << options.help() << std::endl;
      exit(0);
    }

    // ------------------------------------------
    // use current time as seed for random generator

    if (result.count("seed")) {
        std::srand(result["seed"].as<unsigned>()); 
    } else
        std::srand(std::time(0));

    // ------------------------------------------
    // set precision

    SET_PRECISION(result["precision"].as<unsigned int>())
    safety_margin = result["safety-margin"].as<float>();

    // ------------------------------------------
    // parameters

    unsigned int n_iter = result["n-iter"].as<unsigned int>();

    // ------------------------------------------
    // global variables

    if (result.count("tol")) tolerance = result["tol"].as<PROBA_TYPE>();

    // ------------------------------------------
    // initialize state

    unsigned int size = result["size"].as<unsigned int>();
    unsigned int num_graph = result["n-graph"].as<unsigned int>();
    state_t state(size, num_graph);

    if (!result.count("zero"))
        state.randomize();

    PROBA_TYPE const theta_pi = M_PI*result["theta"].as<PROBA_TYPE>();
    PROBA_TYPE const phi_pi = M_PI*result["phi"].as<PROBA_TYPE>();

    PROBA_TYPE theta2_pi = theta_pi;
    PROBA_TYPE phi2_pi = phi_pi;

    if (result.count("different-params")) {
        theta2_pi = M_PI*result["theta2"].as<PROBA_TYPE>();
        phi2_pi = M_PI*result["phi2"].as<PROBA_TYPE>();
    }

    // ------------------------------------------
    // read rule

    state_t::rule_t* rule = new state_t::rule_t();

    std::string rule_ = result["rule"].as<std::string>();
    if (rule_ == "split_merge") {
        rule = new split_merge_rule(theta_pi, phi_pi, 0);
    } else if (rule_ == "erase_create") {
        rule = new erase_create_rule(theta_pi, phi_pi, 0);
    } else if (rule_ == "coin") {
        rule = new coin_rule(theta_pi, phi_pi, 0);
    } else
        throw;

    rule->move = !result.count("no-move-1");
    rule->n_iter = rule->move ? result["niter-1"].as<unsigned int>() : 1;

    // ------------------------------------------
    // read second rule

    state_t::rule_t* rule2 = new state_t::rule_t();

    if (result.count("rule2")) {

        rule_ = result["rule2"].as<std::string>();
        if (rule_ == "split_merge") {
            rule2 = new split_merge_rule(theta2_pi, phi2_pi, 0);
        } else if (rule_ == "erase_create") {
            rule2 = new erase_create_rule(theta2_pi, phi2_pi, 0);
        } else if (rule_ == "coin") {
            rule2 = new coin_rule(theta2_pi, phi2_pi, 0);
        } else
            throw;

        rule2->move = !result.count("no-move-2");
        rule2->n_iter = rule2->move ? result["niter-2"].as<unsigned int>() : 1;
    }

    return {state,
        rule, rule2, 
        n_iter, result["start-serializing"].as<unsigned int>(), result["n-fast"].as<unsigned int>()};
}

std::tuple<state_t,
    state_t::rule_t*, state_t::rule_t*, 
    unsigned int, int> probabilist_parser(
    cxxopts::Options &options, int argc, char* argv[]) {

    options.add_options() ("h,help", "Print help")
        ("r,rule", "dynamic's rule", cxxopts::value<std::string>()->default_value("erase_create"))
        ("rule2", "dynamic's rule", cxxopts::value<std::string>()->default_value(""))

        ("N,n-graph", "number of random graphs to start with", cxxopts::value<unsigned int>()->default_value("3"))
        ("start-serializing", "iteration for which to start serializing", cxxopts::value<unsigned int>()->default_value("0"))

        ("n,n-iter", "number of iteration", cxxopts::value<unsigned int>()->default_value("3"))
        
        ("P,precision", "number of bits of precision", cxxopts::value<unsigned int>()->default_value("128"))

        ("q", "q for the rule (useless for \"coin\" rule)", cxxopts::value<PROBA_TYPE>()->default_value("0.5"))
        ("p", "p for the rule", cxxopts::value<PROBA_TYPE>()->default_value("0.5"))

        ("different-params", "allow setting different parameters for the second rule", cxxopts::value<bool>())

        ("q2", "q for the second rule  (useless for \"coin\" rule)", cxxopts::value<PROBA_TYPE>()->default_value("0.5"))
        ("p2", "p for the second rule", cxxopts::value<PROBA_TYPE>()->default_value("0.5"))

        ("niter-1", "number of application of the first rule per iteration", cxxopts::value<unsigned int>()->default_value("1"))
        ("niter-2", "number of application of the second rule per iteration", cxxopts::value<unsigned int>()->default_value("1"))

        ("no-move-1", "remove the move after the first rule (and set niter-1 to 1)")
        ("no-move-2", "remove the move after the second rule (and set niter-2 to 1)")

        ("s,size", "starting size", cxxopts::value<unsigned int>()->default_value("8"))

        ("seed", "random engine seed", cxxopts::value<unsigned>())
        ("z,zero", "start with and empty state");

    // parse
    auto result = options.parse(argc, argv);

    if (result.count("help")) {
      std::cout << options.help() << std::endl;
      exit(0);
    }

    // ------------------------------------------
    // use current time as seed for random generator

    if (result.count("seed")) {
        std::srand(result["seed"].as<unsigned>()); 
    } else
        std::srand(std::time(0));

    // ------------------------------------------
    // set precision

    SET_PRECISION(result["precision"].as<unsigned int>())

    // ------------------------------------------
    // parameters

    unsigned int n_iter = result["n-iter"].as<unsigned int>();

    // ------------------------------------------
    // initialize state

    unsigned int size = result["size"].as<unsigned int>();
    unsigned int num_graph = result["n-graph"].as<unsigned int>();
    state_t state(size, num_graph);

    if (!result.count("zero"))
        state.randomize();

    PROBA_TYPE const p = result["p"].as<PROBA_TYPE>();
    PROBA_TYPE const q = result["q"].as<PROBA_TYPE>();

    PROBA_TYPE p2 = p;
    PROBA_TYPE q2 = q;

    if (result.count("different-params")) {
        p2 = result["p2"].as<PROBA_TYPE>();
        q2 = result["q2"].as<PROBA_TYPE>();
    }

    // ------------------------------------------
    // read rule

    state_t::rule_t* rule = new state_t::rule_t();

    std::string rule_ = result["rule"].as<std::string>();
    if (rule_ == "split_merge") {
        rule = new split_merge_rule(p, q);
    } else if (rule_ == "erase_create") {
        rule = new erase_create_rule(p, q);
    } else if (rule_ == "coin") {
        rule = new coin_rule(p);
    } else
        throw;

    rule->move = !result.count("no-move-1");
    rule->n_iter = rule->move ? result["niter-1"].as<unsigned int>() : 1;

    // ------------------------------------------
    // read second rule

    state_t::rule_t* rule2 = new state_t::rule_t();

    if (result.count("rule2")) {

        rule_ = result["rule2"].as<std::string>();
        if (rule_ == "split_merge") {
            rule2 = new split_merge_rule(p, q);
        } else if (rule_ == "erase_create") {
            rule2 = new erase_create_rule(p, q);
        } else if (rule_ == "coin") {
            rule2 = new coin_rule(p);
        } else
            throw;

        rule2->move = !result.count("no-move-2");
        rule2->n_iter = rule2->move ? result["niter-2"].as<unsigned int>() : 1;
    }

    return {state,
        rule, rule2, 
        n_iter, result["start-serializing"].as<unsigned int>()};
}

std::tuple<state_t, state_t::rule_t*, state_t::rule_t*, unsigned int, int, unsigned int> test_parser(
    cxxopts::Options &options, int argc, char* argv[]) {

    options.add_options() ("h,help", "Print help")
        ("r,rule", "dynamic's rule", cxxopts::value<std::string>()->default_value("erase_create"))

        ("num-graph-print", "maximum number of graphs to print", cxxopts::value<int>()->default_value("20"))

        ("i,injectivity", "revrse iteration after normal iterations for injectivity test")

        ("n-fast", "number of fast iteration in-between normal iterations", cxxopts::value<unsigned int>()->default_value("0"))

        ("v,verbose", "debuging level (float)", cxxopts::value<float>()->default_value("0"))

        ("n,n-iter", "number of iteration", cxxopts::value<unsigned int>()->default_value("3"))
        ("R,n-reversed-iter", "number of reversed iteration", cxxopts::value<unsigned int>()->default_value("0"))

        ("safety-margin", "safety margin on the maximum number of graphs", cxxopts::value<float>()->default_value("0.2"))
        ("T,tol", "probability tolerance", cxxopts::value<PROBA_TYPE>()->default_value("0"))
        ("P,precision", "number of bits of precision", cxxopts::value<unsigned int>()->default_value("128"))

        ("t,theta", "theta for the rule (as a multiple of pi)", cxxopts::value<PROBA_TYPE>()->default_value("0.25"))
        ("p,phi", "phi for the rule (as a multiple of pi)", cxxopts::value<PROBA_TYPE>()->default_value("0"))
        ("x,xi", "xi for the rule (as a multiple of pi)", cxxopts::value<PROBA_TYPE>()->default_value("0"))

        ("s,size", "starting size", cxxopts::value<unsigned int>()->default_value("8"))

        ("seed", "random engine seed", cxxopts::value<unsigned>())
        ("z,zero", "start with and empty state");

    // parse
    auto result = options.parse(argc, argv);

    if (result.count("help")) {
      std::cout << options.help() << std::endl;
      exit(0);
    }

    // ------------------------------------------
    // use current time as seed for random generator

    if (result.count("seed")) {
        std::srand(result["seed"].as<unsigned>()); 
    } else
        std::srand(std::time(0));

    // ------------------------------------------
    // set precision

    SET_PRECISION(result["precision"].as<unsigned int>())
    safety_margin = result["safety-margin"].as<float>();

    // ------------------------------------------
    // parameters

    unsigned int n_iter = result["n-iter"].as<unsigned int>();
    unsigned int n_reversed_iteration = result["n-reversed-iter"].as<unsigned int>();
    if (result.count("injectivity"))
        n_reversed_iteration = n_iter;

    // ------------------------------------------
    // global variables

    if (result.count("tol")) tolerance = result["tol"].as<PROBA_TYPE>();
    if (result.count("verbose")) verbose = result["verbose"].as<float>();
    if (result.count("num-graph-print")) max_num_graph_print = result["num-graph-print"].as<int>();

    // ------------------------------------------
    // initialize state

    unsigned int size = result["size"].as<unsigned int>();
    state_t state(size);

    if (!result.count("zero"))
        state.randomize();

    PROBA_TYPE const theta_pi = M_PI*result["theta"].as<PROBA_TYPE>();
    PROBA_TYPE const phi_pi = M_PI*result["phi"].as<PROBA_TYPE>();
    PROBA_TYPE const xi_pi = M_PI*result["xi"].as<PROBA_TYPE>();

    // ------------------------------------------
    // read rule

    state_t::rule_t *rule, *reversed_rule;

    std::string rule_ = result["rule"].as<std::string>();
    if (rule_ == "split_merge") {
        rule = new split_merge_rule(theta_pi, phi_pi, xi_pi);
        reversed_rule = new split_merge_rule(theta_pi, phi_pi, -xi_pi);
    } else if (rule_ == "erase_create") {
        rule = new erase_create_rule(theta_pi, phi_pi, xi_pi);
        reversed_rule = new erase_create_rule(theta_pi, phi_pi, -xi_pi);
    } else if (rule_ == "coin") {
        rule = new coin_rule(theta_pi, phi_pi, xi_pi);
        reversed_rule = new coin_rule(theta_pi, phi_pi, -xi_pi);
    } else
        throw;
    return {state, rule, reversed_rule, n_iter, n_reversed_iteration, result["n-fast"].as<unsigned int>()};
}