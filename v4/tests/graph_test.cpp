#include "../classical/rules.hpp"
#include "../classical/graph.hpp"
#include <vector>
#include <stdio.h>

int main() {
  printf("A graphs is inputed by a series of ndoes, having particules (`>` for right and `<` for left) or not.\n");
  printf("With nodes being separated by a '-'.\n");
  printf("you can also input 'r' to create a random graph of size 6\n");
  printf("For exemple the graph:\n    >-<>--<\nis read as:\n    -| |0|>|--|<|1|>|--| |2| |--|<|3| |-\n");

  while (true) {
    printf("\n----------------------------------------------------\n\nPleas enter next graph (Ctrl+C to exit):\n");

    std::vector<unsigned int> left, right;
    graph_t* g_1;
    int n = 0;

    while (true) {
      switch (std::getchar()) {
        case '<':
          left.push_back(n);
          break;

        case '>':
          right.push_back(n);
          break;

        case '-':
          ++n;
          break;

        case '\n':
          ++n;
          goto read;

        case 'r':
          while(std::getchar() != '\n') { }
          goto random;

        default:
          return -1;
      }
    }

read:
    g_1 = new graph(n, left, right);
    printf("\nread from input*: "); g_1->print();
    goto loop;

random:
    g_1 = new graph(6);
    printf("new graph(6):     "); g_1->print();

    g_1->randomize();
    printf("\nrandomize()*:     "); g_1->print();

loop:
    graph_t* g_2 = g_1->copy();

    auto split_merge = get_split_merge(g_1);
    printf("\n"); print_split_merge(split_merge); printf("\n");

    g_1->split_merge(split_merge);
    printf("\nmerge_split():    "); g_1->print();

    split_merge = get_split_merge(g_1);

    printf("\n"); print_split_merge(split_merge); printf("\n");
    g_1->split_merge(split_merge);
    printf("\nmerge_split():    "); g_1->print();

    printf("\ncopy*:            "); g_2->print();

    if (g_1->equal(g_2)) {
       printf("\ninitial copy equal to previous graph\n");
    } else {
      printf("\ninitial copy not equal to previous graph\n");
    }
  }
}
    