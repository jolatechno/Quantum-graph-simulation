U_CXX=$(CXX) -std=c++2a
U_CFLAGS=$(CFLAGS) -DTBB_PREVIEW_CONCURRENT_ORDERED_CONTAINERS -ltbb -fopenmp

targets=$(basename $(wildcard **.cpp))

all: $(targets)

test:
	$(eval U_CFLAGS+=-DTEST)

verbose:
	$(eval U_CFLAGS+=-DVERBOSE)

gprof:
	$(eval U_CFLAGS+=-pg)

libs:
	$(U_CXX) -w ../**/**.hpp

$(targets):
	$(U_CXX) $@.cpp -o$@.o $(U_CFLAGS)
