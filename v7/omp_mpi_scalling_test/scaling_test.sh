#!/bin/bash

print_usage() {
  printf "Usage: ./multithreading_test.sh ops...
	-h: this help menu
	-f: file to execute (default=\"scaling_test.out\")
	-a: arguments for scaling_test.cpp

	-t: list of number of threads to test (ex: 1,2,5, default:number_of_available_thread)
	-n: corresponding list of number of mpi nodes (deafult:1)
"
}

indent() { sed 's/^/	/'; }

n_threads=$(nproc --all)
n_nodes=1
args=""
use_omp=false
file="scaling_test.out"

errfile="err.txt"

while getopts 'f:a:n:ht:' flag; do
  case "$flag" in
  	h) print_usage
       exit 1 ;;
    f) file="${OPTARG}" ;;
    a) args="${OPTARG}" ;;
		t) IFS=', ' read -r -a n_threads <<< "${OPTARG}" ;;
		n) IFS=', ' read -r -a n_nodes <<< "${OPTARG}" ;;
    *) print_usage
       exit 1 ;;
  esac
done

command="${file} ${args}"
echo "{"
echo "	\"command\" : \"${command}\","
echo "	\"results\" : {"

echo "" > ${errfile}

for i in "${!n_threads[@]}"; do
	n_thread=${n_threads[i]}
	n_node=${n_nodes[i]}

	separator=""
	if [ $n_thread -gt 1 ]; then
		separator=","
	fi

	echo "${n_thread},${n_node}:" >> ${errfile}
	echo "		\"${n_thread},${n_node}\" : {"
	
	
		echo "$(mpirun --oversubscribe --cpus-per-proc ${n_thread}  -n ${n_node} -x OMP_NUM_THREADS=${n_thread} ${command} 2>> ${errfile} | indent | indent)${separator}"
	
	echo "" >> ${errfile}
done

echo "	}"
echo "}"