#!/bin/bash

print_usage() {
  printf "Usage: ./multithreading_test.sh ops...
	-h: this help menu
	-n: number of iterations (default = 2)
	-r: rule (default = 'split_merge')
	-s: initial state size (default = 12)
	-m: safety margin (default = 0.2)
	-a: additional arguments

	-t: starting num thread (as a power of 2)
"
}

niter="2"
size="12"
safety_margin="0.2"
rule="split_merge"
n_thread=$(nproc --all)
args=""

errfile="err.txt"

while getopts 'n:r:s:m:a:ht:' flag; do
  case "$flag" in
  	h) print_usage
       exit 1 ;;
    n) niter="${OPTARG}" ;;
		r) rule="${OPTARG}" ;;
    s) size="${OPTARG}" ;;
		m) safety_margin="${OPTARG}" ;;
		a) args="${OPTARG}" ;;
		t) n_thread="${OPTARG}" ;;
    *) print_usage
       exit 1 ;;
  esac
done

command="./state_test.out -v 1 -r ${rule} -n ${niter} -s ${size} --safety-margin ${safety_margin} --seed 0 ${args}"
echo "{"
echo "	\"command\" : \"${command}\","
echo "	\"results\" : {"

while [ $n_thread -ge 1 ]
do
	separator=""
	if [ $n_thread -gt 1 ]; then
		separator=","
	fi
	
	#cpu_cores="$(seq -s ',' 0 $(($n_thread - 1)))"
	#echo "		\"${n_thread}\" :	$(numactl --physcpubind="${cpu_cores}" ${command})${separator}"
	echo "${n_thread}:" >> ${errfile}
	echo "		\"${n_thread}\" :	$(OMP_NUM_THREADS="${n_thread}" ${command} 2>> ${errfile})${separator}"
	echo "" >> ${errfile}

	n_thread=$(( $n_thread / 2))
done

echo "	}"
echo "}"