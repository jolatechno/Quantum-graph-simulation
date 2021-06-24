#include <cxxopts.hpp>
#include "../state.hpp"
#include "../rules.hpp"
#include <ctime>

std::tuple<unsigned int, void*, void*, unsigned int,  unsigned int, int, std::string, bool> parser(
    cxxopts::Options &options, int argc, char* argv[]) {

    options.add_options() ("h,help", "Print help")
        ("r,rule", "dynamic's rule", cxxopts::value<std::string>()->default_value("erase_create_all"))
        ("rule2", "dynamic's rule", cxxopts::value<std::string>()->default_value(""))

        ("num-graph-print", "maximum number of graphs to print", cxxopts::value<int>()->default_value("20"))

        ("i,injectivity", "revrse iteration after normal iterations for injectivity test")

        ("N,normalize", "normalize after each iteration")

        ("v,verbose", "debuging level (float, default = 0)", cxxopts::value<float>()->default_value("0"))

        ("n,n-iter", "number of iteration (default = 3)", cxxopts::value<unsigned int>()->default_value("3"))
        ("R,n-reversed-iter", "number of reversed iteration (default = 0)", cxxopts::value<unsigned int>()->default_value("0"))

        ("g,n-graphs", "maximum number of graphs", cxxopts::value<int>()->default_value("-1"))
        ("T,tol", "probability tolerance", cxxopts::value<PROBA_TYPE>()->default_value("0"))
        ("P,precision", "number of bits of precision", cxxopts::value<unsigned int>()->default_value("128"))

        ("t,teta", "teta for the rule (as a multiple of pi)", cxxopts::value<PROBA_TYPE>()->default_value("0.25"))
        ("p,phi", "phi for the rule (as a multiple of pi)", cxxopts::value<PROBA_TYPE>()->default_value("0"))

        ("different-params", "allow setting different parameters for the second rule", cxxopts::value<bool>())

        ("teta2", "teta for the second rule (as a multiple of pi)", cxxopts::value<PROBA_TYPE>()->default_value("0.25"))
        ("phi2", "phi for the second rule (as a multiple of pi)", cxxopts::value<PROBA_TYPE>()->default_value("0"))

        ("s,size", "starting size", cxxopts::value<unsigned int>()->default_value("8"))

        ("seed", "random engine seed", cxxopts::value<unsigned>());

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

    void* rule;
    void* rule2;

    std::string rule_ = result["rule"].as<std::string>();
    if (rule_ == "split_merge_all") {
        rule = new split_merge_rule(teta_pi, phi_pi);
    } else if (rule_ == "erase_create_all") {
        rule = new erase_create_rule(teta_pi, phi_pi);
    } else
        throw;

    // ------------------------------------------
    // read second rule

    rule_ = result["rule2"].as<std::string>();
    if (rule_ == "split_merge_all") {
        rule2 = new split_merge_rule(teta2_pi, phi2_pi);
        rule_ += "_";
    } else if (rule_ == "erase_create_all") {
        rule2 = new erase_create_rule(teta2_pi, phi2_pi);
        rule_ += "_";
    } else
        rule2 = rule;

    rule_ = result["rule"].as<std::string>();

    return {size, rule, rule2, n_iter, n_reversed_iteration, max_n_graphs, rule_, result.count("normalize")};
}