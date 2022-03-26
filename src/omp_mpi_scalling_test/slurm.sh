#!/usr/bin/env bash
#SBATCH -J scaling_test
#SBATCH --time=0-2:00:00 --exclusive
#SBATCH -o %j.out
#SBATCH -e %j.err

NUM_HWTHREADS=$(lscpu -p | grep -c "^[0-9]")

echo -e "===== loading modules (${MODULES}) ====\n"

module purge
for module in ${MODULES//,/ }; do
    echo -e "loading ${module}"
    module load ${module}
done

export OMP_PROC_BIND=false
export GOMP_CPU_AFFINITY=0-${NUM_HWTHREADS}

echo -e "\n===== my job information ====\n"

echo “Node List: ” $SLURM_NODELIST
echo “Number of Nodes: ” $SLURM_JOB_NUM_NODES
echo “my jobID: ” $SLURM_JOB_ID
echo “Partition: ” $SLURM_JOB_PARTITION
echo “submit directory:” $SLURM_SUBMIT_DIR
echo “submit host:” $SLURM_SUBMIT_HOST
echo “In the directory: $PWD”
echo “As the user: $USER”

echo -e "\n“Program name: ” ${NAME}"
echo "n“Program parameters: ” ${rule}"
echo "“Number of thread list: ” ${n_threads}"
echo "“Number of task per nodes list: ” ${n_nodes}"
echo "“Srun arguments: ” ${MPI_ARGS}"

echo -e "\n===== job results ====\n"


temp_file=$(mktemp)

command="./${NAME} ${rule}"
echo "{"
echo "  \"mpi_command\" : \"srun --spread-job --cpu-bind=threads --ntasks=${total_n_node} --cpus-per-task=${n_thread} ${MPI_ARGS}\","
echo "  \"command\" : \"${command}\","
echo "  \"results\" : {"

last_element=$((${#n_threads[@]} - 1))
for i in "${!n_threads[@]}"; do
    n_thread=${n_threads[i]}
    n_node=${n_nodes[i]}
    total_n_node=$(($n_node * $SLURM_JOB_NUM_NODES))

    separator=""
    if (( $i < $last_element )); then
        separator=","
    fi

    echo "      \"${n_thread},${n_node}\" : {"
    
    for map_by in ldoms sockets none; do #hwthread core L3cache
        >&2 echo -e "\n\n\n${n_thread},${n_node} (${map_by}):"

        start=`date +%s.%N`
        if [ "$map_by" = "none" ]; then
            srun --spread-job --cpu-bind=threads --mask_cpu=${map_by} --ntasks=${total_n_node} --cpus-per-task=${n_thread} ${MPI_ARGS} ${command} > ${temp_file}
        else
            srun --spread-job --cpu-bind=threads --ntasks=${total_n_node} --cpus-per-task=${n_thread} ${MPI_ARGS} ${command} > ${temp_file}
        fi

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

echo "  }"
echo "}"

#end job
exit 0