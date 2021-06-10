#!/bin/bash

command="./quantum_iteration.out -n 500 -s 8 -r step_split_merge_all -N -T 0 -g 1000000 -P 256 --seed 123 > res_\${N_THREADS}.json 2> /dev/null"
script="
for N_THREADS in 64 32 16 8 4 2 1
do
	time (OMP_NUM_THREADS=\${N_THREADS} ${command}) 2> time_\${N_THREADS}_threads.txt
	sleep 1
done"

nohup bash -c """${script}""" &> /dev/null &