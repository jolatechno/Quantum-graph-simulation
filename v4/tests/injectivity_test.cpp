#include "../classical/rules.hpp"
#include "../classical/graph.hpp"
#include "../classical/checks.hpp"
#include <stdio.h>

int main() {
  graph_t* g = new graph(20);

  for (int i = 0; i < 100000; ++i) {
    g->randomize();
    graph_t* c = g->copy();

    auto split_merge = get_split_merge(g);
    g->split_merge(split_merge);

    split_merge = get_split_merge(g);
    g->split_merge(split_merge);

    if (g->hash() != c->hash() /*!g->equal(c)*/){
      c->print(); printf("\n");
      g->print(); printf("\n");
      return -2;
    }

    if (!graph_checker(g))
      return -1;
  }
  printf("Ok\n");
}