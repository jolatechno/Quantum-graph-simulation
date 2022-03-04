# Quantum-graph-simulation

## Implementation

__*!!!ALL the implementation relies on [v7/IQS](./v7/) found at [jolatechno/IQS](https://github.com/jolatechno/IQS)!!!*__

## Compilation

In any directory with compilable code, you can compile the code using the `make` command (the targets of make are the main file name without any extensions).

# Compilable files

...

## Usage

...

# Experiment used in the paper:

  ```bash
# to clear slurm queue
squeue -u $USER | awk '{print $1}' | tail -n+2 | xargs scancel

# !!!!!!
# command to replicate results on plafrim
# !!!!!!

# compile
module load compiler/gcc/11.2.0
module load mpi/openmpi/4.0.1
make CFLAGS="-ozonda_scaling_test.out -march=znver2" CXX=mpic++
make CFLAGS="-obora_scaling_test.out -march=skylake" CXX=mpic++



# single node scaling
./mpi_scaling.sh \
  -n 64,32,16,8,4,2,1 \
  -t 1,1,1,1,1,1,1 \
  -f zonda_scaling_test.out \
  -s " -J erase_create -C zonda --exclusive --time=0-5:00" \
  -a 7,max_num_object=2000000,seed=0\|15\|step\;erase_create -ores_ec_
./mpi_scaling.sh \
  -n 64,32,16,8,4,2,1 \
  -t 1,1,1,1,1,1,1 \
  -f zonda_scaling_test.out \
  -s " -J split_merge -C zonda --exclusive --time=0-5:00" \
  -a 8,max_num_object=20000000,seed=0\|15\|step\;split_merge -ores_sm_


# get results from single node
./csv-from-tmp.py res_ec_
./csv-from-tmp.py res_sm_


# multi-node scaling
./mpi_scaling.sh -N 41,38,35,32,29,26,23,20,18,16,14,12,10,8,6,4 \
  -n 36 -t 1 \
  -f bora_scaling_test.out \
  -s "-C bora --exclusive -J erase_create --time=0-00:5" \
  -a 9,max_num_object=-1,seed=0\|14\|step\;erase_create -oec_bora_
./mpi_scaling.sh -N 41,38,35,32,29,26,23,20,18,16,14,12,10,8,6,4,2,1 \
  -n 36 -t 1 \
  -f bora_scaling_test.out \
  -s "-C bora --exclusive -J split_merge --time=0-00:5" \
  -a 9,seed=0\|15\|step\;split_merge -osm_bora_


# get results from multi-node
./csv-from-tmp.py ec_bora_
./csv-from-tmp.py sm_bora_


# single node multi-rule stability test
./mpi_scaling.sh -n 64 -t 1 \
  -f zonda_scaling_test.out \
  -s " -J bi_rule -C zonda --exclusive --time=0-2:00" \
  -a 5,seed=0\|15\|step\;erase_create\;step\;split_merge -otest_birule_

# multi-bode multi-rule stability test
./mpi_scaling.sh -N 2,4,8,16 \
  -n 36 -t 1 \
  -f bora_scaling_test.out \
  -s "-C bora --exclusive -J bi_rule --time=0-2:00" \
  -a 5,seed=0\|15\|step\;erase_create\;step\;split_merge -otest_birule_

# multi-node stability test
./mpi_scaling.sh -N 2,4,8,16 \
  -n 36 -t 1 \
  -f bora_scaling_test.out \
  -s "-C bora --exclusive -J ec_long --time=0-2:00" \
  -a 20,seed=0\|20\|step\;erase_create -otest_long_ec_
./mpi_scaling.sh -N 2,4,8,16 \
  -n 36 -t 1 \
  -f bora_scaling_test.out \
  -s "-C bora --exclusive -J sm_long --time=0-2:00" \
  -a 20,seed=0\|30\|step\;split_merge -otest_long_sm_
```