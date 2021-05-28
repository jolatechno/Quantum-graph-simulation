U_CXX=$(CXX) -std=c++2a
U_CFLAGS=$(CFLAGS) -fopenmp

targets=$(basename $(wildcard **.cpp))

all: $(targets)

test:
	$(eval U_CFLAGS+=-DTEST)

test-full: test
	$(eval U_CFLAGS+=-DFULL)

verbose:
	$(eval U_CFLAGS+=-DVERBOSE)

$(targets):
	$(U_CXX) $@.cpp -o$@.out $(U_CFLAGS)
