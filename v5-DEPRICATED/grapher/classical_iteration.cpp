#include "../classical/rules.hpp"
#include "../classical/graph.hpp"
#include "../classical/checks.hpp"
#include "../utils/classical_parser.hpp"

int main(int argc, char* argv[]) {
  // ------------------------------------------
  // making stdout unbuffered so it can get redirected to file even if there is an error

  std::setvbuf(stdout, NULL, _IONBF, 0);

  cxxopts::Options options("iterate classical case and print out sizes");
  auto [rule, _, n_iter, size] = parse_classical(options, argc, argv);

  graph_t g = graph_t(size);
  g.randomize();

  printf("%ld", g.size());

  for (int i = 0; i < n_iter; ++i) {
    rule(g);

    printf(",%ld", g.size());
  }

  printf("\n");
}