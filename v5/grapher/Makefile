U_CXX=$(CXX) -std=c++2a
U_CFLAGS=$(CFLAGS) -ltbb -fopenmp

NI=20
NG=20000

TOL=5e-7

TETA=M_PI_4
PHI=M_PI_2

SIZE=8
DSIZE=3
N_GRAPHS=3

targets=$(basename $(wildcard **.cpp))

all: $(targets)

test:
	$(eval U_CFLAGS+=-DTEST)

verbose:
	$(eval U_CFLAGS+=-DVERBOSE)

gprof:
	$(eval U_CFLAGS+=-pg)

#starting state

zero_randomize:
	$(eval U_CFLAGS+=-DZERO_RANDOMIZE=true)

randomize:
	$(eval U_CFLAGS+=-DRANDOMIZE=true)

#parameters

slow_reduce:
	$(eval U_CFLAGS+=-DSLOW_REDUCE)

slow_compare:
	$(eval U_CFLAGS+=-DSLOW_COMPARE)

normalize:
	$(eval U_CFLAGS+=-DNORMALIZE)

#rules

rule_s_sm:
	$(eval U_CFLAGS+=-DRULE="\"step_split_merge_all\"")

rule_s_ec:
	$(eval U_CFLAGS+=-DRULE="\"step_erase_create_all\"")

rule_s_sm_ec:
	$(eval U_CFLAGS+=-DRULE="\"step_split_merge_all\"" -DRULE2="\"erase_create_all\"")

rule_s_ec_sm:
	$(eval U_CFLAGS+=-DRULE="\"step_erase_create_all\"" -DRULE2="\"split_merge_all\"")

rule_s_ec_s_sm:
	$(eval U_CFLAGS+=-DRULE="\"step_split_merge_all\"" -DRULE2="\"step_erase_create_all\"")

#targets

libs:
	$(U_CXX) -w ../**/**.hpp

$(targets):
	$(U_CXX) $@.cpp -o$@.o $(U_CFLAGS) -DN_ITER=$(NI) -DMAX_NUM_GRAPHS=$(NG) -DSIZE=$(SIZE) -DTOL=$(TOL) -DTETA=$(TETA) -DPHI=$(PHI)

run:
	nohup bash -c "time ( ./quantum_iteration.o 2> err.txt > res.json ) 2> time.txt"  2> /dev/null &
