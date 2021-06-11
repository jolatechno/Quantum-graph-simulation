#include "../classical/rules.hpp"
#include "../classical/graph.hpp"
#include "../quantum/rules.hpp"
#include "../quantum/state.hpp"
#include "../quantum/checks.hpp"
#include "../utils/quantum_test_parser.hpp"

#define _USE_MATH_DEFINES
#include <cmath>

int main(int argc, char* argv[]) {
	cxxopts::Options options("check for quantum injectivity");
    auto [rule, reversed_rule, rule_, reversed_rule_, n_iter, max_n_graphs, size, normalize_] = parse_test_quantum(options, argc, argv);
	
	printf("A graphs is inputed by a series of ndoes, having particules (`>` for right and `<` for left) or not.\n");
	printf("With nodes being separated by a '-'.\n");
	printf("you can also input 'r' to create a random graph of size 6\n");
	printf("For exemple the graph:\n    >-<>--<\nis read as:\n    -| |0|>|--|<|1|>|--| |2| |--|<|3| |-\n");

	while (true) {
		printf("\n----------------------------------------------------\n\nPleas enter next graph (Ctrl+C to exit):\n");

		std::vector<BOOL_TYPE> left, right;
	    graph_t g_1;

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
    	g_1 = graph_t(left, right);
    	printf("\nread from input*: "); print(g_1);
    	goto loop;

random:
    	g_1 = graph_t(size);
    	printf("new graph(%d):  ", size); print(g_1);

    	g_1.randomize();
    	printf("\nrandomize(%d)*:  ", size); print(g_1); printf("\n");

loop:
		const auto sum = [&](state_t *s) {
			if (normalize_)
	            s->normalize();

	        if (max_n_graphs > 0)
	        	s->discard_all(max_n_graphs);


	        #ifdef TEST
				if (check(s)) {
					printf("\nstate is OK\n");
				} else
					printf("\nstate is not OK\n");
			#endif
		};

		state_t* s = new state(g_1);

		for (int i = 0; i < n_iter; ++i) {
			s->step_all(rule);
			s->reduce_all();

			sum(s);
		}

		//----
		
		printf("%ld graph of size ", s->graphs().size());
        size_stat(s);
        printf(" after %d %s()\n", n_iter, rule_.c_str());

		for (int i = 0; i < n_iter; ++i) {
			s->step_all(reversed_rule);
			s->reduce_all();

			sum(s);
		}

		printf("\nafter %d %s():\n", n_iter, reversed_rule_.c_str()); print(s);
	}
}
