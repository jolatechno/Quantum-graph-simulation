#include "../classical/rules.hpp"
#include "../classical/graph.hpp"
#include <ctime>

int main() {
  std::srand(std::time(nullptr)); // use current time as seed for random generator

  printf("A graphs is inputed by a series of ndoes, having particules (`>` for right and `<` for left) or not.\n");
  printf("With nodes being separated by a '-'.\n");
  printf("you can also input 'r' to create a random graph of size 6\n");
  printf("For exemple the graph:\n    >-<>--<\nis read as:\n    -| |0|>|--|<|1|>|--| |2| |--|<|3| |-\n");

  while (true) {
    printf("\n----------------------------------------------------\n\nPleas enter next graph (Ctrl+C to exit):\n");

    std::vector<char /*bool*/> left, right;
    graph_t g_1;

    bool l = false;
    bool r = false;

    while (true) {
      switch (std::getchar()) {
        case '<':
          l = true;
          break;

        case '>':
          r = true;
          break;

        case '-':
          left.push_back(l);
          right.push_back(r);
          l = false;
          r = false;
          break;

        case '\n':
          left.push_back(l);
          right.push_back(r);
          l = false;
          r = false;
          goto read;

        case 'r':
          while(std::getchar() != '\n') { }
          goto random;

        default:
          return -1;
      }
    }

read:
    g_1 = graph_t(left, right);
    printf("\nread from input*: "); print(g_1);
    goto loop;

random:
    g_1 = graph(6);
    printf("new graph(6):     "); print(g_1);

    g_1.randomize();
    printf("\nrandomize()*:     "); print(g_1);

loop:
    graph_t g_2 = graph_t(g_1);

    //--------------

    g_1.step();
    printf("\n\nstep():           "); print(g_1);

    //---

    g_1.reversed_step();
    printf("\nreversed_step():  "); print(g_1);

    //--------------

    auto split_merge = get_split_merge(g_1);
    printf("\n\n"); print_split_merge(split_merge);

    g_1.split_merge(split_merge);
    printf("\nmerge_split():    "); print(g_1);

    //---

    split_merge = get_split_merge(g_1);
    printf("\n"); print_split_merge(split_merge);

    g_1.split_merge(split_merge);
    printf("\nmerge_split():    "); print(g_1);

    //--------------

    auto erase_create = get_erase_create(g_1);
    printf("\n\n"); print_erase_create(erase_create);
    
    g_1.erase_create(erase_create);
    printf("\nerase_create():   "); print(g_1);

    //---

    erase_create = get_erase_create(g_1);
    printf("\n"); print_erase_create(erase_create);
    
    g_1.erase_create(erase_create);
    printf("\nerase_create():   "); print(g_1);

    //--------------

    printf("\n\ncopy*:            "); print(g_2);

    printf("\nhashes are: %ld %ld\n", g_1.hash(), g_2.hash());

    if (g_1.hash() == g_2.hash() /*g_1->equal(g_2)*/) {
       printf("\ninitial copy's hash equal to previous graph's hash\n");
    } else {
      printf("\ninitial copy's hash not equal to previous graph's hash\n");
    }
  }
}
    