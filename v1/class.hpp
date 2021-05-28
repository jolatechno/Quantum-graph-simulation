#include <omp.h> //for parralelism
#include <stdbool.h> //for bool
#include <cstdlib> //for malloc etc..
#include <vector> //for vector

/*
defining flags
*/


#define STRATEGY_INTERACT_ALL 0
#define STRATEGY_INTERACT_NONE 1
void print_list(int *list, int size);


/*
state class definition
*/


typedef class state {
private:
  int sort_in_place();
  #pragma omp declare target
  int inline overflowed_clockwise();
  int inline overflowed_conter_clockwise();
  #pragma omp end declare target
public:
  bool color = false;
  double probability = 1;
  size_t n_nodes;
  size_t n_particules, n_clockwise = 0;
  int *particules;

  state(size_t n) { //allocate
    n_particules = n;
    particules = (int*)malloc(n*sizeof(int));
  }
  ~state() { //free
    free(particules);
  }

  int randomize();
  int randomize(int n_clockwise_);
  state* copy();
  #pragma omp declare target
  bool inline equal(state *other);
  int inline switch_direction(int i);
  int inline split_node(int node);
  int inline split_nodes(int *nodes, int size);
  int inline merge_node(int node);
  int inline merge_nodes(int *nodes, int size);
  int inline merge_split(int* split_list, int split_size, int* merge_list, int merge_size);
  int inline step_all();
  #pragma omp end declare target
} state;


/*
class defining the rule of an interaction
*/


typedef class rule {
public:
  #pragma omp declare target
  int inline get_merge(state *s, int* list_merge, int *n_merge);
  int inline get_split(state *s, int* list_split, int *n_split);
  #pragma omp end declare target
} rule;


/*
------------------------------------------------------------
FOR QUANTUM CASE ONLY:
------------------------------------------------------------
state_list class definition
*/


typedef class state_list {
public:
  double prob_min;
  std::vector<state*> states;

  state_list(); //TODO

  int step_all(rule* r);
  #pragma omp declare target
  int add_state(state *s);
  int step(double interaction_prob);
  #pragma omp end declare target
} state_list;
