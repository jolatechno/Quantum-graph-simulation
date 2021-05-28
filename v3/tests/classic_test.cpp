#include "../classical/rules.hpp"
#include "../classical/graph.hpp"
#include <stdio.h>

int main() {
  for (int j = 0; j < 7; ++j) {
    graph_t* g = new graph(50);

    g->randomize(20);

    if (!graph_checker(g))
      return -1;

    printf("%lu", g->size());

    for (int i = 0; i < 5000000; ++i) {
      g->step();

      auto split_merge = get_split_merge(g);
      g->split_merge(split_merge);

      printf(",%lu", g->size());
    }
    if (j < 6)
      printf(";");
  }
}