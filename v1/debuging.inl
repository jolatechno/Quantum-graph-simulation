#include <cstdio> //for printf

void print_list(int *list, int size) {
  for (int i = 0; i < size; i++)
    printf("%d ", list[i]);
  printf("\n");
}

void print_state(state *s) {
  printf("%s ", s->color ? "true" : "false");
  char *clockwise = (char*)malloc(s->n_nodes*sizeof(char));
  char *counter_clockwise = (char*)malloc(s->n_nodes*sizeof(char));

  for (int i = 0; i < s->n_nodes; i++) {
    clockwise[i] = ' ';
    counter_clockwise[i] = ' ';
  }

  int i = 0;
  for (; i < s->n_clockwise; i++)
    clockwise[s->particules[i]] = '>';

  for (; i < s->n_particules; i++)
    counter_clockwise[s->particules[i]] = '<';

  for (int j = 0; j < s->n_nodes; j++)
    printf("-|%c|%d|%c|-", counter_clockwise[j], j, clockwise[j]);
  printf("\n");
}
