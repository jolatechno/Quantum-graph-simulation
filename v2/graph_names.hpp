#pragma once
#include "node.hpp"

typedef class graph_name {
  friend class graph;

private:
  /* node list */
  std::vector<node*> nodes;

  /* position of the node with the zero name */
  int zero_index;

public:
  /* constructor */
  graph_name(int size);
  graph_name* copy();

  /* comparison tools */
  bool equal(graph_name* other);

  /* split and merge */
  void split(int i);
  void merge(int i);

  /* debuging */
  void print();

} graph_name;
