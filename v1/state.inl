//#include <cstring> //f
#include <algorithm> //for sort
#include <cstddef> //for random
#include <cstring> //for memcpy
#include <cmath> //for floor


/*
state private functions
*/


int state::sort_in_place() {
  #pragma omp parallel
  {
    #pragma omp single
    if (n_clockwise != 0)
      std::sort(&particules[0], &particules[n_clockwise]);

    #pragma omp single
    if (n_particules != n_clockwise)
      std::sort(&particules[n_clockwise], &particules[n_particules]);
  }
  return 0; //can't have errors for now
}

int state::overflowed_clockwise() {
  for (int i = n_clockwise - 1; i > 0; i--)
    particules[i] = particules[i - 1]; //moove particule "right"

  particules[0] = 0; //only one particule can overflow at a time

  return 0; //can't have errors for now
}

int state::overflowed_conter_clockwise() {
  for (int i = n_clockwise; i < n_particules - 1; i++)
    particules[i] = particules[i + 1]; //moove particule "left"

  particules[n_particules - 1] = n_nodes - 1; //only one particule can overflow at a time

  return 0; //can't have errors for now
}


/*
state member function definition
*/


state* state::copy() {
  state *new_state = new state(n_particules);
  new_state->n_nodes = n_nodes;
  new_state->n_particules = n_particules;
  new_state->n_clockwise = n_clockwise;
  new_state->color = color;

  int size = (n_particules)*sizeof(int);
  new_state->particules = (int*)malloc(size);
  memcpy(new_state->particules, particules, size);

  return new_state;
}

bool state::equal(state *other) { //suposing both states are sorted
  if (n_nodes != other->n_nodes)
    return false;

  if (n_clockwise != other->n_clockwise)
    return false;

  if (n_particules != other->n_particules)
    return false;

  if (color != other->color)
    return false;

  if (n_particules == 0)
    return true;

  for (int i = 0; i < n_particules; i++)
    if (particules[i] != other->particules[i])
      return false;

  return true;
}

int state::randomize(int n_clockwise_) {
  n_clockwise = n_clockwise_;

  int jump, max_jump, pos = 0;
  for (int i = 0; i < n_clockwise; i++) { //generating a list of strictly different sorted integer in [0: n_nodes]
    max_jump = (int)std::floor((float)(n_nodes - pos) / (float)(n_clockwise - i)); //the max jump to have at least one jump between all particules
    jump = 1 + std::rand() % std::max(1, max_jump - 1); //std::max to avoid division by zero
    pos += jump; //incrementing the current position
    particules[i] = pos;
  }

  pos = 0;
  for (int i = n_clockwise; i < n_particules; i++) { //generating a list of strictly different sorted integer in [0: n_nodes]
    max_jump = (int)std::floor((float)(n_nodes - pos) / (float)(n_particules - i)); //the max jump to have at least one jump between all particules
    jump = 1 + std::rand() % std::max(1, max_jump - 1); //std::max to avoid division by zero
    pos += jump; //incrementing the current position
    particules[i] = pos;
  }

  return 0; //no need for sorting since the particules are generatated in a sorted way
}

int state::randomize() {
  int n_clockwise_ = 1 + std::rand() % (n_particules - 1); //choose the number of particules going one way
  return randomize(n_clockwise_);
}

int state::step_all() {
  int err = 0;

  #pragma omp parallel
  {
    #pragma omp for
    for (int i = 0; i < n_clockwise; i++)
      particules[i]++;

    #pragma omp for
    for (int j = n_clockwise; j < n_particules; j++)
      particules[j]--;
  }

  if (particules[n_clockwise - 1] >= n_nodes)
    err = overflowed_clockwise();


  if (particules[n_clockwise] < 0 && err == 0)
    err = overflowed_conter_clockwise();

  return err;
}

int state::split_node(int node) {
  #pragma omp parallel
  {
    #pragma omp for
    for (int i = 0; i < n_clockwise; i++)
      if (particules[i] >= node) //if the new node is in-between the origin and the particule
        particules[i]++; //move farther from the origin

    #pragma omp for
    for (int j = n_clockwise; j < n_particules; j++)
      if (particules[j] > node) //if the new node is in-between the origin and the particule
        particules[j]++; //move farther from the origin
  }

  n_nodes++; //increase the number of nodes
  return 0; //can't have errors for now
}

int state::split_nodes(int *nodes, int size) {
  if (size == 0)
    return 0;

  /*#pragma parallel
  {
    #pragma omp single
    {*/
      int num_add = size - 1;
      for (int i = n_clockwise - 1; i >= 0 ; i--) {
        particules[i] += num_add + 1; //move farther from the origin

        if (particules[i] - num_add - 1 == nodes[num_add]) { //check if we passed a node
          num_add--;
          if (num_add < 0) //check if their are other nodes to add
            break;
        }
      }
    /*}

    #pragma omp single
    {*/
      int num_add_ = size - 1;
      for (int j = n_particules - 1; j >= n_clockwise; j--) {
        if (particules[j] == nodes[num_add_]) { //check if we passed a node
          num_add_--;
          if (num_add_ < 0) //check if their are other nodes to add
            break;
        }

        particules[j] += num_add_ + 1; //move farther from the origin
      }
    /*}
  }*/

  n_nodes += size; //increase the number of nodes
  return 0; //can't have errors for now
}

int state::merge_node(int node) {
  if (node == n_nodes - 1) { //check if the node is the last
    color = !color; //change the color

    if (n_clockwise > 1)
      for (int i = 0; i < n_clockwise - 1; i++)
        particules[i] = particules[i + 1]; //moove particule "left"

    particules[n_clockwise - 1] = n_nodes - 1; //mooving the first particule at the last node

    for (int i = 0; i < n_particules; i++) //moove all particules clother to the origin
      particules[i]--;

    n_nodes--; //decrease the number of nodes
    return 0;
  }

  #pragma omp parallel for
  for (int i = 0; i < n_particules; i++)
    if (particules[i] > node) //if the new node is in-between the origin and the particule
      particules[i]--; //move clother to the origin

  n_nodes--; //decrease the number of nodes
  return 0; //can't have errors for now
}

int state::merge_nodes(int *nodes, int size) {
  if (size == 0)
    return 0;

  bool last_node = false;
  if (nodes[size - 1] == n_nodes - 1) { //check if the last node to delete is the last
    last_node = true; //note that the last node is to delete
    size--; //reduce the number of node to delete
    if (size <= 0) return merge_node(n_nodes - 1); //check if their are any node to delete

    color = !color; //change the color
    if (n_clockwise > 1)
      for (int i = 0; i < n_clockwise - 1; i++)
        particules[i] = particules[i + 1]; //moove particule "left"

    particules[n_clockwise - 1] = n_nodes - 1; //only one particule can overflow at a time
  }

  /*#pragma omp parallel
  {
    #pragma omp single
    {*/
      int num_add = 0;
      for (int i = 0; i < n_clockwise; i++) {
        if (num_add < size)
          if (particules[i] > nodes[num_add] - last_node) //check if we passed a node
            num_add++;

        particules[i] -= num_add + last_node; //move clother from the origin
      }
    /*}

    #pragma omp single
    {*/
      int num_add_ = 0;
      for (int i = n_clockwise; i < n_particules; i++) {
        if (num_add_ < size)
          if (particules[i] > nodes[num_add_] - last_node) //check if we passed a node
            num_add_++;

        particules[i] -= num_add_ + last_node; //move clother from the origin
      }
    /*}
  }*/

  n_nodes -= size + last_node; //increase the number of nodes
  return 0; //can't have errors for now
}


int state::merge_split(int* split_list, int split_size, int* merge_list, int merge_size) {
  state* split_node_list = new state(0); //create a state with merge nodes as particules
  split_node_list->n_nodes = n_nodes;
  split_node_list->n_particules = split_size;
  split_node_list->particules = split_list;
  split_node_list->n_clockwise = 0;

  int err = merge_nodes(merge_list, merge_size); //merge nodes
  if (err != 0) return err; //check for errors

  err = split_node_list->merge_nodes(merge_list, merge_size); //to know where to actually split the nodes
  if (err != 0) return err; //check for errors

  return split_nodes(split_node_list->particules, split_size); //split the nodes
}
