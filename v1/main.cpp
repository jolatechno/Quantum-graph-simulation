/*
compile using:

g++ main.cpp -fopenmp -foffload=nvptx-none -fcf-protection=none -fno-stack-protector
*/

#include <time.h> //for the random seed

#include "class.hpp"

#include "debuging.inl"
#include "rule.inl"
#include "state.inl"
#include "quantum_state.inl"

#define N_RUNS 5
#define N_ITER 5000
#define N_PARTICULES 100
#define N_NODE0 500


int main() {
  srand (time(NULL)); //initialize random seed
  rule *ref = new rule();

  int split_size, *split_indexes = (int*)malloc(N_PARTICULES*sizeof(int));
  int merge_size, *merge_indexes = (int*)malloc(N_PARTICULES*sizeof(int));

  for (int j = 0; j < N_RUNS; j++) {
    state *s = new state(N_PARTICULES);
    s->n_nodes = N_NODE0;
    s->randomize(N_PARTICULES/2);

    for (int i = 0; i < N_ITER; i++) {
      s->step_all();
      ref->get_split(s, split_indexes, &split_size);
      ref->get_merge(s, merge_indexes, &merge_size);
      s->merge_split(split_indexes, split_size, merge_indexes, merge_size);

      printf("%ld", s->n_nodes);
      if (i != N_ITER - 1)
        printf(",");
    }
    if (j != N_RUNS - 1)
      printf(";");
  }

  printf("\n");

  return 0;
}
