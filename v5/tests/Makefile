U_CXX=$(CXX) -O3 -std=c++2a
U_CFLAGS=$(CFLAGS) -ltbb -fopenmp

TOL=5e-7

NI=5
TETA=M_PI_4
PHI=M_PI_2

P=128

targets=$(basename $(wildcard **.cpp))

all: $(targets)

test:
	$(eval U_CFLAGS+=-DTEST)

verbose:
	$(eval U_CFLAGS+=-DVERBOSE)

gprof:
	$(eval U_CFLAGS+=-pg)

#parameters

normalize:
	$(eval U_CFLAGS+=-DNORMALIZE)

use_mpfr:
	$(eval U_CFLAGS+=-DUSE_MPRF -lmpfr)

#targets

clean:
	rm *.out

libs:
	$(U_CXX) -w ../**/**.hpp

$(targets):
	$(U_CXX) $@.cpp -o$@.out $(U_CFLAGS) -DN_ITER=$(NI) -DTOL=$(TOL) -DTETA=$(TETA) -DPHI=$(PHI) -DPRECISION=$(P)
