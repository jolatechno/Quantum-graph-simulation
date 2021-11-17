U_CFLAGS=$(CFLAGS) -O3 -fopenmp -ltbb
path=
TBB_FLAGS=$(CFLAGS)

targets=$(basename $(wildcard **.cpp))

all: $(targets)

clean:
	rm *.out

MAKE_TBB:
	cd ../../; \
	./tbb-install.sh -p "${path}" -c ${TBB_FLAGS}

USE_TBB:
	$(eval U_CFLAGS+= $(shell ../../get-tbb-cflags.sh -p "${path}"))

$(targets):
	$(CXX) $@.cpp -o${path}$@.out $(U_CFLAGS)