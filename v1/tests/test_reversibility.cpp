/*
compile using:

g++ test_reversibility.cpp -fopenmp -foffload=nvptx-none -fcf-protection=none -fno-stack-protector
*/

#include "../class.hpp"

#include "../debuging.inl"
#include "../rule.inl"
#include "../state.inl"
#include "../quantum_state.inl"

#include "reversibility_test.inl"

#define SIZE 4
#define PRINT 64

int main() {
  rule *ref = new rule();

  int num_graphs = number_of_graphs(SIZE);
  state** state_list = list_all_graphs(SIZE);

  for (long int i = 0; i < PRINT; i++) {
    printf("%ld: ", i); print_state(state_list[i]);
  }

  printf("is distinct: %s\n", check_distinct(state_list, num_graphs) ? "true" : "false");

  state **new_state_list = (state**)malloc(num_graphs*sizeof(state*));

  //#pragma omp parallel for
  for (int i = 0; i < num_graphs; i++) {
    state *s = state_list[i]->copy(); //state to consider
    const int n_particules = s->n_particules; //numer of particules

    // list for interactions
    int split_size, *split_indexes = (int*)malloc(n_particules*sizeof(int));
    int merge_size, *merge_indexes = (int*)malloc(n_particules*sizeof(int));

    //apply an iteration
    s->step_all();
    ref->get_split(s, split_indexes, &split_size);
    ref->get_merge(s, merge_indexes, &merge_size);
    s->merge_split(split_indexes, split_size, merge_indexes, merge_size);

    if (!check(s)) {
      print_state(s);
      printf("with %ld particules\n\n", s->n_particules);
    }

    new_state_list[i] = s;
  }

  for (long int i = 0; i < PRINT; i++) {
    printf("%ld: ", i); print_state(new_state_list[i]);
  }

  /*
  debugging information
  */


  int i, j;
  bool distinct = check_distinct(new_state_list, num_graphs, &i, &j);
  printf("is distinct: %s\n", distinct ? "true" : "false");

  if (!distinct) {
    print_state(state_list[i]);
    print_state(state_list[j]);

    printf("with %ld particules \n\n", state_list[i]->n_particules);

    state_list[i]->step_all();
    state_list[j]->step_all();
    print_state(state_list[i]);
    print_state(state_list[j]);

    int split_size, *split_indexes = (int*)malloc(state_list[i]->n_particules*sizeof(int));
    int merge_size, *merge_indexes = (int*)malloc(state_list[i]->n_particules*sizeof(int));

    ref->get_split(state_list[i], split_indexes, &split_size);
    ref->get_merge(state_list[i], merge_indexes, &merge_size);
    printf("\nsplit at: "); print_list(split_indexes, split_size);
    printf("megre at: "); print_list(merge_indexes, split_size);

    ref->get_split(state_list[j], split_indexes, &split_size);
    ref->get_merge(state_list[j], merge_indexes, &merge_size);
    printf("\nsplit at: "); print_list(split_indexes, split_size);
    printf("megre at: "); print_list(merge_indexes, split_size);

    print_state(new_state_list[i]);
    print_state(new_state_list[j]);

    printf("particules at: "); print_list(new_state_list[i]->particules, new_state_list[i]->n_particules);
  }

  return 0;
}
