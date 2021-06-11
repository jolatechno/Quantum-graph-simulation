#include <cxxopts.hpp>
#include "../quantum/state.hpp"
#include "../quantum/rules.hpp"
#include <ctime>

std::tuple<state_t *, state_t::rule_t, state_t::rule_t, unsigned int, int, std::string, bool> parse_quantum(
    cxxopts::Options &options, int argc, char* argv[]) {

    options.add_options() ("h,help", "Print help")
        ("r,rule", "dynamic's rule", cxxopts::value<std::string>()->default_value("step_split_merge_all"))
        ("rule2", "dynamic's rule", cxxopts::value<std::string>()->default_value(""))

        ("N,normalize", "normalize after each iteration")

        ("slow-compare", "slower but closer to exact comparaisons")

        ("v,verbose", "show debugging informations")

        ("n,n-iter", "number of iteration", cxxopts::value<unsigned int>()->default_value("10"))

        ("g,n-graphs", "maximum number of graphs", cxxopts::value<int>()->default_value("-1"))
        ("T,tol", "probability tolerance", cxxopts::value<PROBA_TYPE>()->default_value("0"))
        ("P,precision", "number of bits of precision", cxxopts::value<unsigned int>()->default_value("128"))

        ("t,teta", "teta for the rule (as a multiple of pi)", cxxopts::value<PROBA_TYPE>()->default_value("0.25"))
        ("p,phi", "phi for the rule (as a multiple of pi)", cxxopts::value<PROBA_TYPE>()->default_value("0"))

        ("different-params", "allow setting different parameters for the second rule", cxxopts::value<bool>())

        ("teta2", "teta for the second rule (as a multiple of pi)", cxxopts::value<PROBA_TYPE>()->default_value("0.25"))
        ("phi2", "phi for the second rule (as a multiple of pi)", cxxopts::value<PROBA_TYPE>()->default_value("0"))

        ("s,size", "starting size", cxxopts::value<unsigned int>()->default_value("8"))

        ("zero-randomize", "randomize initial state with empty graphs", cxxopts::value<bool>()->default_value("false"))
        ("randomize", "randomize initial state", cxxopts::value<bool>()->default_value("false"))

        ("delta-size", "delta of sizes in the random starting state", cxxopts::value<unsigned int>()->default_value("5"))
        ("num-graphs", "number of graph for each size in random starting state", cxxopts::value<unsigned int>()->default_value("3"))

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
    unsigned int delta_size = result["delta-size"].as<unsigned int>();
    bool randomize = result["randomize"].as<bool>();
    bool zero_randomize = result["zero-randomize"].as<bool>();
    unsigned int num_graphs = result["num-graphs"].as<unsigned int>();

    state_t* s;

    if (randomize) {
        s = new state();
        s->randomize(size - delta_size/2, size + delta_size/2 + 1, num_graphs);
    } else if (zero_randomize) {
        s = new state();
        s->zero_randomize(size - delta_size/2, size + delta_size/2 + 1);
    } else {
        graph_t g_1 = graph_t(size);
        g_1.randomize();
        s = new state(g_1);
    }

    tolerance = result["tol"].as<PROBA_TYPE>();
   slow_comparisons = result.count("slow-compare");
    verbose = result.count("verbose");

    PROBA_TYPE const teta_pi = result["teta"].as<PROBA_TYPE>();
    PROBA_TYPE const phi_pi = result["phi"].as<PROBA_TYPE>();

    auto const [do_not, do_] = unitary(teta_pi, phi_pi);

    state_t::mag_t do_not_2, do_2;
    if (result.count("different-params")) {
        PROBA_TYPE teta2_pi = M_PI*result["teta2"].as<PROBA_TYPE>();
        PROBA_TYPE phi2_pi = M_PI*result["phi2"].as<PROBA_TYPE>();

        auto [do_not_2_, do_2_] = unitary(teta2_pi, phi2_pi);
        do_not_2 = do_not_2_;
        do_2 = do_2_;
    } else {
        do_not_2 = do_not;
        do_2 = do_;
    }

    // ------------------------------------------
    // read rule

    state_t::rule_t rule, rule2;

    std::string rule_ = result["rule"].as<std::string>();
    if (rule_ == "step_split_merge_all") {
        rule = step_split_merge_all(do_not, do_);
    } else if (rule_ == "step_erase_create_all") {
        rule = step_erase_create_all(do_not, do_);
    } else if (rule_ == "split_merge_all") {
        rule = split_merge_all(do_not, do_);
    } else if (rule_ == "erase_create_all") {
        rule = erase_create_all(do_not, do_);
    } else
        throw;

    // ------------------------------------------
    // read second rule

    rule_ = result["rule2"].as<std::string>();
    if (rule_ == "step_split_merge_all") {
        rule2 = step_split_merge_all(do_not_2, do_2);
        rule_ += "_";
    } else if (rule_ == "step_erase_create_all") {
        rule2 = step_erase_create_all(do_not_2, do_2);
        rule_ += "_";
    } else if (rule_ == "split_merge_all") {
        rule2 = split_merge_all(do_not_2, do_2);
        rule_ += "_";
    } else if (rule_ == "erase_create_all") {
        rule2 = erase_create_all(do_not_2, do_2);
        rule_ += "_";
    } else
        rule2 = rule;

    rule_ = result["rule"].as<std::string>();

    return {s, rule, rule2, n_iter, max_n_graphs, rule_, result.count("normalize")};
}