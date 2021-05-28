int state_list::add_state(state *s) {
  for (auto state = states.begin(); state != states.end(); ++state)
    if (s->equal(*state)) {
      (*state)->probability += s->probability;

      if ((*state)->probability == 0)
        states.erase(state);

      return 0; //can't have errors for now
    }

  states.push_back(s);
  return 0; //can't have errors for now
}

int state_list::step_all(rule* r) {
  int size = states[0]->n_particules;
  int *split_indexes = (int*)malloc(states.size() * size * sizeof(int));
  int *merge_indexes = (int*)malloc(states.size() * size * sizeof(int));

  #pragma omp parallel for
  for (int i = 0; i < states.size(); ++i) {
    int split_size, merge_size;

    states[i]->step_all();

    r->get_split(states[i], &split_indexes[size * i], &split_size);
    r->get_merge(states[i], &merge_indexes[size * i], &merge_size);
    states[i]->merge_split(&split_indexes[size * i], split_size, &merge_indexes[size * i], merge_size);
  }

  return 0; //shortcut
}
