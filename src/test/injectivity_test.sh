#!/bin/bash

print_usage() {
  printf "Usage: ./injectivity_test.sh ops...
  -h: shows this help menu
  -v: enable verbose

  -s: minimum size (default = 1)
  -S: maxium size (default = 5)

  -n: number of iterations (default = 4)

  -r: minimum random seed (default = 0)
  -R: maxium random seed (default = 100)

  -m: test split merge rule (default : test all three rules)
  -e: test erase create rule
  -c: test coin rule
"
}

min_seed=0
max_seed=100

n_iter=4

min_size=1
max_size=5

verbose=false

rule="erase_create coin split_merge"
rule_=""
overwriten_rule=false

while getopts 's:S:n:r:R:hvmec' flag; do
  case "$flag" in
  	h) print_usage
       exit 1 ;;

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

if [ "$overwriten_rule" == true ]; then
	rule="${rule_}"
fi

temp_file=$(mktemp)

for rule in $rule; do
	for size in `seq ${min_size} ${max_size}`; do
		found=false

		if [ "$verbose" == true ]; then
			echo "testing injectivity for graphs of size ${size} for ${n_iter} iterations of ${rule}..."
		fi
			
		for seed in `seq ${min_seed} ${max_seed}`; do
			command="./ping_pong_test.out ${n_iter},reversed_n_iter=${n_iter},seed=${seed}\|${size}\|step\;${rule},theta=0.25,phi=0.125,xi=-0.125 2> /dev/null"
			res=$(eval $command 2> ${temp_file})

			n_line=$(echo "${res}"  | wc -l)

			#check if there is more then one graph at the end
			if [ "${n_line}" != 3 ]; then
				echo "${command} (more than one graph !)"

				>&2 echo -e "\n\n\n\n\n${command} (more than one graph !):"
				>&2 echo -e "${res}\n\n"
				>&2 cat ${temp_file}

				found=true
			else
				first_line=$(echo "${res}" | sed -n '1p')
				third_line=$(echo "${res}" | sed -n '3p')

				#check if the graph at the end is the same as the graph at the start
				if [ "${first_line}" != "${third_line}" ]; then
					echo "${command} (graphs not equal !)"

					>&2 echo -e "\n\n\n\n\n${command} (graphs not equal !):"
					>&2 echo -e "${res}\n\n"
					>&2 cat ${temp_file}
						
					found=true
				fi
			fi
		done

		if [ "$verbose" == true ] && [ "${found}" != true ]; then
			echo "...OK"
		fi

	done
done
