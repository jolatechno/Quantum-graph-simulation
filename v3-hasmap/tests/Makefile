U_CXX=$(CXX) -std=c++2a
U_CFLAGS=$(CFLAGS) -fopenmp -DTEST -DVERBOSE

targets=$(basename $(wildcard **.cpp))

all: $(targets)

$(targets):
	$(U_CXX) $@.cpp -o$@.out $(U_CFLAGS)
