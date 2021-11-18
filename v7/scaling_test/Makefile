U_CFLAGS=$(CFLAGS) --std=c++2a -O3 -fopenmp
path=
TBB_FLAGS=$(CFLAGS)

targets=$(basename $(wildcard **.cpp))

all: $(targets)

clean:
	rm *.out

$(targets):
	$(CXX) $@.cpp -o${path}$@.out $(U_CFLAGS)