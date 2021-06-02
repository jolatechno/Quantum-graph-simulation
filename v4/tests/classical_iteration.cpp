#include "../classical/rules.hpp"
#include "../classical/graph.hpp"
#include "../classical/checks.hpp"
#include <stdio.h>
#include <algorithm>    // std::reverse
#include <vector>

const int n_iteration = 1000;
const int n_iter = 20;

int main() {
  std::vector<float> sizes(2*n_iteration + 1, 0);

  for (int j = 0; j < n_iter - 1; ++j) {
    graph_t* g = new graph(50);
    g->randomize(4);

    graph_t* g_0 = g->copy();

    for (int i = n_iteration - 1; i >= 0; --i) {
      split_merge(g);
      g->reversed_step();
      sizes[i] += g->size() / n_iter;
    }

    sizes[n_iteration] += g_0->size() / n_iter;

    for (int i = n_iteration + 1; i < 2*n_iteration + 1; ++i) {
      g_0->step();
      split_merge(g_0);
      sizes[i] += g_0->size() / n_iter;
    }
  }

  printf("%f", sizes[0]);
    for (auto &s : sizes)
      printf(",%f", s);
}