#!/bin/bash

print_usage() {
  printf "Usage: ./mpi_injectivity_test.sh ops...
  -t: numer of thread per task (default = 1)
  -p: number of task (default = 1)

	-s: minimum size (default = 1)
	-S: maxium size (default = 5)

	-n: number of iterations (default = 4)

	-r: minimum random seed (default = 0)
	-R: maxium random seed (default = 10)

	-m: test split merge rule (default : test all three rules)
	-e: test erase create rule
	-c: test coin rule

	-v: enable verbose
"
}

min_seed=0
max_seed=10

n_iter=4

min_size=1
max_size=5

verbose=false

rule="erase_create coin split_merge"
rule_=""
overwriten_rule=false
n_thread="1"
n_node="1"

while getopts 's:S:n:r:R:hvmect:p:' flag; do
  case "$flag" in
  	h) print_usage
       exit 1 ;;

    t) n_thread="${OPTARG}" ;;
		p) n_node="${OPTARG}" ;;

    n) n_iter="${OPTARG}" ;;

		s) min_size="${OPTARG}" ;;
    S) max_size="${OPTARG}" ;;

		r) min_seed="${OPTARG}" ;;
    R) max_seed="${OPTARG}" ;;

		v) verbose=true ;;
		
		m) rule_+=" split_merge"; overwriten_rule=true;;
		e) rule_+=" erase_create"; overwriten_rule=true;;
		c) rule_+=" coin"; overwriten_rule=true;;

    *) print_usage
       exit 1 ;;
  esac
done


runner="mpirun --quiet --rank-by numa --bind-to hwthread --map-by ppr:${n_node}:node:PE=${n_thread} -x OMP_NUM_THREADS=${n_thread}"

if [ "$overwriten_rule" == true ]; then
	rule="${rule_}"
fi

if [ "$verbose" == true ]; then
	echo "runner is : ${runner}"
fi

temp_file=$(mktemp)

for rule in $rule; do
	for size in `seq ${min_size} ${max_size}`; do
		found=false

		if [ "$verbose" == true ]; then
			echo "testing injectivity for graphs of size ${size} for ${n_iter} iterations of ${rule}..."
		fi
			
		for seed in `seq ${min_seed} ${max_seed}`; do
			command="mpi_ping_pong_test.out ${n_iter},reversed_n_iter=${n_iter},seed=${seed}\|${size}\|step\;${rule},theta=0.25,phi=0.125,xi=-0.125"
			res=$(eval $runner $command 2> ${temp_file})

			n_line=$(echo "${res}" | wc -l)

			first_line=$(echo "${res}" | sed -n '2p')
			last_line=$(echo "${res}" | sed -n "${n_line}p")

			#check if the graph at the end is the same as the graph at the start
			if [ "${first_line}" != "${last_line}" ]; then
				echo "${command} (graphs not equal !)"

				>&2 echo -e "\n\n\n\n\n${command} (graphs not equal !):"
				>&2 echo -e "${res}\n\n"
				>&2 cat ${temp_file}

				found=true
			fi
		done

		if [ "$verbose" == true ] && [ "${found}" != true ]; then
			echo "...OK"
		fi

	done
done
