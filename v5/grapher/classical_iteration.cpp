#include "../classical/rules.hpp"
#include "../classical/graph.hpp"
#include "../classical/checks.hpp"

const int n_iter = 20;

#ifndef N_ITER
    #define N_ITER 10000
#endif

int main() {
  std::vector<float> sizes(2*N_ITER + 1, 0);

  for (int j = 0; j < n_iter - 1; ++j) {
    graph_t* g = new graph(50);
    g->randomize(4);

    graph_t* g_0 = g->copy();

    for (int i = N_ITER - 1; i >= 0; --i) {
      split_merge(g);
      g->reversed_step();
      sizes[i] += g->size() / n_iter;
    }

    sizes[N_ITER] += g_0->size() / n_iter;

    for (int i = N_ITER + 1; i < 2*N_ITER + 1; ++i) {
      g_0->step();
      split_merge(g_0);
      sizes[i] += g_0->size() / n_iter;
    }
  }

  printf("%f", sizes[0]);
    for (auto &s : sizes)
      printf(",%f", s);
}