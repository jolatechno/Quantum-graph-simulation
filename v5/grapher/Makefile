U_CXX=$(CXX) -std=c++2a
U_CFLAGS=$(CFLAGS) -ltbb -fopenmp

RULE="\"erase_create_step_split_merge_all\""
NI=20
NG=20000
TOL=5e-7

targets=$(basename $(wildcard **.cpp))

all: $(targets)

test:
	$(eval U_CFLAGS+=-DTEST)

verbose:
	$(eval U_CFLAGS+=-DVERBOSE)

gprof:
	$(eval U_CFLAGS+=-pg)

#parameters

fast_reduce:
	$(eval U_CFLAGS+=-DFAST_REDUCE)

slow_compare:
	$(eval U_CFLAGS+=-DSLOW_COMPARE)

#rules

rule_s_sm:
	$(eval RULE="\"step_split_merge_all\"")

rule_s_ec:
	$(eval RULE="\"step_erase_create_all\"")

rule_sm_s_ec:
	$(eval RULE="\"split_merge_step_erase_create_all\"")

rule_ec_s_sm:
	$(eval RULE="\"erase_create_step_split_merge_all\"")

rule_qec_s_sm:
	$(eval RULE="\"step_split_merge_all\"" -DRULE2="\"erase_create_all\"")

#targets

libs:
	$(U_CXX) -w ../**/**.hpp

$(targets):
	$(U_CXX) $@.cpp -o$@.o $(U_CFLAGS) -DRULE=$(RULE) -DN_ITER=$(NI) -DMAX_NUM_GRAPHS=$(NG) -DTOL=$(TOL)

run:
	nohup ./quantum_iteration.o > res.json 2> /dev/null &
