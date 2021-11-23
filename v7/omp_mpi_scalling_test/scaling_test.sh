#!/bin/bash

print_usage() {
  printf "Usage: ./scaling_test.sh ops...
	-h: this help menu
	-f: file to execute (default=\"scaling_test.out\")
	-a: arguments for scaling_test.cpp

	-t: list of number of threads to test (ex: 1,2,5, default:number_of_available_thread)
	-n: corresponding list of number of mpi nodes (deafult:1)
	-m: additional mpirun args
"
}

indent() { sed 's/^/	/'; }

n_threads=$(nproc --all)
n_nodes=1
args=
file="scaling_test.out"
mpirun_args=

while getopts 'f:a:n:ht:m:' flag; do
  case "$flag" in
  	h) print_usage
       exit 1 ;;
    f) file="${OPTARG}" ;;
		m) mpirun_args="${OPTARG}" ;;
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

last_element=$((${#n_threads[@]} - 1))
for i in "${!n_threads[@]}"; do
	n_thread=${n_threads[i]}
	n_node=${n_nodes[i]}

	separator=""
	if (( $i < $last_element )); then
		separator=","
	fi

	>&2 echo "${n_thread},${n_node}:"
	echo "		\"${n_thread},${n_node}\" : {"
	
	mpirun --quiet --rank-by numa --bind-to hwthread --map-by ppr:${n_node}:node:PE=${n_thread} --report-bindings -x OMP_NUM_THREADS=${n_thread} ${mpirun_args} ${command} 2>&2 | indent | indent 
	echo ${separator}
	
	>&2 echo -e "\n\n\n"
done

echo "	}"
echo "}"