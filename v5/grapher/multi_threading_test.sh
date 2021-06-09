#!/bin/bash

command="./quantum_iteration.out -n 500 -s 8 -r step_split_merge_all -T 1e-20 -g 2000000 -P 256 > res_\${N_THREADS}.json 2> /dev/null"
script="
for N_THREADS in 1 2 4 8 16 32 64
do
	time (OMP_NUM_THREADS=\${N_THREADS} ${command}) 2> time_\${N_THREADS}_threads.txt
	sleep 1
done"

nohup bash -c """${script}""" &> /dev/null &