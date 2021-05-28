/* constructor */


node::node(int idx) {
  countains_zero_ = idx == 0;
  zero_idx = 0;
  names.push_back(idx);
  bracket_depth.push_back(0);
  immediate_left_bracket.push_back(0);
}


/*
copy operator
*/


node* node::copy() { //tTODO
  return copy(0, names.size());
}


/*
comparison tools
*/


bool node::countains_zero() {
  return countains_zero_;
}

bool node::equal(node* other) {
  /* check if both countain zero or both don't */
  if (countains_zero_ != other->countains_zero_)
    return false;

  /* if both countainsvzero, check if they are at the same position */
  if (countains_zero_)
    if (zero_idx != other->zero_idx)
      return false;

  /* compare sizes */
  if (names.size() != other->names.size())
    return false;

  /* check names */
  if (names != other->names)
    return false;

  /* check bracket_depth */
  if (bracket_depth != other->bracket_depth)
    return false;

  /* check immediate_left_bracket */
  if (immediate_left_bracket != other->immediate_left_bracket)
    return false;

  return true;
}


/*
merge and split operators
*/#include <cstdio>


void node::merge_right(node* right) { //TODO
  /* empty element */
  printf("%ld %d\n", right->names.size(), right->names[0] == EMPTY_ELLEMENT);
  if (right->_is_empty()) {
    /* merging phi_n and phi_{n + 1} */

    printf("%ld %d %d %d\n", names.size(), names[0] == EMPTY_ELLEMENT, bracket_depth[0], right->bracket_depth[0] - 1);
    if (_is_empty() && _empty_depth() == right->_empty_depth() - 1)
      return;
    printf("%ld %d %d %d\n", names.size(), names[0] == EMPTY_ELLEMENT, bracket_depth[0], right->bracket_depth[0] - 1);

    /* the other is just a simple empty ellement */
    if (right->_empty_depth() == 0)
      return;
  }

  /* countains_zero */
  if (right->countains_zero_) {
    countains_zero_ = true;
    zero_idx = right->zero_idx + names.size();
  }

  /* merge all vector */
  names.insert(names.end(), right->names.begin(), right->names.end());
  bracket_depth.insert(bracket_depth.end(), right->bracket_depth.begin(), right->bracket_depth.end());
  immediate_left_bracket.insert(immediate_left_bracket.end(), right->immediate_left_bracket.begin(), right->immediate_left_bracket.end());

  /* increment left bracket */
  immediate_left_bracket[0]++;

  /* increment the bracket depth */
  for (auto it = bracket_depth.begin(); it < bracket_depth.end(); ++it)
    (*it)++;
}

node* node::right() { //TODO
  /* calculate the split index if it hasen't been calculated */
  if (!split_index_calculated) calculate_split_index();

  if (names.size() == split_index)
    if (names.size() == 1 && names[0] == EMPTY_ELLEMENT) {
      /* copy this node */
      node* r = copy();
      r->bracket_depth[0]++; //increment the depth
      r->immediate_left_bracket[0]++; //increment the immediate_left_bracket

      return r;
    } else {
      return new node(EMPTY_ELLEMENT); //return an empty node
    }

  /* copy the node up to the split index */
  node* r = copy(split_index, names.size());

  /* decrement the bracket depth */
  for (auto it = r->bracket_depth.begin(); it < r->bracket_depth.end(); ++it)
    (*it)--;

  return r;
}

void node::set_left() { //TODO
  /* calculate the split index if it hasen't been calculated */
  if (!split_index_calculated) calculate_split_index();

  /* check if the node is a single node*/
  if (names.size() == 1 && (bracket_depth[0] == 0 || names[0] == EMPTY_ELLEMENT))
    return;

  /* reset split calculated */
  split_index_calculated = false;

  /* remove elements after the split_node */
  names.erase(names.begin() + split_index, names.end());
  bracket_depth.erase(bracket_depth.begin() + split_index, bracket_depth.end());
  immediate_left_bracket.erase(immediate_left_bracket.begin() + split_index, immediate_left_bracket.end());

  /* decreament left bracket */
  immediate_left_bracket[0]--;

  /* decrement the bracket depth */
  for (auto it = bracket_depth.begin(); it < bracket_depth.end(); ++it)
    (*it)--;

  /* countains zero */
  if (countains_zero_ && zero_idx >= names.size())
    countains_zero_ = false;
}
