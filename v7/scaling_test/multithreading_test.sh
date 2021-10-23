#!/bin/bash

print_usage() {
  printf "Usage: ./multithreading_test.sh ops...
	-h: this help menu
	-a: arguments for scaling_test.cpp
	-o: use OMP_NUM_THREADS rather than numactl

	-t: list of number of threads to test (ex: 1,2,5, default:number_of_available_thread)
"
}

indent() { sed 's/^/	/'; }

n_threads=$(nproc --all)
args=""
use_omp=false

errfile="err.txt"

while getopts 'a:oht:' flag; do
  case "$flag" in
  	h) print_usage
       exit 1 ;;
    a) args="${OPTARG}" ;;
	o) use_omp=true;;
	t) n_threads="${OPTARG}" ;;
    *) print_usage
       exit 1 ;;
  esac
done

command="./scaling_test.out ${args}"
echo "{"
#echo "	\"rule\" : \"${rule}\","
echo "	\"command\" : \"${command}\","
#echo "	\"memory_bandwidth\" : $(./memory_test.out 2>> ${errfile} | indent),"
echo "	\"results\" : {"

echo "" > ${errfile}

for n_thread in ${n_threads//,/ }
do
	separator=""
	if [ $n_thread -gt 1 ]; then
		separator=","
	fi

	echo "${n_thread}:" >> ${errfile}
	echo "		\"${n_thread}\" : {"
	
	if ${use_omp}; then
		echo "$(OMP_NUM_THREADS="${n_thread}" ${command} 2>> ${errfile} | indent | indent)${separator}"
	else
		cpu_cores="$(seq -s ',' 0 $(($n_thread - 1)))"
		echo "$(OMP_PROC_BIND=true  numactl --physcpubind="${cpu_cores}" ${command} | indent | indent)${separator}"
	fi
	
	echo "" >> ${errfile}
done

echo "	}"
echo "}"