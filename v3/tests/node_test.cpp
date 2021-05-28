#include "../classical/node.hpp"
#include <stdio.h>

int main() {
  node_t* n_1 = new node(1);
  printf("new node(1): "); n_1->print();

  node_t* n_1_l = n_1->left();
  printf("\n.l:          "); n_1_l->print();

  node_t* n_1_r = n_1->right();
  printf("\n.r:          "); n_1_r->print();

  node_t* n_1_r_l = new node(n_1_r, n_1_l);
  printf("\n.r∧.l:       "); n_1_r_l->print();

  node_t* n_1_l_r = new node(n_1_l, n_1_r);
  printf("\n.l∧.r:       "); n_1_l_r->print();

  if (n_1->equal(n_1_l_r)) {
    printf("\n1 == 1.l∧1.r");
  } else {
    printf("\n1 != 1.l∧1.r");
  }

  printf("\ndo pointer match (for 1 and 1.l∧1.r): %p != %p\n", n_1_l_r, n_1);
}
