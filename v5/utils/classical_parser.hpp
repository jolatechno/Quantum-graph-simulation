#include <cxxopts.hpp>
#include "../classical/rules.hpp"
#include <ctime>

std::tuple<std::function<void(graph_t &g)>, std::string, unsigned int, unsigned int> parse_classical(
    cxxopts::Options &options, int argc, char* argv[]) {

    options.add_options() ("h,help", "Print help")
        ("r,rule", "dynamic's rule", cxxopts::value<std::string>()->default_value("step_split_merge_all"))

        ("n,n-iter", "number of iteration", cxxopts::value<unsigned int>()->default_value("1000"))

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
    // parameters

    unsigned int n_iter = result["n-iter"].as<unsigned int>();

    // ------------------------------------------
    // initialize state

    unsigned int size = result["size"].as<unsigned int>();

    // ------------------------------------------
    // read rule

    std::function<void(graph_t &g)> rule;

    std::string rule_ = result["rule"].as<std::string>();
    if (rule_ == "step_split_merge_all") {
        rule = [](graph_t &g) {
            g.step();
            split_merge(g);
        };

    } else if (rule_ == "step_erase_create_all") {
        rule = [](graph_t &g) {
            g.step();
            erase_create(g);
        };

    } else if (rule_ == "reversed_step_erase_create_all") {
        rule = [](graph_t &g) {
            erase_create(g);
            g.reversed_step();
        };

    } else if (rule_ == "reverse_step_erase_create_all") {
        rule = [](graph_t &g) {
            erase_create(g);
            g.reversed_step();
        };

    } else if (rule_ == "split_merge_all") {
        rule = split_merge;

    } else if (rule_ == "erase_create_all") {
        rule = erase_create;

    } else
        throw;

    return {rule, rule_, n_iter, size};
}