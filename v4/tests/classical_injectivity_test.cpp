#include "../classical/rules.hpp"
#include "../classical/graph.hpp"
#include "../classical/checks.hpp"
#include <stdio.h>

int main() {
  for (int j = 0; j < 1000; ++j) {
    
    graph_t* g = new graph(10);

    for (int i = 0; i < 20; ++i) {
      g->randomize();
      graph_t* c = g->copy();

      g->step();
      auto split_merge = get_split_merge(g);
      g->split_merge(split_merge);

      graph_t* c2 = g->copy();

      split_merge = get_split_merge(c2);
      c2->split_merge(split_merge);
      c2->reversed_step();


      if (c2->hash() != c->hash() /*!g->equal(c)*/){
        c2->print(); printf("\n");
        c->print(); printf("\n");
        return -2;
      }

      if (!graph_checker(g))
        return -1;
    }
  }
    
  printf("Ok\n");
}