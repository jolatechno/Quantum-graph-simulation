#!/bin/bash

print_usage() {
  printf "Usage: ./multithreading_test.sh ops...
	-h: this help menu
	-n: number of iterations (default = 2)
	-r: rule (default = 'split_merge')
	-s: initial state size (default = 12)
	-m: safety margin (default = 0.2)
	-a: additional arguments
	-o: use OMP_NUM_THREADS rather than numactl

	-t: list of number of threads to test (ex: 1,2,5, default:number_of_available_thread)
"
}

indent() { sed 's/^/	/'; }

niter=2
size=12
safety_margin=0.2
rule="split_merge"
n_threads=$(nproc --all)
args=""
use_omp=false

errfile="err.txt"

while getopts 'n:r:s:m:a:oht:' flag; do
  case "$flag" in
  	h) print_usage
       exit 1 ;;
    n) niter="${OPTARG}" ;;
		r) rule="${OPTARG}" ;;
    s) size="${OPTARG}" ;;
		m) safety_margin="${OPTARG}" ;;
		a) args="${OPTARG}" ;;
		o) use_omp=true;;
		t) n_threads="${OPTARG}" ;;
    *) print_usage
       exit 1 ;;
  esac
done

command="./scaling_test.out -r ${rule} -n ${niter} -s ${size} --safety-margin ${safety_margin} --seed 0 ${args}"
echo "{"
echo "	\"rule\" : \"${rule}\","
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