#include <ctime>

#include "node.hpp"

#include "node_public.inl"
#include "node_private.inl"
#include "node_debug.inl"

#include "graph_names.hpp"

#include "graph_names_public.inl"
#include "graph_names_debug.inl"

#include "graph.hpp"

#include "graph_public.inl"
#include "graph_private.inl"
#include "graph_debug.inl"

#include "rules.inl"

void print_vector(std::vector<int> &vect) {
  for (auto it = vect.begin(); it < vect.end(); ++it)
    printf("%d ", *it);
  printf("\n");
}

int main() {
  std::srand(std::time(nullptr));

  /*graph* g = new graph(20);
  g->randomize(5, 5);

  std::vector<int> split_nodes;
  std::vector<int> merge_nodes;

  for (int i = 0; i < 20; i++) {
    g->step_all();

    g->print();

    get_split(g, split_nodes);
    get_merge(g, merge_nodes);
    g->split_merge(split_nodes, merge_nodes);


    print_vector(split_nodes);
    print_vector(merge_nodes);

    g->print();
    g->print_name();
    printf("\n");
  } */

  for (int j = 0; j < 100; j++) {
    graph* g = new graph(20);
    g->randomize(5, 5);

    std::vector<int> split_nodes;
    std::vector<int> merge_nodes;

    for (int i = 0; i < 500; i++) {
      //test
      //-------------------------------
      graph* g2 = g->copy();

      g->print();
      g->print_name();

      printf("\n");

      get_split(g2, split_nodes);
      get_merge(g2, merge_nodes);

      printf("split at "); print_vector(split_nodes);
      printf("merge at "); print_vector(merge_nodes);

      g2->split_merge(split_nodes, merge_nodes);

      g2->print();
      g2->print_name();

      printf("\n");

      get_split(g2, split_nodes);
      get_merge(g2, merge_nodes);
      g2->split_merge(split_nodes, merge_nodes);

      if (!g->equal(g2)) {
        printf("\n---------------------------\nerror:%d\n---------------------------\n\n", i);
        g->print();
        g->print_name();

        printf("\n");

        g2->print();
        g2->print_name();
        return -1;
      }

      //---------------------------
      //end of Test

      //g->print();
      //g->print_graph();

      g->step_all();
      //printf("\n");
      //g->print();
      get_split(g, split_nodes);
      get_merge(g, merge_nodes);
      g->split_merge(split_nodes, merge_nodes);

    }

    //g->print_graph();
    printf("success\n");

  }

  /*
  Test graphs:

  graph* g = new graph(5);
  //g->randomize();
  g->print();

  graph* g2 = g->copy();

  g->print_graph();

  g->split(2);
  g->print_graph();

  g->merge(5);
  g->print_graph();

  g->merge(1);
  g->print_graph();

  g->split(1);
  g->print_graph();

  g->split(4);
  g->print_graph();

  g->merge(1);
  g->print_graph();

  printf("%s\n", g2->equal(g) ? "true" : "false");
  */

  /*
  Test nodes :

  node* node0 = new node(0);
  node* node1 = new node(1);
  node* node2 = new node(2);

  node0->merge_right(node1);
  node0->print(); printf("\n");

  node0->merge_right(node2);
  node0->print(); printf("\n");

  node* right = node0->copy();

  for (int i = 0; i < 4; i++) {
    printf("\nnext split:\n");
    right = right->right();
    right->print(); printf("\n");
  }

  node0 = new node(0);
  node0->merge_right(node1);
  node0->merge_right(node2);

  for (int i = 0; i < 4; i++) {
    printf("\nnext split:\n");
    right = node0->right();
    node0->set_left();

    node0->print(); printf("\n");
    right->print(); printf("\n");
  }*/
}
