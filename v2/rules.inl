void get_split(graph* g, std::vector<int> &split_nodes) {
  split_nodes.clear();

  auto it_left = g->left.begin();
  auto it_right = g->right.begin();

  while (it_left < g->left.end() && it_right < g->right.end()) {
    if (*it_left == *it_right) {
      split_nodes.push_back(*it_left);

      ++it_left; ++it_right;
    } else if (*it_left < *it_right) {
      ++it_left;
    } else {
      ++it_right;
    }
  }
}

void get_merge(graph* g, std::vector<int> &merge_nodes) {
  merge_nodes.clear();

  auto it_left = g->left.begin();
  auto it_right = g->right.begin();

  int n_nodes = g->num_nodes();

  while (it_left < g->left.end() && it_right < g->right.end()) {
    if (*it_left == *it_right - 1) {
      if(it_left < g->left.end() - 1)
        if(*(it_left + 1) == *it_right)
          goto abort;

      if(it_right > g->right.begin())
        if(*(it_right - 1) == *it_left)
          goto abort;

      merge_nodes.push_back(*it_left);

abort:
      ++it_left; ++it_right;
    } else if (*it_left < *it_right - 1) {
      ++it_left;
    } else {
      ++it_right;
    }
  }

  if (*(g->left.end() - 1) == n_nodes - 1 && g->right[0] == 0 &&
    *(g->right.end() - 1) != n_nodes - 1 && g->left[0] != 0)
    merge_nodes.push_back(n_nodes - 1);
}
