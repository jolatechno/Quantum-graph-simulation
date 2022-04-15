#!/bin/bash

print_usage() {
  printf "Usage: ./scaling_test.sh ops...
	-h: this help menu
	-f: file to execute (default=\"scaling_test.out\")
	-a: arguments for scaling_test.cpp

	-t: list of number of threads to test (ex: 1,2,5, default:number_of_available_thread)
	-n: corresponding list of number of mpi nodes (deafult:1)
	-m: additional mpirun args
	-G: if not provided, maximum number of object in total (default: 0=\"auto-truncation\").

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
timeLimit=5
max_total_object=0

while getopts 'f:a:n:ht:m:N:G:' flag; do
  case "$flag" in
  	h) print_usage
       exit 1 ;;
    f) file="${OPTARG}" ;;
		m) mpirun_args="${OPTARG}" ;;
    a) args="${OPTARG}" ;;
		t) IFS=', ' read -r -a n_threads <<< "${OPTARG}" ;;
		n) IFS=', ' read -r -a n_nodes <<< "${OPTARG}" ;;
		N) total_n_nodes="${OPTARG}";;
		G) max_total_object="${OPTARG}";;
    *) print_usage
       exit 1 ;;
  esac
done

temp_file=$(mktemp)

num_object=$(($max_total_object / $total_n_nodes))

args_left=${args%%,*}
args_right=${args#*,}
args="${args_left},max_num_object=${num_object},${args_right}"

command="./${file} ${args}"
echo "{"
echo "  \"mpi_command\" : \"mpirun --rank-by numa --bind-to hwthread --map-by ${map_by}:PE=\$t:span -n \$n -x OMP_NUM_THREADS=\$t ${mpirun_args}\","
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
		>&2 echo -e "\n\n\n${n_thread},${n_node} (${map_by}):"

		start=`date +%s.%N`
		mpirun --bind-to hwthread --map-by ${map_by}:PE=${n_thread}:span -n ${total_n_node} ${mpirun_args} ${command} > ${temp_file}

		# delete core-dump file to free-up memory
		rm -f core.*

		exit_code=$?
		runtime=$( echo "`date +%s.%N` - $start" | bc -l )

		if [ "$exit_code" -eq 0 ]; then
			echo "$(cat ${temp_file})${separator}" | indent | indent 
			break
		fi

		if (( $(echo "$runtime > $timeLimit" |bc -l) )); then
			echo -e "\t\"total\" : $runtime\n}" | indent | indent 
			break
		fi
	done

	>&2 echo -e "\n\n\n"
done

echo "	}"
echo "}"