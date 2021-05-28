#include <algorithm> //for rotate


/*
round shift
*/


void graph::circular_shift_left(std::vector<int> &list) {
  std::rotate(list.begin(), list.begin() + 1, list.end());
}

void graph::circular_shift_right(std::vector<int> &list) {
  std::rotate(list.rbegin(), list.rbegin() + 1, list.rend());
}


/*
split and merge on lis
 */

void print_vector(std::vector<int> &v);

void graph::_split_right(std::vector<int> &positions, std::vector<int> &nodes) {
  if (nodes.empty() || positions.empty())
    return;

  int i = nodes.size() - 1;

  for (auto it = positions.rbegin(); it < positions.rend(); ++it) {
    /* check how many more nodes to split */
    while (*it < nodes[i]) {
      /* check if theire are anymore nodes to split */
      if (i <= 0)
        return;

      i--;
    }

    /* moove the particule */
    (*it) += i + 1;

  }
}

void graph::_split_left(std::vector<int> &positions, std::vector<int> &nodes) {
  if (nodes.empty() || positions.empty())
    return;

  int i = nodes.size() - 1;

  for (auto it = positions.rbegin(); it < positions.rend(); ++it) {
    /* check how many more nodes to split */
    while (*it <= nodes[i]) {
      /* check if theire are anymore nodes to split */
      if (i <= 0)
        return;

      i--;
    }

    /* moove the particule */
    (*it) += i + 1;
  }
}

void graph::_merge(std::vector<int> &positions, std::vector<int> &nodes, int n_nodes)  {
  if (nodes.empty() || positions.empty())
    return;

  int i = 0;

  /* check if the last node is merged with the first */
  bool last_node = *(nodes.end() - 1) == n_nodes - 1;

  /* check if a position is at zero */
  if (last_node && positions[0] == 0) {
    positions[0] = n_nodes - 1;
    circular_shift_left(positions);
  }

  for (auto it = positions.begin(); it < positions.end(); ++it) {
    /* check how many more nodes to split */
    if (i < nodes.size() - last_node)
      while (i < nodes.size() - last_node && *it > nodes[i])
        i++;

    /* move the particule */
    (*it) -= i + last_node;
  }
}
