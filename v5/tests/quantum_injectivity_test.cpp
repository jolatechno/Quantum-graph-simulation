#include "../classical/rules.hpp"
#include "../classical/graph.hpp"
#include "../quantum/rules.hpp"
#include "../quantum/state.hpp"
#include "../quantum/checks.hpp"
#include <ctime>

#define _USE_MATH_DEFINES
#include <cmath>

#ifndef N_ITER
    #define N_ITER 5
#endif

#ifndef TETA
    #define TETA M_PI_4
#endif

#ifndef PHI
    #define PHI M_PI_2
#endif

int main() {
	std::srand(std::time(nullptr)); // use current time as seed for random generator
	
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
    	g_1 = graph_t(6);
    	printf("new graph(6):  "); print(g_1);

    	g_1.randomize();
    	printf("\nrandomize()*:  "); print(g_1); printf("\n");

loop:
		const auto sum = [&](state_t *s) {
			PROBA_TYPE proba = 0;
			for (auto const & [_, mag] : s->graphs())
				proba += std::norm(mag);

			printf("total proba is %Lf\n", proba);

			#ifdef NORMALIZE
	            s->normalize();
	        #endif
		};

		state_t* s = new state(g_1);
	    auto [non_merge, merge] = unitary(TETA, PHI);

	    auto rule = step_split_merge_all(non_merge, merge);
	    auto reversed_rule = reversed_step_split_merge_all(non_merge, merge);

		for (int i = 0; i < N_ITER; ++i) {
			s->step_all(rule);
			s->reduce_all();

			sum(s);

			#ifdef TEST
				if (check(s)) {
					printf("\nstate is OK\n");
				} else
					printf("\nstate is not OK\n");
			#endif
		}

		//----
		
		printf("%ld graph of size ", s->graphs().size());
        size_stat(s);
        printf(" after %d step_merge_split()\n", N_ITER);

		for (int i = 0; i < N_ITER; ++i) {
			s->step_all(reversed_rule);
			s->reduce_all();

			sum(s);

			#ifdef TEST
				if (check(s)) {
					printf("\nstate is OK\n");
				} else
					printf("\nstate is not OK\n");
			#endif
		}

		printf("\nafter %d reversed_step_merge_split():\n", N_ITER); print(s);

		//--------------------------

		auto rule_ = step_erase_create_all(non_merge, merge);
	    auto reversed_rule_ = reversed_step_erase_create_all(non_merge, merge);

		for (int i = 0; i < N_ITER; ++i) {
			s->step_all(rule_);
			s->reduce_all();

			sum(s);

			#ifdef TEST
				if (check(s)) {
					printf("\nstate is OK\n");
				} else
					printf("\nstate is not OK\n");
			#endif
		}

		//----
		
        printf("%ld graph of size ", s->graphs().size());
        size_stat(s);
        printf(" after %d step_erase_create_all()\n", N_ITER);

		for (int i = 0; i < N_ITER; ++i) {
			s->step_all(reversed_rule_);
			s->reduce_all();

			sum(s);

			#ifdef TEST
				if (check(s)) {
					printf("\nstate is OK\n");
				} else
					printf("\nstate is not OK\n");
			#endif
		}

		printf("\nafter %d reversed_step_erase_create_all():\n", N_ITER); print(s);
	}
}
