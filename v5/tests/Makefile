U_CXX=$(CXX) -std=c++2a
U_CFLAGS=$(CFLAGS) -ltbb -fopenmp

TOL=5e-7

NI=5
TETA=M_PI_4
PHI=M_PI_2

targets=$(basename $(wildcard **.cpp))

all: $(targets)

test:
	$(eval U_CFLAGS+=-DTEST)

verbose:
	$(eval U_CFLAGS+=-DVERBOSE)

gprof:
	$(eval U_CFLAGS+=-pg)

#parameters

slow_reduce:
	$(eval U_CFLAGS+=-DSLOW_REDUCE)

slow_compare:
	$(eval U_CFLAGS+=-DSLOW_COMPARE)

normalize:
	$(eval U_CFLAGS+=-DNORMALIZE)

#targets

libs:
	$(U_CXX) -w ../**/**.hpp

$(targets):
	$(U_CXX) $@.cpp -o$@.o $(U_CFLAGS) -DN_ITER=$(NI) -DTOL=$(TOL) -DTETA=$(TETA) -DPHI=$(PHI)
