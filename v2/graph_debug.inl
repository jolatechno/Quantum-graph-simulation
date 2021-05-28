#include <cstdio>


/*
print graph
*/


void graph::print() {
  int i_left = 0;
  int i_right = 0;

  /* get the number of nodes */
  int n_nodes = num_nodes();

  /* print all nodes */
  for (int i = 0; i < n_nodes; i++) {
    char left_char = ' ';
    char right_char = ' ';

    if (i_left < left.size())
      if (left[i_left] == i) {
        i_left++;
        left_char = '<';
      }

    if (i_right < right.size())
      if (right[i_right] == i) {
        i_right++;
        right_char = '>';
      }

    printf("|%c|%d|%c|", left_char, i, right_char);

    if (i < n_nodes - 1)
      printf("--");
  }

  printf("\n");
}


/*
simply call the graph_name::print() for ptint graph
*/


void graph::print_name() {
  name->print();
}
