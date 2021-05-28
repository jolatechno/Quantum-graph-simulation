#include <cstdio>

void node::print() {
  int left_bracket, right_bracket;

  for (int i = 0; i < names.size() - 1; i++) {
    left_bracket = immediate_left_bracket[i];
    right_bracket = bracket_depth[i] - bracket_depth[i + 1];

    /* print left brackets */
    for (int j = 0; j < left_bracket; j++) printf("[");

    /* print names */
    if (names[i] == EMPTY_ELLEMENT) {
      printf("[ ]");
    } else {
      /* print spaces */
      if (immediate_left_bracket[i] == 0) printf(" ");

      printf("%d", names[i]);
    }

     /* print right brackets */
    for (int j = 0; j < right_bracket; j++) printf("]");
 }

 /*
 last iteration
 */

 int i = names.size() - 1;

 left_bracket = immediate_left_bracket[i];
 right_bracket = bracket_depth[i];

 /* print left brackets */
 for (int j = 0; j < left_bracket; j++) printf("[");

 /* print names */
 if (names[i] == EMPTY_ELLEMENT) {
   printf("[ ]");
 } else {
   /* print spaces */
   if (immediate_left_bracket[i] == 0) printf(" ");

   printf("%d", names[i]);
 }

 /* print right brackets */
 for (int j = 0; j < right_bracket; j++) printf("]");
}
