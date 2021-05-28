#pragma once
#include "node.hpp"
#include "graph_names.hpp"

typedef class graph {
private:
  /* graph name */
  graph_name* name;

  /* round shift */
  void circular_shift_left(std::vector<int> &list);
  void circular_shift_right(std::vector<int> &list);

  /* split and merge on list */
  void _split_left(std::vector<int> &positions, std::vector<int> &nodes);
  void _split_right(std::vector<int> &positions, std::vector<int> &nodes);
  void _merge(std::vector<int> &positions, std::vector<int> &nodes, int n_nodes);
public:
  /* number of nodes */
  int num_nodes();

  /* particules list */
  std::vector<int> left;
  std::vector<int> right;

  /* constructor */
  graph(int size);
  graph* copy();
  void randomize(int n_left, int n_right);

  /* comparison tools */
  bool equal(graph* other);

  /* split and merge */
  void split(std::vector<int> &nodes);
  void merge(std::vector<int> &nodes);
  void split_merge(std::vector<int> &split_nodes, std::vector<int> &merge_nodes);
  void step_all();

  /* debuging */
  void print();
  void print_name();

} graph;
