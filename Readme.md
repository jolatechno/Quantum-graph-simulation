# Quantum-graph-simulation

## Implementation

__*!!!ALL the implementation relies on [src/IQS](./src/) found at [jolatechno/IQS](https://github.com/jolatechno/IQS)!!!*__

## Results

Scaling results are in the [jolatechno/Quantum-graph-simulation-plots](https://github.com/jolatechno/Quantum-graph-simulation-plots) sub-repository.

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
./executable_file 2,safety_margin=0.5\|12\|step\;erase_create
```

To simulate 2 iteration of `step` followed by `split_merge` with one starting graph with 12 nodes and 2 starting graphs with 14 nodes and a pure imaginary magnitutde, the following argument is passed:

```bash
./executable_file 2\|12\;14,n_graphs=2,imag=1,real=0\|step\;split_merge
```

To simulate 2 iteration, starting with a single graph of size 12, and applying `step` two times followed by `coin` with `theta=0.125`, the following argument is passed:

```bash
./executable_file 2\|12\|step,n_iter=2\;split_merge,theta=0.125
```

## Running a simple test

The [src/test/ping_pong_test.ccp](./src/test/) file (later used for injectivity test) simply print the intial state, and after running the simulation (including the inverse transformation) will print the final state. If you don't wat to apply the reverse transformation you can simply pass `reversed_n_iters=0`.

After compiling it using `make ping_pong_test`, you can run it, for example for 4 iterations of `step` followed by `split_merge` starting with a single graph of size 12 using the following command:

```bash
./pin_pong_test.out 4,reversed_n_iters=0\|12\|step\;split_merge
```

### Using MPI

The [src/test/mpi_ping_pong_test.ccp](./src/test/) file has the same function as [src/test/ping_pong_test.ccp](./src/test/), but support MPI. Since it gather the final state on the main rank at the end before printing it, if their is too much object by this time to fit in memory of a single node, the program will crash.

After compiling it using `make CXX=mpic++ mpi_ping_pong_test`, you can run it, for example with 8 processes (see [mpirun(1) man page](https://www.open-mpi.org/doc/v4.0/man1/mpirun.1.php) for more info on mpirun), for 4 iterations of `step` followed by `split_merge` starting with a single graph of size 12 using the following command:

```bash
mpirun -n 8 mpi_ping_pong_test.out 4,reversed_n_iters=0\|12\|step\;split_merge
```

## Obtaining scaling results

The compilable file used to obtain scaling results is [src/omp_mpi_scalling_test/scaling_test.cpp](./src/omp_mpi_scalling_test/). After compiling it using `make CXX=mpic++`, actual scaling results are obtained using the [src/omp_mpi_scalling_test/scaling_test.sh](./src/omp_mpi_scalling_test/) script, which get passed the following arguments:
 - `-h`: simple help infos.
 - `-f`: compiled file used, default is `scaling_test.out`.
 - `-a`: argument passed to `scaling_test.out`, in the form described above (_n\_iter_\|_graph\_size_\|_rules_).
 - `-t`: list of the number of threads to test (ex: `1,2,6` the default is the number_of available threads).
 - `-n`: list of the number of mpi rank to spawn per node (ex: `6,3,1` default is `1`).
 - `-m`: additionals arguments for `mpirun`.
 - `-N`: total number of nodes used for MPI, default is `1`.

For example, to test the scaling on 5 nodes for 1 rank times 6 threads, 3 ranks time 2 threads, and 6 ranks times 1 threads for 3 iteration of `step` followed by `erase_create` starting with a single graph of 12 nodes, the command will be:

```bash
./scaling_test.sh -N 5 -t 6,2,1 -n 1,3,6 -a 4,reversed_n_iters=0\|12\|step\;split_merge
```

Note that the output (after separating `stderr` from `stdout`) will be formated as a json.

### Slurm integration

To obtain scaling results for different number of nodes, using slurm [src/omp_mpi_scalling_test/mpi_scaling.sh](./src/omp_mpi_scalling_test/) is used (which simply calls [src/omp_mpi_scalling_test/scaling_test.sh](./src/omp_mpi_scalling_test/) script, and stores the results in `src/omp_mpi_scalling_test/tmp`). It get passed the following arguments:
 - `-h`: simple help infos.
 - `-a`: argument passed to `scaling_test.out`, similar to `-a` for `scaling_test.sh`.
 - `-f`, `_t` and `-n`: same as `scaling_test.sh`.
 - `-N`: list of number of nodes to ask from sbatch (example `1,2,4` default is `1`).
 - `-s`: additional arguments to pass to sbatch (to ask for specific nodes for example).
 - `-o`: base name of the output files (default is `res_`, so the results for _n_ ranks will be _res\_n.out_ and _res\_n.err_).

For example, to test the scaling on 1,2,4 and 6 nodes for 1 rank times 6 threads, 3 ranks time 2 threads, and 6 ranks times 1 threads for 3 iteration of `step` followed by `erase_create` starting with a single graph of 12 nodes, the command will be:

```bash
./mpi_scaling.sh -N 1,2,4,6 -t 6,2,1 -n 1,3,6 -a 4,reversed_n_iters=0\|12\|step\;split_merge
```

#### Data processing

The [src/omp_mpi_scalling_test/csv-from-tmp.py](./src/omp_mpi_scalling_test/) script (requiering python 3) simply takes a base name (`-o` argument of [src/omp_mpi_scalling_test/mpi_scaling.sh](./src/omp_mpi_scalling_test/)) and returns a csv formated compilation of the results obtained by using [src/omp_mpi_scalling_test/mpi_scaling.sh](./src/omp_mpi_scalling_test/).

## Reproducing injectivity test

Injectivity testing for multiple graphs is done using [src/test/injectivity_test.sh](./src/test/) script (relying on [src/test/ping_pong_test.ccp](./src/test/) which should be compiled using `make ping_pong_test`). It takes the following arguments (not detailing other less usefull debuging flags):
 - `-h`: show help infos.
 - `-v`: show verbose.
 - `-n`: number of iteration, default is `4`.
 - `-s`: minimum graph size to test, default is `1`.
 - `-S`: maximum graph size to test, default is `5`.
 - `-r`: minimum random seed to test, default is `0`.
 - `-R`: minimum random seed to test, default is `100`.

### MPI injectivity test

Injectivity testing for multiple graphs is done using [src/test/mpi_injectivity_tst.sh](./src/test/) script (relying on [src/test/mpi_ping_pong_test.ccp](./src/test/) which should be compiled using `make CXX=mpic++ mpi_ping_pong_test`). It takes the following arguments (not detailing other less usefull debuging flags):
 - `-h`: show help infos.
 - `-v`: show verbose.
 - `-n`, `-s`, `-S`, `-r` and `-R`: same as `injectivity_test.sh`, but with different default value (detailed in the `-h` menu).
 - `-t`: number of thread per rank, default is `1`.
 - `-p`: number of rank per node, default is `1`.

## Obtaining QCGD dynamic evolution

Actual QCGDs simulations are supported (only using MPI) by the [src/simulation/quantum_iterations.cpp](./src/simulation/) file, which can be compiled using `make CXX=mpic++`, and simply takes the same arguments as [src/test/mpi_ping_pong_test.ccp](./src/test/) or [src/test/mpi_ping_pong_test.ccp](./src/test/), and prints out a json-formated list of the average values after each application of a rule. For example:

```bash
mpirun -n 8 quantum_iterations.out 4\|12\|step\;split_merge
```

# Experiment used in the paper:

  ```bash
# to clear slurm queue
squeue -u $USER | grep "acc" | awk '{print $1}' | xargs scancel
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
  -a 7,max_num_object=20000000,seed=0\|15\|step\;split_merge -ores_sm_


# get results from single node
./csv-from-tmp.py res_ec_
./csv-from-tmp.py res_sm_


# multi-node scaling
./mpi_scaling.sh -N 41,38,35,32,29,26,23,20,18,16,14,12,10,8,6 \
  -n 36 -t 1 \
  -f bora_scaling_test.out \
  -s "-C bora --exclusive -J erase_create --time=0-00:5" \
  -a 13,max_num_object=-1,seed=0\|14\|step\;erase_create -oec_bora_
./mpi_scaling.sh -N 6 \
  -n 36,18,8,4,2 -t 1,1,1,1,1 \
  -f bora_scaling_test.out \
  -s "-C bora --exclusive -J erase_create --time=0-2:00" \
  -a 13,max_num_object=-1,seed=0\|14\|step\;erase_create -oec_bora_
./mpi_scaling.sh -N 41,38,35,32,29,26,23,20,18,16,14,12,10,8,6,4,2,1 \
  -n 36 -t 1 \
  -f bora_scaling_test.out \
  -s "-C bora --exclusive -J split_merge --time=0-00:5" \
  -a 9,seed=0\|15\|step\;split_merge -osm_bora_


# get results from multi-node
./csv-from-tmp.py ec_bora_
./csv-from-tmp.py sm_bora_


# accuracy testing
./mpi_scaling.sh -N 41,38,35,32,29,26,23,20,18,16,14,12,10,8,6,4,2,1 \
  -n 36 -t 1 \
  -f bora_scaling_test.out \
  -s "-C bora --exclusive -J accuracy --time=0-00:5" \
  -a 9,seed=0\|11\|step\;split_merge,theta=0.125 -oacc_bora_


# get results from multi-node
./csv-from-tmp.py acc_bora_


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