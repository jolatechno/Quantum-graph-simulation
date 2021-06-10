#include "../classical/rules.hpp"
#include "../classical/graph.hpp"
#include "../classical/checks.hpp"
#include "../utils/classical_parser.hpp"

int main(int argc, char* argv[]) {
  cxxopts::Options options("check for classical injectivity");
  auto [_, __, n_iter, size] = parse_classical(options, argc, argv);

  #pragma omp parallel
  #pragma omp master
  for (int j = 0; j < n_iter; ++j)
  #pragma omp task
  {
    
    graph_t g = graph_t(size);

    for (int i = 0; i < 20; ++i) {
      g.randomize();
      graph_t c = g;

      g.step();
      g.reversed_step();
      split_merge(g);

      graph_t c2 = g;

      split_merge(c2);
      erase_create(c2);
      erase_create(c2);

      if (c.hash() != c2.hash() /*!g->equal(c)*/){
        print(c2); printf("\n");
        print(c); printf("\n");
        throw;
      }

      if (!graph_checker(g))
        throw;
    }
  }
    
  printf("Ok\n");
}