#!/usr/bin/env bash
#SBATCH -J scaling_test
#SBATCH --time=0-2:00:00 --exclusive
#SBATCH -o %j.out
#SBATCH -e %j.err

NUM_HWTHREADS=$(lscpu -p | grep -c "^[0-9]")

for module in ${${MODULES}//,/ }; do
    module load $module
done

export OMP_PROC_BIND=false
export GOMP_CPU_AFFINITY=0-${NUM_HWTHREADS}

echo -e "===== my job information ====\n"

echo “Node List: ” $SLURM_NODELIST
echo “Number of Nodes: ” $SLURM_JOB_NUM_NODES
echo “my jobID: ” $SLURM_JOB_ID
echo “Partition: ” $SLURM_JOB_PARTITION
echo “submit directory:” $SLURM_SUBMIT_DIR
echo “submit host:” $SLURM_SUBMIT_HOST
echo “In the directory: $PWD”
echo “As the user: $USER”

echo -e "\n===== job results ====\n"

temp_file=$(mktemp)

command="${file} ${args}"
echo "{"
echo "  \"mpi_command\" : \"srun --cpus-per-task=${n_thread} --task=${n_node} ${mpirun_args}\","
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

	echo "		\"${n_thread},${n_node}\" : {"
	
	for map_by in ldoms sockets boards; do #hwthread core L3cache
		>&2 echo -e "\n\n\n${n_thread},${n_node} (${map_by}):"

		start=`date +%s.%N`
		srun --cpu-bin=${map_by} --cpus-per-task=${n_thread} --task=${n_node} ${mpirun_args} ${command} > ${temp_file}

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


#./scaling_test.sh -m "${MPI_ARGS}" -N ${SLURM_JOB_NUM_NODES} -n ${n_per_node} -t ${n_threads} -a ${rule} -f ${NAME}

#end job
exit 0