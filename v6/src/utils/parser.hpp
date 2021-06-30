#include <cxxopts.hpp>
#include "../state.hpp"
#include "../rules.hpp"
#include <ctime>

std::tuple<state_t, rule_t*, unsigned int,  unsigned int, int, bool> test_parser(
    cxxopts::Options &options, int argc, char* argv[]) {

    options.add_options() ("h,help", "Print help")
        ("r,rule", "dynamic's rule", cxxopts::value<std::string>()->default_value("erase_create"))

        ("num-graph-print", "maximum number of graphs to print", cxxopts::value<int>()->default_value("20"))

        ("i,injectivity", "revrse iteration after normal iterations for injectivity test")

        ("N,normalize", "normalize after each iteration")

        ("v,verbose", "debuging level (float)", cxxopts::value<float>()->default_value("0"))

        ("n,n-iter", "number of iteration", cxxopts::value<unsigned int>()->default_value("3"))
        ("R,n-reversed-iter", "number of reversed iteration", cxxopts::value<unsigned int>()->default_value("0"))

        ("g,n-graphs", "maximum number of graphs", cxxopts::value<int>()->default_value("-1"))
        ("T,tol", "probability tolerance", cxxopts::value<PROBA_TYPE>()->default_value("0"))
        ("P,precision", "number of bits of precision", cxxopts::value<unsigned int>()->default_value("128"))

        ("t,teta", "teta for the rule (as a multiple of pi)", cxxopts::value<PROBA_TYPE>()->default_value("0.25"))
        ("p,phi", "phi for the rule (as a multiple of pi)", cxxopts::value<PROBA_TYPE>()->default_value("0"))

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
    unsigned int n_reversed_iteration = result["n-reversed-iter"].as<unsigned int>();
    if (result.count("injectivity"))
        n_reversed_iteration = n_iter;
    unsigned int max_n_graphs = result["n-graphs"].as<int>();

    // ------------------------------------------
    // global variables

    tolerance = result["tol"].as<PROBA_TYPE>();
    verbose = result["verbose"].as<float>();
    max_num_graph_print = result["num-graph-print"].as<int>();

    // ------------------------------------------
    // initialize state

    unsigned int size = result["size"].as<unsigned int>();
    state_t state(size);

    if (!result.count("zero"))
        state.randomize();

    PROBA_TYPE const teta_pi = M_PI*result["teta"].as<PROBA_TYPE>();
    PROBA_TYPE const phi_pi = M_PI*result["phi"].as<PROBA_TYPE>();

    // ------------------------------------------
    // read rule

    rule_t* rule;
    std::string rule_ = result["rule"].as<std::string>();
    if (rule_ == "split_merge") {
        rule = new split_merge_rule(teta_pi, phi_pi);
    } else if (rule_ == "erase_create") {
        rule = new erase_create_rule(teta_pi, phi_pi);
    } else if (rule_ == "coin") {
        rule = new coin_rule(teta_pi, phi_pi);
    } else
        throw;
    return {state, rule, n_iter, n_reversed_iteration, max_n_graphs, result.count("normalize")};
}

std::tuple<state_t,
    rule_t*, unsigned int, bool,
    rule_t*, unsigned int, bool,
    unsigned int, int, std::string, bool> iteration_parser(
    cxxopts::Options &options, int argc, char* argv[]) {

    options.add_options() ("h,help", "Print help")
        ("r,rule", "dynamic's rule", cxxopts::value<std::string>()->default_value("erase_create"))
        ("rule2", "dynamic's rule", cxxopts::value<std::string>()->default_value(""))

        ("N,normalize", "normalize after each iteration")

        ("n,n-iter", "number of iteration", cxxopts::value<unsigned int>()->default_value("3"))

        ("g,n-graphs", "maximum number of graphs", cxxopts::value<int>()->default_value("-1"))
        ("T,tol", "probability tolerance", cxxopts::value<PROBA_TYPE>()->default_value("0"))
        ("P,precision", "number of bits of precision", cxxopts::value<unsigned int>()->default_value("128"))

        ("t,teta", "teta for the rule (as a multiple of pi)", cxxopts::value<PROBA_TYPE>()->default_value("0.25"))
        ("p,phi", "phi for the rule (as a multiple of pi)", cxxopts::value<PROBA_TYPE>()->default_value("0"))

        ("different-params", "allow setting different parameters for the second rule", cxxopts::value<bool>())

        ("teta2", "teta for the second rule (as a multiple of pi)", cxxopts::value<PROBA_TYPE>()->default_value("0.25"))
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

    // ------------------------------------------
    // parameters

    unsigned int n_iter = result["n-iter"].as<unsigned int>();
    unsigned int max_n_graphs = result["n-graphs"].as<int>();

    // ------------------------------------------
    // global variables

    tolerance = result["tol"].as<PROBA_TYPE>();

    // ------------------------------------------
    // initialize state

    unsigned int size = result["size"].as<unsigned int>();
    state_t state(size);

    if (!result.count("zero"))
        state.randomize();

    PROBA_TYPE const teta_pi = M_PI*result["teta"].as<PROBA_TYPE>();
    PROBA_TYPE const phi_pi = M_PI*result["phi"].as<PROBA_TYPE>();

    PROBA_TYPE teta2_pi = teta_pi;
    PROBA_TYPE phi2_pi = phi_pi;

    if (result.count("different-params")) {
        teta2_pi = M_PI*result["teta2"].as<PROBA_TYPE>();
        phi2_pi = M_PI*result["phi2"].as<PROBA_TYPE>();
    }

    // ------------------------------------------
    // read rule

    rule_t* rule;
    bool move_first = !result.count("no-move-1");
    unsigned int n_iter_1 = move_first ? result["niter-1"].as<unsigned int>() : 1;

    std::string rule_ = result["rule"].as<std::string>();
    if (rule_ == "split_merge") {
        rule = new split_merge_rule(teta_pi, phi_pi);
    } else if (rule_ == "erase_create") {
        rule = new erase_create_rule(teta_pi, phi_pi);
    } else if (rule_ == "coin") {
        rule = new coin_rule(teta_pi, phi_pi);
    } else
        throw;

    if (move_first)
        rule_ += "_move";

    if (n_iter_1 > 1)
        rule_ += "_" + std::to_string(n_iter_1);

    // ------------------------------------------
    // read second rule

    rule_t* rule2;
    bool move_second = false;
    unsigned int n_iter_2 = 0;

    if (result.count("rule2")) {
        move_second = !result.count("no-move-2");
        n_iter_2 = move_second ? result["niter-2"].as<unsigned int>() : 1;

        std::string rule2_ = result["rule2"].as<std::string>();
        if (rule2_ == "split_merge") {
            rule2 = new split_merge_rule(teta2_pi, phi2_pi);
        } else if (rule2_ == "erase_create") {
            rule2 = new erase_create_rule(teta2_pi, phi2_pi);
        } else if (rule2_ == "coin") {
            rule2 = new coin_rule(teta2_pi, phi2_pi);
        } else
            throw;

        rule_ += "_" + rule2_;

        if (move_second)
            rule_ += "_move";

        if (n_iter_2 > 1)
            rule_ += "_" + std::to_string(n_iter_2);
    }

    return {state,
        rule, n_iter_1, move_first,
        rule2, n_iter_2, move_second,
        n_iter, max_n_graphs, rule_, result.count("normalize")};
}