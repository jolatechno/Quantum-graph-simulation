# Quantum-graph-simulation

## Implementation

__*!!!ALL the implementation relies on [v7/IQS](./v7/) found at [jolatechno/IQS](https://github.com/jolatechno/IQS)!!!*__

## Results



## Compilation

In any directory with compilable code, you can compile the code using the `make` command (the targets of make are the main file name without any extensions).

To compile for MPI (which is requiered for almost all file) `CXX=mpic++` should be added to the make command to link MPI binary library.

# Usage

## General interface

All compiled file get passed a spacial argument describing exactly the simulation. It is formated as:

_n\_iter_,_option1_=x,_option2_=y...|_graph\_size1_,_graph\_option1=x,...;_graph\_size2_,...|_rule1_,_rule1\_option1_=x...;_rule2_,...

__IMPORTANT:__ Note that the characters "|" and ";" are used by the program are sparator, but are also characters recognize by bash, so they should be backspace escaped to not be interpreted by bash.

_n\_iter_ and _graph\_size_ should be integer describing respectivly the number of iter and the initial graph size of the simulation.


### General options

The _options_ are:
 - `seed` : the random seed used to generate random objects. If not given, selected as random.
 - `reversed_n_iters` : number of iteration to do with the inverse transformation (only used in certain files, for injectivity testing, default is _n\_iter_ when used).
 - `max_num_object` : representing the maximum number of object to keep. `0` represents auto-truncation (keeping the maximum number of graph within memory limits), `-1` represent no truncation (can cause crashes when running out of memory). The default is `0`.
 - `safety_margin` : representing `iqs::safety_margin` (see the Readme from [jolatechno/IQS](https://github.com/jolatechno/IQS)).
 - `tolerance` : representing `iqs::tolerance`.
 - `truncation_tolerance` : representing `iqs::truncation_tolerance`.
 - `max_truncate_step` : representing `iqs::max_truncate_step`.
 - `min_truncate_step` : representing `iqs::min_truncate_step`.
 - `simple_truncation` : representing `iqs::simple_truncation`.
 - `load_balancing_bucket_per_thread` : representing `iqs::load_balancing_bucket_per_thread`.
 - `min_equalize_size` : representing `iqs::mpi::min_equalize_size` (only interpreted when MPI is used).
 - `equalize_inbalance` : representing `iqs::mpi::equalize_inbalance` (only interpreted when MPI is used).
 - `minimize_truncation` : representing `iqs::mpi::minimize_truncation` (only interpreted when MPI is used).

### Graph generation options

The _graph\_options_ are used to parametrize the initial state. They are as follow:
 - `n_graphs` : reprents the number of graph with a given set of option. Default is `1`.
 - `real` : represent the real part of the magnitude shared by all of the graphs with a given set of option.
 - `imag` : represent the imaginary part of the magnitude shared by all of the graphs with a given set of option.

__IMPORTANT:__ Note that the initial state is normalized after generation, so the magnitude don't have to add up to 1.

### Rules

Implemented _rules_ (all described in further details in [TO LINK](https://google.com)) are:
 - `step` : a simple `iqs::modifier` moving all particles in the same direction as their orientation.
 - `reversed_step` : inverse transformation of `step`.
 - `coin` : flip particle going left and right locally.
 - `erase_create` : exchange an empty node and a full node locally.
 - `split_merge` : exchange a full node and two nodes locally. Can create and destroy nodes.

#### Options

 For `coin`, `erase_create` and `split_merge`, the _rule\_options_ are:
  - `theta` : `theta` in the unitarty matrix, increasing the probability of interaction. Represented as a ratio of Pi, default is `0.25`.
  - `phi` : `phi` in the unitary matrix, a phase between the diagonal elements and non-diagonal terms.  Represented as a ratio of Pi, default is `0`.
  - `xi` : `xi` in the unitary matrix, a phase between the two diagonal elements.  Represented as a ratio of Pi, default is `1`.
  - `n_iter` : number of application of the rule before switching to the next rule/iteration. Default is `1`.

### Examples 

So to simulate 2 iteration of `step` followed by `erase_create` with a single starting graph with 12 nodes, and with a safety margin of `0.5`, the following argument is passed:

```bash
./executable_file 2,safety_margin=0.5\|12\|step\|erase_create
```

To simulate 2 iteration of `step` followed by `split_merge` with one starting graph with 12 nodes and 2 starting graphs with 14 nodes and a pure imaginary magnitutde, the following argument is passed:

```bash
./executable_file 2\|12\;14,n_graphs=2,imag=1,real=0\|step\|split_merge
```

To simulate 2 iteration, starting with a single graph of size 12, and applying `step` two times followed by `coin` with `theta=0.125`, the following argument is passed:

```bash
./executable_file 2\|12\|step,n_iter=2\|split_merge,theta=0.125
```


## Obtaining scaling results

...

### Slurm integration

...

### Data processing

...

## Reproducing injectivity test

...

### MPI injectivity test

...

### Slurm integration

...

## Obtain QCGD dynamic evolution

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