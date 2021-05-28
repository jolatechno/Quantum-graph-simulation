#include <cstdio>

void graph_name::print() {
  for (int j = 0; j < nodes.size(); j++) {
    nodes[j]->print();
    if (j < nodes.size() - 1)
      printf(" --");
  }
  printf("\n");
}
