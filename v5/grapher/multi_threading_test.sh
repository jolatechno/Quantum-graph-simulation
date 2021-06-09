#!/bin/bash

# compile
make NI=7 NG=2e5 TOL=0 use_mpfr P=192 rule_s_sm quantum_iteration

script="
for N_THREADS in 1 2 4 8 16 32 64
do
	time (OMP_NUM_THREADS=\${N_THREADS} ./quantum_iteration.out &> /dev/null) 2> time_\${N_THREADS}_threads.txt
done"

nohup bash -c """${script}""" &> /dev/null &