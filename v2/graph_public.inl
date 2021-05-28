#include <cstdlib>
#include <cstddef> //for random
#include <cmath> //for floor


/*
graph constructor
*/


graph::graph(int size) {
  name = new graph_name(size);
}


/*
copy
*/


graph* graph::copy() { //TODO
  graph* copied = new graph(0);

  /* copy the name */
  copied->name = name->copy();

  /* copy particules */
  copied->left = left;
  copied->right = right;

  return copied;
}


/*
number of nodes
*/


int graph::num_nodes() {
  return name->nodes.size();
}


/*
randomize
*/


void graph::randomize(int n_left, int n_right) {
  /* read the number of nodes */
  int n_nodes = num_nodes();

  int jump, max_jump, pos = 0;
  for (int i = 0; i < n_right; i++) { //generating a list of strictly different sorted integer in [0: n_nodes]
    max_jump = (int)std::floor((float)(n_nodes - pos) / (float)(n_right - i)); //the max jump to have at least one jump between all particules
    jump = 1 + std::rand() % std::max(1, max_jump - 1); //std::max to avoid division by zero
    pos += jump; //incrementing the current position
    right.push_back(pos);
  }

  pos = 0;
  for (int i = 0; i < n_left; i++) { //generating a list of strictly different sorted integer in [0: n_nodes]
    max_jump = (int)std::floor((float)(n_nodes - pos) / (float)(n_left - i)); //the max jump to have at least one jump between all particules
    jump = 1 + std::rand() % std::max(1, max_jump - 1); //std::max to avoid division by zero
    pos += jump; //incrementing the current position
    left.push_back(pos);
  }
}


/*
comparison
*/


bool graph::equal(graph* other) {
  /* cehck if both graph have the same number of particules going one way.. */
  if (left.size() != other->left.size())
    return false;

  /* ...and the other */
  if (right.size() != other->right.size())
    return false;

  /* check if both graph have the same number of nodes */
  if (num_nodes() != other->num_nodes())
    return false;

  /* calculate the offset */
  int j, idx_offset, offset = name->zero_index - other->name->zero_index;

  idx_offset = 0;
  if (offset > 0) {
    while (other->right[idx_offset] < offset)
      idx_offset++;
  } else if (offset < 0) {
    while (right[idx_offset] < -offset)
      idx_offset++;
  }

  /* compare particules going one way... */
  for (int i = 0; i < right.size(); i++) {
    j = (i + idx_offset)%right.size();

    if (right[i] != (other->right[j] + offset)%num_nodes())
        return false;

  }

  idx_offset = 0;
  if (offset > 0) {
    while (other->left[idx_offset] < offset)
      idx_offset++;
  } else if (offset < 0) {
    while (left[idx_offset] < -offset)
      idx_offset++;
  }

  /* ...and  the other way */
  for (int i = 0; i < left.size(); i++) {
    j = (i + idx_offset)%left.size();

    if (left[i] != (other->left[j] + offset)%num_nodes())
        return false;

  }

  //return name->equal(other->name);

  if (!name->equal(other->name)) {
    printf("false at name\n");
    return false;
  }

  return true;
}


/*
split and merge
*/

void graph::split(std::vector<int> &nodes) {
  /* split */
  _split_left(left, nodes);
  _split_right(right, nodes);

  /* split names */
  for (auto it = nodes.rbegin(); it != nodes.rend(); ++it)
    name->split(*it);
}

void graph::merge(std::vector<int> &nodes) {
  /* get the number of nodes */
  int n_nodes = num_nodes();

  /* merge */
  _merge(left, nodes, n_nodes);
  _merge(right, nodes, n_nodes);

  /* merge names */
  for (auto it = nodes.rbegin(); it != nodes.rend(); ++it)
    name->merge(*it);
}

void graph::split_merge(std::vector<int> &split_nodes, std::vector<int> &merge_nodes) {
  /* "merge" the positions of the nodes to merge */
  _merge(split_nodes, merge_nodes, num_nodes());

  /* merge the actual nodes */
  merge(merge_nodes);

  /* split */
  split(split_nodes);
}


/*
step all
*/


void graph::step_all() {
  /* get the number of nodes */
  int n_nodes = num_nodes();

  /* moove particules going one way... */
  for (auto it = left.begin(); it < left.end(); ++it)
    (*it)--;

  /* check if one overflowed */
  if (left[0] < 0) {
    left[0] = n_nodes - 1;
    circular_shift_left(left);
  }

  /* moove particules going one way */
  for (auto it = right.begin(); it < right.end(); ++it)
    (*it)++;

  /* check if one overflowed */
  if (*(right.end() - 1) >= n_nodes) {
    circular_shift_right(right);
    right[0] = 0;
  }
}
