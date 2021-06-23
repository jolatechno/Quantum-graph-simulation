U_CXX=$(CXX) -O3 -std=c++2a
U_CFLAGS=$(CFLAGS) -fopenmp

targets=$(basename $(wildcard **.cpp))

use_mpfr:
	$(eval U_CFLAGS+=-DUSE_MPRF -lmpfr)

clean:
	rm *.out ../**/**.gch ../**/**/**.gch

all: $(targets)

libs:
	$(U_CXX) $(U_CFLAGS) -w ../**/**.hpp ../**/**/**.hpp

$(targets):
	$(U_CXX) $@.cpp -o$@.out $(U_CFLAGS)