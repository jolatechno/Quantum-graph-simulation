#!/bin/bash

print_usage() {
  printf "Usage: ./multithreading_test.sh ops...
	-h: this help menu
	-n: number of iterations (default = 2)
	-s: initial state size (default = 12)
	-g: maximum number of graph (default = -1 <=> inf)"
}

niter="2"
size="12"

while getopts 'n:s:h' flag; do
  case "$flag" in
  	h) print_usage
       exit 1 ;;
    n) niter="${OPTARG}" ;;
    s) size="${OPTARG}" ;;
    *) print_usage
       exit 1 ;;
  esac
done

command="./state_test.out -i -n ${niter} -s ${size} -T 1e-18 --seed 26"
echo ${command} >> time.txt

script="
for N_THREADS in 64 32 16 8 4 2 1
do
	echo OMP_NUM_THREADS=\${N_THREADS} >> time.txt
	time (OMP_NUM_THREADS=\${N_THREADS} ${command}) 2>> time.txt
	sleep 1
done"

nohup bash -c "${script}" &> /dev/null &