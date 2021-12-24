#!/bin/bash

print_usage() {
  printf "Usage: ./scaling_test.sh ops...
	-h: this help menu
	-f: file to execute (default=\"scaling_test.out\")
	-a: arguments for scaling_test.cpp

	-t: list of number of threads to test (ex: 1,2,5, default:number_of_available_thread)
	-n: corresponding list of number of mpi nodes (deafult:1)
	-m: additional mpirun args

	-N: total number of node
"
}

indent() { sed 's/^/	/'; }

n_threads=$(nproc --all)
total_n_nodes=1
n_nodes=1
args=
file="scaling_test.out"
mpirun_args=

while getopts 'f:a:n:ht:m:N:' flag; do
  case "$flag" in
  	h) print_usage
       exit 1 ;;
    f) file="${OPTARG}" ;;
		m) mpirun_args="${OPTARG}" ;;
    a) args="${OPTARG}" ;;
		t) IFS=', ' read -r -a n_threads <<< "${OPTARG}" ;;
		n) IFS=', ' read -r -a n_nodes <<< "${OPTARG}" ;;
		N) total_n_nodes="${OPTARG}";;
    *) print_usage
       exit 1 ;;
  esac
done

temp_file=$(mktemp)

command="${file} ${args}"
echo "{"
echo "	\"command\" : \"${command}\","
echo "	\"results\" : {"

last_element=$((${#n_threads[@]} - 1))
for i in "${!n_threads[@]}"; do
	n_thread=${n_threads[i]}
	n_node=${n_nodes[i]}
	total_n_node=$(($n_node * $total_n_nodes))

	separator=""
	if (( $i < $last_element )); then
		separator=","
	fi

	echo "		\"${n_thread},${n_node}\" : {"
	
	for map_by in numa socket node; do #hwthread core L3cache
		res=$(mpirun --rank-by numa --bind-to hwthread --map-by ${map_by}:PE=${n_thread}:span -n ${total_n_node} --report-bindings -x OMP_NUM_THREADS=${n_thread} ${mpirun_args} ${command} 2> ${temp_file})
		if [ $? -eq 0 ]; then
			echo "${res}${separator}" | indent | indent 

			>&2 echo "${n_thread},${n_node} (${map_by}):"
			>&2 cat ${temp_file}

			break
		fi
	done

	>&2 echo -e "\n\n\n"
done

echo "	}"
echo "}"