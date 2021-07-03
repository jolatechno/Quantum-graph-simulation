#!/bin/bash

print_usage() {
  printf "Usage: ./multithreading_test.sh ops...
	-h: this help menu
	-n: number of iterations (default = 2)
	-s: initial state size (default = 12)
	-g: maximum number of graph (default = -1 <=> inf)

	-t: starting num thread (as a power of 2)
"
}

niter="2"
size="12"
max_n_graphs="-1"
n_thread=$(nproc --all)

while getopts 'n:s:g:ht:' flag; do
  case "$flag" in
  	h) print_usage
       exit 1 ;;
    n) niter="${OPTARG}" ;;
    s) size="${OPTARG}" ;;
		g) max_n_graphs="${OPTARG}" ;;
		t) n_thread="${OPTARG}" ;;
    *) print_usage
       exit 1 ;;
  esac
done

command="./state_test.out -r split_merge -n ${niter} -s ${size} -g ${max_n_graphs} -T 1e-18 --seed 26"
echo "" >> time.txt
echo ${command} >> time.txt

while [ $n_thread -ge 1 ]
do
	echo "" >> time.txt; echo "OMP_NUM_THREADS=${n_thread}" >> time.txt

	time (OMP_NUM_THREADS=${n_thread} ${command}) 2>> time.txt >> /dev/null
	n_thread=$(( $n_thread / 2))
done