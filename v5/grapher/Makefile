U_CXX=$(CXX) -O3 -std=c++2a
U_CFLAGS=$(CFLAGS) -ltbb -fopenmp

targets=$(basename $(wildcard **.cpp))

all: $(targets)

test:
	$(eval U_CFLAGS+=-DTEST)

verbose:
	$(eval U_CFLAGS+=-DVERBOSE)

gprof:
	$(eval U_CFLAGS+=-pg)

#parameters

use_mpfr:
	$(eval U_CFLAGS+=-DUSE_MPRF -lmpfr)

#targets

clean:
	rm *.out

libs:
	$(U_CXX) -w ../**/**.hpp

$(targets):
	$(U_CXX) $@.cpp -o$@.out $(U_CFLAGS)

run:
	nohup bash -c "time ( ./quantum_iteration.out 2> err.txt > res.json ) 2> time.txt"  2> /dev/null &
