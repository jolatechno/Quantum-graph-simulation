#include "../classical/rules.hpp"
#include "../classical/graph.hpp"
#include "../classical/checks.hpp"

int main() {
  #pragma omp parallel
  #pragma omp single
  for (int j = 0; j < 1000; ++j)
  #pragma omp task
  {
    
    graph_t* g = new graph(10);

    for (int i = 0; i < 20; ++i) {
      g->randomize();
      graph_t* c = new graph_t(*g);

      g->step();
      g->reversed_step();
      split_merge(*g);

      graph_t* c2 = new graph_t(*g);

      split_merge(*c2);
      erase_create(*c2);
      erase_create(*c2);

      if (c->hash() != c2->hash() /*!g->equal(c)*/){
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