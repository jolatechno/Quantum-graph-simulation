#!/bin/bash

print_usage() {
  printf "Usage: ./mpi_scaling_test.sh ops...
	-h: this help menu
	-a: arguments for scaling_test.cpp
	-m: additional arguments to mpirun/slurm
	-u: if true, uses mpirun through scaling_test.sh, otherwise uses slurm
	-M: list of module to be loaded (comma separated)
	-f: file to execute (default=\"scaling_test.out\")

	-t: list of number of threads to test (ex: 1,2,5, default:number_of_available_thread)
	-n: corresponding list of number of mpi processes per node (deafult:1)
	-N: list of number of nodes (the above list will run for EACH node, deafult:1)

	-s: additional sbatch arguments
	-o: base name of the output file (default=\"res_\")
"
}

n_threads=$(nproc --all)
n_nodes=1
n_per_nodes=1
args=
sbatch_args=
mpirun_args=
modules=
base_name="out_"
file="scaling_test.out"
use_mpi=

errfile="err.txt"

while getopts 'a:n:ht:N:s:o:f:m:M:U' flag; do
  case "$flag" in
  	h) print_usage
    	exit 1 ;;
		s) sbatch_args="${OPTARG}" ;;
	  a) args="${OPTARG}" ;;
		o) base_name="${OPTARG}" ;;
		t) n_threads="${OPTARG}" ;;
		n) n_per_nodes="${OPTARG}" ;;
		N) IFS=', ' read -r -a n_nodes <<< "${OPTARG}" ;;
		f) file="${OPTARG}" ;;
		m) mpirun_args="${OPTARG}" ;;
		M) modules="${OPTARG}" ;;
		U) use_mpi="true" ;;
	  *) print_usage
	    exit 1 ;;
  esac
done

if [ ! -d ./tmp ]; then
	mkdir tmp
fi

for n_node in "${n_nodes[@]}"; do
	use_mpi="${use_mpi}" n_per_node=${n_per_nodes} n_threads=${n_threads} rule=${args} NAME=${file} MPI_ARGS="${mpirun_args}" MODULES="${modules}" sbatch ${sbatch_args} --output=tmp/${base_name}${n_node}.out --error=tmp/${base_name}${n_node}.err -N ${n_node} slurm.sh
done