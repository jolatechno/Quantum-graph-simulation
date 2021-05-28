#include <cstdlib>


/*
graph_name constructor
*/


graph_name::graph_name(int size) {
  for (int i = 0; i < size; i++)
    /* initialize node list */
    nodes.push_back(new node(i));

  /* initialize zero_index */
  zero_index = 0;
}


/*
copy
*/


graph_name* graph_name::copy() { //TODO
  graph_name* copied = new graph_name(0);

  /* assign zero_index */
  copied->zero_index = zero_index;

  /* copy nodes */
  for (auto it = nodes.begin(); it < nodes.end(); ++it)
    copied->nodes.push_back((*it)->copy());

  return copied;
}


/*
comparison
*/


bool graph_name::equal(graph_name* other) {
  /* check if both graph_name have the smae number of nodes */
  if (nodes.size() != other->nodes.size())
    return false;

  /* calculate the offset */
  int j, offset = zero_index - other->zero_index;

  for (int i = 0; i < nodes.size(); i++) {
    j = (i - offset)%nodes.size();

    /* compare nodes */
    if (!nodes[i]->equal(other->nodes[j])) {
      nodes[i]->print(); printf(" != "); other->nodes[j]->print(); printf(" at node %d (offset = %d)\n", i, offset);
      printf("%d %d sizes: %ld %ld bracket_depth: %d %d immediate_left_bracket: %d %d\n", nodes[i]->names[0] == EMPTY_ELLEMENT, other->nodes[j]->names[0] == EMPTY_ELLEMENT, nodes[i]->names.size(), other->nodes[j]->names.size(), nodes[i]->bracket_depth[0], other->nodes[j]->bracket_depth[0], nodes[i]->immediate_left_bracket[0], other->nodes[j]->immediate_left_bracket[0]);
      return false;
    }


  }

  return true;
}


/*
split and merge
*/

void graph_name::split(int i) {
  /* create new node */
  nodes.insert(nodes.begin() + i + 1, nodes[i]->right());

  /* set old node */
  nodes[i]->set_left();

  /* update the zero_index */
  if (zero_index == i) {
    zero_index += nodes[i + 1]->countains_zero();
  } else if (zero_index > i)
    zero_index++;
}

void graph_name::merge(int i) {
  int other_node = (i + 1)%nodes.size();

  /* merge the node */
  nodes[i]->merge_right(nodes[other_node]);

  /* delete the other node */
  nodes.erase(nodes.begin() + other_node);

  /* update zero_index */
  if (i == nodes.size() && zero_index == 0) {
    zero_index = nodes.size() - 1;
  } else if (zero_index > i)
    zero_index--;
}
