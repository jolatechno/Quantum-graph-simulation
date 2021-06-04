#include "../classical/graph_name.hpp"

int main() {
  graph_name_t n_1 = graph_name(6);
  printf("*new graph_name(6): "); print(n_1);

  graph_name_t n_2 = n_1;

  n_1.merge(5);
  printf("\nmerge 5:            "); print(n_1);

  if (n_1.hash() == n_2.hash()) {
     printf("initial copy's* hash equal to previous graph's hash\n");
  } else {
    printf("initial copy's* hash not equal to previous graph's hash\n");
  }

  n_1.split(3);
  printf("\nsplit 3:            "); print(n_1);

  n_1.split(0);
  printf("\nsplit 0:            "); print(n_1);

  n_1.merge(3);
  printf("\nmerge 3:            "); print(n_1);

  printf("\ncopy*:              "); print(n_2);

  printf("\nhashes are: %ld %ld\n", n_1.hash(), n_2.hash());

  if (n_1.hash() == n_2.hash()) {
     printf("initial copy's* hash equal to previous graph's hash\n");
  } else {
    printf("initial copy's* hash not equal to previous graph's hash\n");
  }
}
