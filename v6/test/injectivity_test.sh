#!/bin/bash -e

print_usage() {
  printf "Usage: ./injectivity_test.sh ops...
	-s: minimum size (default = 1)
	-S: maxium size (default = 5)

	-n: minimum number of iterations (default = 4)
	-N: maxium number of iterations (default = 4)

	-r: minimum random seed (default = 0)
	-R: maxium random seed (default = 100)
"
}

min_seed=0
max_seed=100

min_n_iter=4
max_n_iter=4

min_size=1
max_size=5

while getopts 's:S:n:N:r:R:h' flag; do
  case "$flag" in
  	h) print_usage
       exit 1 ;;

    n) min_n_iter="${OPTARG}" ;;
    N) max_n_iter="${OPTARG}" ;;

		s) min_size="${OPTARG}" ;;
    S) max_size="${OPTARG}" ;;

		r) min_seed="${OPTARG}" ;;
    R) max_seed="${OPTARG}" ;;

    *) print_usage
       exit 1 ;;
  esac
done

found="false"

for n_iter in `seq ${min_n_iter} ${max_n_iter}`; do
	for size in `seq ${min_size} ${max_size}`; do
		echo "testing injectivity fro graphs of size ${size} for ${n_iter} iterations..."
		for seed in `seq ${min_seed} ${max_seed}`; do
			res=$(./state_test.out -r split_merge_all -T 1e-18 -i --seed "${seed}" -n "${n_iter}"  -s "${size}")

			#check if there is more then one graph at the end
			n_line=$(echo "${res}"  | wc -l)
			if [ "${n_line}" != 3 ]; then
				echo "--seed ${seed} -n ${n_iter} -s ${size} (more than one graph !)"
				found="true"
			fi

			#check if the graph at the end is the same as the graph at the start
			first_line=$(echo "${res}" | sed -n '1p')
			third_line=$(echo "${res}" | sed -n '3p')
			if [ "${first_line}" != "${third_line}" ]; then
				echo "--seed ${seed} -n ${n_iter} -s ${size} (graphs not equal !)"
				found="true"
			fi
		done

		if [ "${found}" = "true" ]; then
			exit 0
		fi

		echo "...Ok"
		echo
	done
done