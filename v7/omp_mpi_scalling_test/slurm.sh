#!/usr/bin/env bash
#SBATCH -J scaling_test
#SBATCH --time=0-2:00:00 --exclusive
#SBATCH -N 1 -C zonda
#SBATCH -o %j.out
#SBATCH -e %j.err

module load compiler/gcc/11.2.0
export OMP_PROC_BIND=false
export GOMP_CPU_AFFINITY=0-63
module load mpi/openmpi/3.1.4

echo -e "===== my job information ====\n"

echo “Node List: ” $SLURM_NODELIST
echo “my jobID: ” $SLURM_JOB_ID
echo “Partition: ” $SLURM_JOB_PARTITION
echo “submit directory:” $SLURM_SUBMIT_DIR
echo “submit host:” $SLURM_SUBMIT_HOST
echo “In the directory: $PWD”
echo “As the user: $USER”

echo -e "\n===== job compilation ====\n"

cd /home/jtouzet/Quantum-graph-simulation/v7/omp_mpi_scalling_test
#make CXX=mpic++ CFLAGS="${CFLAGS}"

echo -e "\n===== job results ====\n"

./scaling_test.sh -n ${n_per_node} -t ${n_threads} -a ${rule}

#end job
exit 0
