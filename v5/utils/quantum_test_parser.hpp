#include <cxxopts.hpp>
#include "../classical/graph.hpp"
#include "../quantum/state.hpp"
#include "../quantum/rules.hpp"
#include <ctime>

std::tuple<state_t::rule_t, state_t::rule_t, std::string, std::string, unsigned int, unsigned int, int, bool> parse_test_quantum(
    cxxopts::Options &options, int argc, char* argv[]) {

    options.add_options() ("h,help", "Print help")
        ("r,rule", "dynamic's rule", cxxopts::value<std::string>()->default_value("step_split_merge_all"))

        ("t,teta", "teta for the rule (as a multiple of pi)", cxxopts::value<PROBA_TYPE>()->default_value("0.25"))
        ("p,phi", "phi for the rule (as a multiple of pi)", cxxopts::value<PROBA_TYPE>()->default_value("0"))
       
        ("N,normalize", "normalize after each iteration")

        ("slow-compare", "slower but closer to exact comparaisons")

        ("v,verbose", "show debugging informations")

        ("n,n-iter", "number of iteration", cxxopts::value<unsigned int>()->default_value("10"))

        ("g,n-graphs", "maximum number of graphs", cxxopts::value<int>()->default_value("-1"))
        ("T,tol", "probability tolerance", cxxopts::value<PROBA_TYPE>()->default_value("0"))
        ("P,precision", "number of bits of precision", cxxopts::value<unsigned int>()->default_value("128"))

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
    unsigned int max_n_graphs = result["n-graphs"].as<int>();

    // ------------------------------------------
    // initialize state

    unsigned int size = result["size"].as<unsigned int>();
    tolerance = result["tol"].as<PROBA_TYPE>();
    slow_comparisons = result.count("slow-compare");
    verbose = result.count("verbose");

    PROBA_TYPE const teta_pi = result["teta"].as<PROBA_TYPE>();
    PROBA_TYPE const phi_pi = result["phi"].as<PROBA_TYPE>();
    auto const [do_not, do_] = unitary(teta_pi, phi_pi);

    // ------------------------------------------
    // rules

    state_t::rule_t rule, reversed_rule;
    std::string reversed_rule_;

    std::string rule_ = result["rule"].as<std::string>();
    if (rule_ == "step_split_merge_all") {
        rule = step_split_merge_all(do_not, do_);
        reversed_rule = reversed_step_split_merge_all(do_not, do_);

        reversed_rule_ = "reversed_step_split_merge_all";

    } else if (rule_ == "step_erase_create_all") {
        rule = step_erase_create_all(do_not, do_);
        reversed_rule = reversed_step_erase_create_all(do_not, do_);

        reversed_rule_ = "reversed_step_erase_create_all";

    } else if (rule_ == "split_merge_all") {
        rule = split_merge_all(do_not, do_);
        reversed_rule = split_merge_all(do_not, do_);

        reversed_rule_ = rule_;

    } else if (rule_ == "erase_create_all") {
        rule = erase_create_all(do_not, do_);
        reversed_rule = erase_create_all(do_not, do_);

        reversed_rule_ = rule_;

    } else
        throw;

    return {rule, reversed_rule, rule_, reversed_rule_, n_iter, max_n_graphs, size, result.count("normalize")};
}