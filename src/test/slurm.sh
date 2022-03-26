#!/usr/bin/env bash
#SBATCH -J injectivity_test
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

echo -e "\n===== job results ====\n"

./mpi_injectivity_test.sh -p ${n_per_node} -t ${n_threads} ${args}

#end job
exit 0