int rule::get_merge(state *s, int* list_merge, int *n_merge) {
  *n_merge = 0;

  if (s->n_clockwise == 0 || s->n_particules == s->n_clockwise) //can't have intercations if we don't have particules going both ways
    return 0;

  int i = 0;
  int j = s->n_clockwise;

  while (j < s->n_particules && i < s->n_clockwise) {
    if (s->particules[i] == s->particules[j] + 1) { //if particules are at one node appart
      if (i != 0)
        if (s->particules[i - 1] == s->particules[j]) //check if their aren't two particule at the left node
          goto abort;

      if (j != s->n_particules)
        if (s->particules[i] == s->particules[j + 1]) //check if their aren't two particule at the right node
          goto abort;

      list_merge[(*n_merge)++] = s->particules[j];

abort:
      i++; j++;
    } else if (s->particules[i] > s->particules[j] + 1) { //supposing the particules are sorted
      j++;
    } else {
      i++;
    }
  }

  if (s->particules[s->n_particules - 1] == s->n_nodes - 1 &&  //particule going clockwise at the first node
    s->particules[0] == 0) { //particule going counter-clockwise at the last node)
    if (s->n_particules > s->n_clockwise)
      if(s->particules[s->n_clockwise] == 0) //no particules going counter-clockwise at the first node
        goto abort_last;

    if (s->n_clockwise > 1)
      if (s->particules[s->n_clockwise - 1] == s->n_nodes - 1) //no particules going clockwise at the last node
        goto abort_last;

    list_merge[(*n_merge)++] = s->n_nodes - 1; //interaction at the last node
  }

abort_last:
  return 0; //can't have errors for now
}

int rule::get_split(state *s, int* list_split, int *n_split) {
  *n_split = 0;

  if (s->n_clockwise == 0 || s->n_particules == s->n_clockwise) //can't have intercations if we don't have particules going both ways
    return 0;

  int i = 0;
  int j = s->n_clockwise;

  while (j < s->n_particules && i < s->n_clockwise) {
    if (s->particules[i] == s->particules[j]) { //if particules are at the same node
      list_split[(*n_split)++] = s->particules[i];

      i++; j++;
    } else if (s->particules[i] > s->particules[j]) { //supposing the particules are sorted
      j++;
    } else {
      i++;
    }
  }

  return 0; //can't have errors for now
}
