#!/bin/bash

print_usage() {
  printf "Usage: ./multithreading_test.sh ops...
	-h: this help menu
	-n: number of iterations (default = 2)
	-s: initial state size (default = 12)
	-g: maximum number of graph (default = -1 <=> inf)
"
}

niter="2"
size="12"
max_n_graphs="-1"

while getopts 'n:s:g:h' flag; do
  case "$flag" in
  	h) print_usage
       exit 1 ;;
    n) niter="${OPTARG}" ;;
    s) size="${OPTARG}" ;;
		g) max_n_graphs="${OPTARG}" ;;
    *) print_usage
       exit 1 ;;
  esac
done

command="./state_test.out -r split_merge --rule2 erase_create -n ${niter} -s ${size} -g ${max_n_graphs} -T 1e-18 --seed 26"
echo "" >> time.txt
echo ${command} >> time.txt

script="N_THREADS=$(nproc --all)
while [ \$N_THREADS -ge 1 ]
do
	echo \"\" >> time.txt
	echo OMP_NUM_THREADS=\${N_THREADS} >> time.txt

	time OMP_NUM_THREADS=\${N_THREADS} ${command} 2>> time.txt
	N_THREADS=\$(( \$N_THREADS / 2))
done"

nohup bash -c "${script}" &> /dev/null &