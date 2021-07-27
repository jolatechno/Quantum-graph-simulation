#!/bin/bash

print_usage() {
  printf "Usage: ./multithreading_test.sh ops...
	-h: this help menu
	-n: number of iterations (default = 2)
	-r: rule (default = 'split_merge')
	-s: initial state size (default = 12)
	-m: safety margin (default = 0.2)

	-t: starting num thread (as a power of 2)
"
}

niter="2"
size="12"
safety_margin="0.2"
rule="split_merge"
n_thread=$(nproc --all)

while getopts 'n:r:s:m:ht:' flag; do
  case "$flag" in
  	h) print_usage
       exit 1 ;;
    n) niter="${OPTARG}" ;;
		r) rule="${OPTARG}" ;;
    s) size="${OPTARG}" ;;
		m) safety_margin="${OPTARG}" ;;
		t) n_thread="${OPTARG}" ;;
    *) print_usage
       exit 1 ;;
  esac
done

command="./state_test.out -r ${rule} -n ${niter} -s ${size} --safety-margin ${safety_margin} --seed 0"
echo ""
echo ${command}

while [ $n_thread -ge 1 ]
do
	echo ""; echo "OMP_NUM_THREADS=${n_thread}"

	echo $(OMP_NUM_THREADS=${n_thread} ${command})
	n_thread=$(( $n_thread / 2))
done