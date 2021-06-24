#!/bin/bash

min_seed=0
max_seed=200

min_n_iter=1
max_n_iter=5

min_size=1
max_size=7

found="false"

for n_iter in `seq ${min_n_iter} ${max_n_iter}`; do
	for size in `seq ${min_size} ${max_size}`; do
		for seed in `seq ${min_seed} ${max_seed}`; do
			n_line=$(./state_test.out -r split_merge_all -T 1e-18 -i --seed "${seed}" -n "${n_iter}"  -s "${size}" | wc -l)

			if [ "$n_line" -gt 3 ]; then
				echo "--seed ${seed} -n ${n_iter} -s ${size}"
				found="true"
			fi
		done

		if [ "${found}" = "true" ]; then
			exit 0
		fi
	done
done

echo "Ok"