/* copy operator */


node* node::copy(int start, int end) {
  node* copied = new node(0);

  /* assign new vector */
  copied->names = std::vector<int>(names.begin() + start, names.begin() + end);
  copied->bracket_depth = std::vector<int>(bracket_depth.begin() + start, bracket_depth.begin() + end);
  copied->immediate_left_bracket = std::vector<int>(immediate_left_bracket.begin() + start, immediate_left_bracket.begin() + end);

  /* check if it countains zero */
  if (countains_zero_ && zero_idx < end && zero_idx >= start) {
    copied->countains_zero_ = true;
    copied->zero_idx = zero_idx - start;
  } else {
    copied->countains_zero_ = false;
  }

  return copied;
}


/*
calculate split index
*/


void node::calculate_split_index() {
  if (!split_index_calculated) {
    if (names.size() > 1) {
      for (split_index = 0; split_index < names.size(); split_index++)
        if (bracket_depth[split_index] == immediate_left_bracket[split_index] + 1)
          break;
    } else {
      split_index = 1;
    }

    /* remember that the split index was caclulated */
    split_index_calculated = true;
  }
}


/*
empty depth
*/


bool node::_is_empty() {
  return names.size() == 1 && names[0] == EMPTY_ELLEMENT;
}

int node::_empty_depth() {
  return bracket_depth[0];
}
