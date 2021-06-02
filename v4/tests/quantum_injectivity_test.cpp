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
	    auto [non_merge, merge] = unitary(M_PI_4, M_PI_2);
	    auto rule = step_split_merge_all(non_merge, merge);
	    auto reversed_rule = reversed_step_split_merge_all(non_merge, merge);

		int n_iter = 5;

		for (int i = 0; i < n_iter; ++i) {
			s->step_all(rule);
			s->reduce_all();

			#ifdef TEST
				if (check(s)) {
					printf("\nstate is OK\n");
				} else
					printf("\nstate is not OK\n");
			#endif
		}
		
		if (n_iter <= 2) {
			printf("\nafter %d step_merge_split():\n", n_iter); s->print();
		} else {
			auto [avg, std_dev] = s->size_stat();
        	printf("%ld graph of size %Lf±%Lf after %d step_merge_split()\n", s->graphs().size(), avg, std_dev, n_iter);
		}

		for (int i = 0; i < n_iter; ++i) {
			s->step_all(reversed_rule);
			s->reduce_all();

			#ifdef TEST
				if (check(s)) {
					printf("\nstate is OK\n");
				} else
					printf("\nstate is not OK\n");
			#endif
		}

		printf("\nafter %d reversed_merge_split_step():\n", n_iter); s->print();
	}
}
