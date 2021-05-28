#pragma once
#include <vector>

#define EMPTY_ELLEMENT -1

typedef class node {
private:
  /* list describing the node */
  /*std::vector<int> names;
  std::vector<int> bracket_depth;
  std::vector<int> immediate_left_bracket;*/

  /* presence and position of the name zero */
  bool countains_zero_;
  int zero_idx;

  /* split index */
  bool split_index_calculated = false;
  int split_index;
  void calculate_split_index();

  /* copy up to an index */
  node* copy(int start, int end);

  /* empty depth */
  bool _is_empty();
  int _empty_depth();

public:
  /* list describing the node */
  std::vector<int> names;
  std::vector<int> bracket_depth;
  std::vector<int> immediate_left_bracket;
  /* constructor and destructor */
  node(int idx);
  node* copy();

  /* comparison tools */
  bool countains_zero();
  bool equal(node* other);

  /* merge and split */
  void merge_right(node* right);
  node* right();
  void set_left();

  /* debuging function */
  void print();
} node;
