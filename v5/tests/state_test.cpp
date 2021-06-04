#include "../classical/rules.hpp"
#include "../classical/graph.hpp"
#include "../quantum/rules.hpp"
#include "../quantum/state.hpp"
#include "../quantum/checks.hpp"

#define _USE_MATH_DEFINES
#include <cmath>

int main() {
	printf("A graphs is inputed by a series of ndoes, having particules (`>` for right and `<` for left) or not.\n");
	printf("With nodes being separated by a '-'.\n");
	printf("you can also input 'r' to create a random graph of size 6\n");
	printf("For exemple the graph:\n    >-<>--<\nis read as:\n    -| |0|>|--|<|1|>|--| |2| |--|<|3| |-\n");

	while (true) {
		printf("\n----------------------------------------------------\n\nPleas enter next graph (Ctrl+C to exit):\n");

		std::vector</*bool*/ char> left, right;
	    graph_t* g_1;

	    bool l = false;
	    bool r = false;

	    while (true) {
	    	switch (std::getchar()) {
	      		case '<':
	      			l = true;
	      			break;

	      		case '>':
	      			r = true;
	      			break;

	      		case '-':
	      			left.push_back(l);
	      			right.push_back(r);
	      			l = false;
	      			r = false;
	      			break;

	      		case '\n':
	      			left.push_back(l);
	      			right.push_back(r);
	      			l = false;
	      			r = false;
	      			goto read;

	      		case 'r':
	      			while(std::getchar() != '\n') { }
	      			goto random;

	      		default:
	      			return -1;
	    	}
	    }

read:
    	g_1 = new graph(left, right);
    	printf("\nread from input*: "); print(g_1);
    	goto loop;

random:
    	g_1 = new graph(6);
    	printf("new graph(6):  "); print(g_1);

    	g_1->randomize();
    	printf("\nrandomize()*:  "); print(g_1); printf("\n");

loop:

		state_t* s = new state(g_1);
		auto [non_merge, merge] = unitary(M_PI_4, M_PI_2);
    	auto rule = split_merge_all(non_merge, merge);
    	auto rule_ = erase_create_all(non_merge, merge);

		s->step_all(rule);
		s->reduce_all();

		printf("\nmerge_split():\n"); print(s);

		s->step_all(rule);
		s->reduce_all();

		printf("\n\nmerge_split():\n"); print(s);

		s->step_all(rule_);
		s->reduce_all();

		printf("\nerase_create():\n"); print(s);

		s->step_all(rule_);
		s->reduce_all();

		printf("\n\nerase_create():\n"); print(s);

		if (check(s)) {
			printf("\nstate is OK\n");
		} else
			printf("\nstate is not OK\n");
	}
}
