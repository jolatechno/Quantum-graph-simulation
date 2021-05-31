#include "../classical/rules.hpp"
#include "../classical/graph.hpp"
#include "../quantum/rules.hpp"
#include "../quantum/state.hpp"
#include "../quantum/checks.hpp"
#include <stdio.h>
#include <math.h> //pow cos sin and sqrt
#include <complex>


#define _USE_MATH_DEFINES
#include <cmath>

int main() {
	printf("A graphs is inputed by a series of ndoes, having particules (`>` for right and `<` for left) or not.\n");
	printf("With nodes being separated by a '-'.\n");
	printf("you can also input 'r' to create a random graph of size 6\n");
	printf("For exemple the graph:\n    >-<>--<\nis read as:\n    -| |0|>|--|<|1|>|--| |2| |--|<|3| |-\n");

	while (true) {
		printf("\n----------------------------------------------------\n\nPleas enter next graph (Ctrl+C to exit):\n");

		std::vector<unsigned int> left, right;
		graph_t* g_1;
		int n = 0;

		while (true) {
			switch (std::getchar()) {
				case '<':
					left.push_back(n);
					break;

				case '>':
          			right.push_back(n);
          			break;

        		case '-':
          			++n;
          			break;

        		case '\n':
          			++n;
          			goto read;

        		case 'r':
        			while(std::getchar() != '\n') { }
          			goto random;

        		default:
          			return -1;
      		}
    	}

read:
    	g_1 = new graph(n, left, right);
    	printf("\nread from input*: "); g_1->print();
    	goto loop;

random:
    	g_1 = new graph(6);
    	printf("new graph(6):  "); g_1->print();

    	g_1->randomize();
    	printf("\nrandomize()*:  "); g_1->print(); printf("\n");

loop:

		state_t* s = new state(g_1);
		s->set_params(M_PI_4, M_PI_4);

		s->split_merge_all();
		s->reduce_all();

		printf("\nmerge_split():\n"); s->print();

		s->split_merge_all();
		s->reduce_all();

		printf("\n\nmerge_split():\n"); s->print();

		if (check(s)) {
			printf("\nstate is OK\n");
		} else
			printf("\nstate is not OK\n");
	}
}
