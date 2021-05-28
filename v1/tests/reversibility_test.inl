//function to read a bit out of an integer
bool bit_out_of_int(long int integer, int bit) {
  return (integer >> bit) & true;
}

//function to calculate the number of graphs of a given size
long int number_of_graphs(int size) {
  if (size == 0) return 2; //number of graph with 0 node
  if (size == 1) return 8; //number of graph with 1 node
  return 4 * number_of_graphs(size - 1); //power of 4
}

//function to generate a unic graph of a given size from an integer
state* int_to_graph(int size, long int integer_) {
  //generating the color
  int integer = integer_ >> 1;
  bool color = integer_ & true;

  // counting the number of particules
  int n_particules = 0;
  int n_clockwise = 0;
  for (int i = 0; i < 2*size; i++) {
    int bit = bit_out_of_int(integer, i);
    n_particules += bit;
    if (i % 2 == 1)
      n_clockwise += bit;
  }

  //assigning a graph with the right number of particules
  state *s = new state(n_particules);
  s->n_clockwise = n_clockwise;
  s->n_nodes = size;
  s->color = color;

  //assigning the particules positions
  if (n_particules != 0) {
    int acc_clockwise = 0;
    int acc_conter_clockwise = n_clockwise;
    int i = 0;
    while (acc_clockwise < n_clockwise || acc_conter_clockwise < n_particules) {
      if (bit_out_of_int(integer, i))
        if (i % 2 == 1) {
          s->particules[acc_clockwise] = (i - 1)/2;
          acc_clockwise++;
        } else {
          s->particules[acc_conter_clockwise] = i/2;
          acc_conter_clockwise++;
        }
      i++;
    }
  }

  return s;
}


/*
function to check a state
*/


bool check(state *s) {
  if (s->n_clockwise > s->n_particules)
    printf("more particules going clockwise than particules\n");

  if (s->n_clockwise > 0)
    for (int i = 0; i < s->n_clockwise - 1; i++) {
      if (s->particules[i] < 0 || s->particules[i] >= s->n_nodes) {
        printf("particule going clockwise over the limit at node %d/%ld\n", s->particules[i], s->n_nodes);
        return false;
      }

      if (s->particules[i] > s->particules[i + 1]) {
        printf("particule going clockwise not sorted at node %d/%ld\n", s->particules[i], s->n_nodes);
        return false;
      }

      if (s->particules[i] == s->particules[i + 1]) {
        printf("two particule going clockwise at node %d/%ld\n", s->particules[i], s->n_nodes);
        return false;
      }
    }


  if (s->n_particules - s->n_clockwise > 0)
    for (int i = s->n_clockwise; i < s->n_particules - 1; i++) {
      if (s->particules[i] < 0 || s->particules[i] >= s->n_nodes) {
        printf("particule going counter clockwise over the limit at node %d/%ld\n", s->particules[i], s->n_nodes);
        return false;
      }

      if (s->particules[i] > s->particules[i + 1]) {
        printf("particule going counter clockwise not sorted at node %d/%ld\n", s->particules[i], s->n_nodes);
        return false;
      }

      if (s->particules[i] == s->particules[i + 1]) {
        printf("two particule going counter clockwise at node %d/%ld\n", s->particules[i], s->n_nodes);
        return false;
      }
    }

  return true;
}


/*
function to list all graphs of a given size
*/


state** list_all_graphs(int size) {
  const int num_graph = number_of_graphs(size);
  state **state_list = (state**)malloc(num_graph*sizeof(state*));

  #pragma omp parallel for
  for (long int i = 0; i < num_graph; i++) {
    state_list[i] = int_to_graph(size, i);
    check(state_list[i]);
  }


  return state_list;
}


/*
functions to test if all state in a list are different
*/


bool check_distinct(state **state_list, int size) {
  for (int i = 0; i < size; i++)
    for (int j = i + 1; j < size; j++)
      if(state_list[i]->equal(state_list[j]) && state_list[i]->n_particules != 2)
        return false;

  return true;
}

bool check_distinct(state **state_list, int size, int *i_, int *j_) {
  for (int i = 0; i < size; i++)
    for (int j = i + 1; j < size; j++)
      if(state_list[i]->equal(state_list[j]) && state_list[i]->n_particules != 2) {
        *i_ = i;
        *j_ = j;
        return false;
      }

  return true;
}
