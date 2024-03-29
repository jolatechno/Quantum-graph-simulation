# Quantum-graph-simulation

## Implementation

__*!!!ALL the implementation relies on [src/QuIDS](./src/) found at [jolatechno/QuIDS](https://github.com/jolatechno/QuIDS)!!!*__

## Results

Scaling results are in the [jolatechno/Quantum-graph-simulation-plots](https://github.com/jolatechno/Quantum-graph-simulation-plots) sub-repository.

## Compilation

In any directory with compilable code, you can compile the code using the `make` command (the targets of make are the main file name without any extensions).

To compile for MPI (which is requiered for almost all file) `CXX=mpic++` should be added to the make command to link MPI binary library.

# Usage

## Running a simple test - Ping pong test

The "ping pong" test is the test later used for validation, it consists in applying N iterations, then N reversed iteration to hopefully obtain back the starting state.

The [src/test/ping_pong_test.ccp](./src/test/) file (later used for injectivity test) simply print the intial state, and after running the simulation (including the inverse transformation) will print the final state. If you don't wat to apply the reverse transformation you can simply pass `reversed_n_iters=0`.

Compilation is done using `make ping_pong_test`, you can run it, 

### Using MPI - MPI ping pong test

The [src/test/mpi_ping_pong_test.ccp](./src/test/) file has the same function as [src/test/ping_pong_test.ccp](./src/test/), but support MPI. Since it gather the final state on the main rank at the end before printing it, if their is too much object by this time to fit in memory of a single node, the program will crash.

Compilation is done using `make CXX=mpic++ mpi_ping_pong_test`.

## Obtaining scaling results

The compilable file used to obtain scaling results is [src/omp_mpi_scaling_test/scaling_test.cpp](./src/omp_mpi_scaling_test/). After compiling it using `make CXX=mpic++`, actual scaling results are obtained using the [src/omp_mpi_scaling_test/scaling_test.sh](./src/omp_mpi_scaling_test/) script, which get passed the following arguments:
 - `-h`: simple help infos.
 - `-f`: compiled file used, default is `scaling_test.out`.
 - `-a`: argument passed to `scaling_test.out`, in the form described above (_n\_iter_|_graph\_size_|_rules_). Note that `reversed_n_iter` is used set the iteration at which we start measuring performance (to let the program generate enough graphs at first). Default is `0`.
 - `-t`: list of the number of threads to test (ex: `1,2,6` the default is the number_of available threads).
 - `-n`: list of the number of mpi rank to spawn per node (ex: `6,3,1` default is `1`).
 - `-m`: additionals arguments for `mpirun`.
 - `-N`: total number of nodes used for MPI, default is `1`.

Note that the output (after separating `stderr` from `stdout`) will be formated as a json.

### Slurm integration

To obtain scaling results for different number of nodes, using slurm [src/omp_mpi_scaling_test/mpi_scaling.sh](./src/omp_mpi_scaling_test/) is used (which simply calls [src/omp_mpi_scaling_test/scaling_test.sh](./src/omp_mpi_scaling_test/) script, and stores the results in `src/omp_mpi_scaling_test/tmp`). It get passed the following arguments:
 - `-h`: simple help infos.
 - `-a`: argument passed to `scaling_test.out`, similar to `-a` for `scaling_test.sh`. `reversed_n_iter` is also used set the iteration at which we start measuring performance. Default is `0`.
 - `-M`: comma-separated list of modules to be loaded.
 - `-u`: if true, uses `mpirun` through [scaling_test.sh](./src/omp_mpi_scaling_test/), otherwise uses `slurm`.
 - `-f`, `-t`, `-n` and `-m`: same as [scaling_test.sh](./src/omp_mpi_scaling_test/).
 - `-N`: list of number of nodes to ask from sbatch (example `1,2,4` default is `1`).
 - `-s`: additional arguments to pass to sbatch (to ask for specific nodes for example).
 - `-o`: base name of the output files (default is `out_`, so the results for _n_ ranks will be _res\_n.out_ and _res\_n.err_).

#### Data processing

The [src/omp_mpi_scaling_test/csv-from-tmp.py](./src/omp_mpi_scaling_test/) script (requiering python 3) simply takes a base name (`-o` argument of [src/omp_mpi_scaling_test/mpi_scaling.sh](./src/omp_mpi_scaling_test/)) and returns a csv formated compilation of the results obtained by using [src/omp_mpi_scaling_test/mpi_scaling.sh](./src/omp_mpi_scaling_test/).

Similarly, [src/omp_mpi_scaling_test/mem-csv-from-file.py](./src/omp_mpi_scaling_test/) is used to process the memory usage evolution of a single file into a csv format. The argument provided should simply be the name of the file ("`-o`" base name, number of node, and `.out` extension).

## Reproducing injectivity test

Injectivity testing correspond to running the `ping_pong_test` on a variety of starting graph state/size and rules to insure that the state after the ping-pong is the same as the starting state. 

Injectivity testing is done using [src/test/injectivity_test.sh](./src/test/) script (relying on [src/test/ping_pong_test.ccp](./src/test/) which should be compiled using `make ping_pong_test`). It takes the following arguments (not detailing other less usefull debuging flags):
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


## General interface

All compiled file get passed a special argument describing exactly the simulation. It is formated as:

_n\_iter_,_option1_=x,_option2_=y...|_graph\_size1_,_graph\_option1=x,...;_graph\_size2_,...|_rule1_,_rule1\_option1_=x...;_rule2_,...

where _n\_iter_ and _graph\_size_ are integers describing respectivly the number of iterations and the initial graph size of the simulation.

### General options

The _options_ are:
 - `seed` : the random seed used to generate random objects. If not given, selected as random.
 - `reversed_n_iters` : number of iteration to do with the inverse transformation (only used in certain files, for injectivity testing, default is _n\_iter_ when used).
 - `max_num_object` : representing the maximum number of object to keep per shared memory node. `0` represents auto-truncation (keeping the maximum number of graph within memory limits), `-1` represent no truncation (can cause crashes when running out of memory). The default is `0`.
 - `safety_margin` : representing `quids::safety_margin` (see the Readme from [jolatechno/QuIDS](https://github.com/jolatechno/QuIDS)).
 - `tolerance` : representing `quids::tolerance`.
 - `simple_truncation` : representing `quids::simple_truncation`.
 - `load_balancing_bucket_per_thread` : representing `quids::load_balancing_bucket_per_thread`.
 - `align` : represents `quids::align_byte_length` (the amount to which objects should be alligned)
 - `min_equalize_size` : representing `quids::mpi::min_equalize_size` (only interpreted when MPI is used).
 - `equalize_inbalance` : representing `quids::mpi::equalize_inbalance` (only interpreted when MPI is used).

### Graph generation options

The _graph\_options_ are used to parametrize the initial state. They are as follow:
 - `n_graphs` : reprents the number of graph with a given set of option. Default is `1`.
 - `real` : represent the real part of the magnitude shared by all graphs with a given set of option.
 - `imag` : represent the imaginary part of the magnitude shared by all graphs with a given set of option.

__IMPORTANT:__ Note that the initial state is normalized after generation, so the magnitudes don't have to add up to 1.

### Rules

Implemented _rules_ (all described in further details in [TO LINK](https://google.com)) are:
 - `step` : a simple `quids::modifier` moving all particles in the same direction as their orientation.
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

## Examples 

So to simulate 2 iteration of `step` followed by `erase_create` with a single starting graph with 12 nodes, and with a safety margin of `0.5`, the following argument is passed:

```bash
...command... "2,safety_margin=0.5|12|step;erase_create"
```

To simulate 2 iteration of `step` followed by `split_merge` with one starting graph with 12 nodes and 2 starting graphs with 14 nodes and a pure imaginary magnitutde, the following argument is passed:

```bash
...command... "2|12;14,n_graphs=2,imag=1,real=0|step;split_merge"
```

To simulate 2 iteration, starting with a single graph of size 12, and applying `step` two times followed by `coin` with `theta=0.125`, the following argument is passed:

```bash
...command... "2|12|step,n_iter=2;split_merge,theta=0.125"
```

### Ping pong test

To run a ping pong test for 4 iterations of `step` followed by `split_merge` starting with a single graph of size 12 using the following command:

```bash
./pin_pong_test.out "4,reversed_n_iters=0|12|step;split_merge"
```

### MPI ping pong test

To run the ping pong test with 8 processes (see [mpirun(1) man page](https://www.open-mpi.org/doc/v4.0/man1/mpirun.1.php) for more info on mpirun), for 4 iterations of `step` followed by `split_merge` starting with a single graph of size 12 using the following command:

```bash
mpirun -n 8 mpi_ping_pong_test.out "4,reversed_n_iters=0|12|step;split_merge"
```

### Scaling results

To test the scaling on 5 nodes for 1 rank times 6 threads, 3 ranks time 2 threads, and 6 ranks times 1 threads for 3 iteration of `step` followed by `erase_create` starting with a single graph of 12 nodes, the command will be:

```bash
./scaling_test.sh -N 5 -t 6,2,1 -n 1,3,6 -a "4,reversed_n_iters=0|12|step;split_merge"
```

### Slurm integration

To test the scaling on 1,2,4 and 6 nodes for 1 rank times 6 threads, 3 ranks time 2 threads, and 6 ranks times 1 threads for 3 iteration of `step` followed by `erase_create` starting with a single graph of 12 nodes, the command will be:

```bash
./mpi_scaling.sh -N 1,2,4,6 -t 6,2,1 -n 1,3,6 -a "4,reversed_n_iters=0|12|step;split_merge"
```

## Obtaining QCGD dynamic evolution

Actual QCGDs simulations are supported (only using MPI) by the [src/simulation/quantum_iterations.cpp](./src/simulation/) file, which can be compiled using `make CXX=mpic++`, and simply takes the same arguments as [src/test/mpi_ping_pong_test.ccp](./src/test/) or [src/test/mpi_ping_pong_test.ccp](./src/test/), and prints out a json-formated list of the average values after each application of a rule. For example (for 8 processes):

```bash
mpirun -n 8 quantum_iterations.out "4|12|step;split_merge"
```

# Experiment used in the paper:

## Clusters

We mainly used two clusters:
 - [Plafrim](https://plafrim-users.gitlabpages.inria.fr/doc/#hardware).
 - [Ruche](https://mesocentre.pages.centralesupelec.fr/user_doc/ruche/01_cluster_overview/) provided by __Centrale Supelec__.

## Commands

The following commands

### Plafrim

#### Validation tests

```bash


# ---------------------------
# ---------------------------
# simple stability and validations test from here
# not included in the paper
# ---------------------------
# ---------------------------

# ---------------------------
# other demanding stability tests
# ---------------------------


# single node multi-rule stability test (still inside src/omp_mpi_scaling_test)
./mpi_scaling.sh -u -n 64 -t 1 \
  -f zonda_scaling_test.out \
  -M compiler/gcc/11.2.0,mpi/openmpi/4.0.1 \
  -s " -J test_birule -C zonda --exclusive --time=0-2:00" \
  -a "5,seed=0|15|step;erase_create;step;split_merge" -o test_birule_

# multi-node stability test (still inside src/omp_mpi_scaling_test)
./mpi_scaling.sh -u -N 2,4,8,16 \
  -n 36 -t 1 \
  -f bora_scaling_test.out \
  -M compiler/gcc/11.2.0,mpi/openmpi/4.0.1 \
  -m "--mca mtl psm2" \
  -s "-C bora --exclusive -J ec_long --time=0-2:00" \
  -a "10,seed=0|30|step;erase_create" -o test_very_long_ec_
./mpi_scaling.sh -u -N 2,4,8,16 \
  -n 36 -t 1 \
  -f bora_scaling_test.out \
  -M compiler/gcc/11.2.0,mpi/openmpi/4.0.1 \
  -m "--mca mtl psm2" \
  -s "-C bora --exclusive -J sm_long --time=0-2:00" \
  -a "20,seed=0|30|step;split_merge" -o test_long_sm_

# multi-bode multi-rule stability test (still inside src/omp_mpi_scaling_test)
./mpi_scaling.sh -u \
  -N 2,4,8,16 \
  -n 36 -t 1 \
  -f bora_scaling_test.out \
  -M compiler/gcc/11.2.0,mpi/openmpi/4.0.1 \
  -m "--mca mtl psm2" \
  -s "-C bora --exclusive -J test_bi_rule --time=0-2:00" \
  -a "10,seed=0|20|step;erase_create;step;split_merge" -o test_birule_


# ---------------------------
# injectivity tests
# ---------------------------


cd src/test

# compilation (in src/test)
module purge
module load compiler/gcc/11.2.0
module load mpi/openmpi/4.0.1
make CXX=mpic++


# simple single node openmp injectivity test (still in src/test)
./injectivity_test.sh -v


# simple single node mpi injectivty test (still in src/test) with NUM_THREAD threads per task and NUM_TASK tasks (with NUM_THREAD*NUM_TASK <= #CPU Cores)
./mpi_injectivity_test.sh -v -t NUM_THREAD -p NUM_TASK


# multi-node mpi injectivity test
MODULES=compiler/gcc/11.2.0,mpi/openmpi/4.0.1 n_per_node=36 n_threads=1 args="-v -s 6 -S 14 -n 8" sbatch -N 10 -C bora slurm.sh





# ---------------------------
# ---------------------------
# commands offten used when running experiments
# ---------------------------
# ---------------------------

# to clear slurm queue
squeue -u $USER | awk '{print $1}' | tail -n+2 | xargs scancel
squeue -u $USER | grep "14 (" | awk '{print $1}' | xargs scancel
squeue -u $USER | grep "QOSMaxCpuPerUserLimit" | awk '{print $1}' | xargs scancel
```

### Ruche

#### Scalling tests

```bash
# ---------------------------
# ---------------------------
# command to replicate results on ruche
#   '-> cluster info page: https://mesocentre.pages.centralesupelec.fr/user_doc/ruche/01_cluster_overview/
# ---------------------------
# ---------------------------


# ---------------------------
# compile
# ---------------------------

module purge
module load gcc/11.2.0/gcc-4.8.5
#module load mpich/3.3.2/gcc-9.2.0
module load intel-mpi/2019.9.304/intel-20.0.4.304
#module load openmpi/4.1.1/gcc-11.2.0

make CFLAGS="-march=cascadelake -DSAFETY_MARGIN=0.15 -DEQUALIZE_FACTOR=0.25" CXX=mpic++ #CXX=mpicxx


# ---------------------------
# scaling tests
# ---------------------------


# multi-node strong scaling (still inside src/omp_mpi_scaling_test)
./mpi_scaling.sh -N 1,2,4,6,8,10,14,16,20,24 \
  -n 40 -t 1 -G 900000 -f scaling_test.out \
  -M gcc/11.2.0/gcc-4.8.5,intel-mpi/2019.9.304/intel-20.0.4.304 \
  -m "--mpi=pmi2" -s "-p cpu_short,cpu_med,cpu_prod,cpu_scale --exclusive -J s_ec_s --time=0-01:00" \
  -a "15,reversed_n_iter=10,seed=0|16|step;erase_create" -o strong_ec_short_
./mpi_scaling.sh -N 26,30,34,36,40,44,46,50 \
  -n 40 -t 1 -G 900000 -f scaling_test.out \
  -M gcc/11.2.0/gcc-4.8.5,intel-mpi/2019.9.304/intel-20.0.4.304 \
  -s "-p cpu_prod,cpu_scale --exclusive -J s_ec_s --time=0-01:00" \
  -a "15,reversed_n_iter=10,seed=0|16|step;erase_create" -o strong_ec_short_
./mpi_scaling.sh -N 55,60,65,60,65,70,75,80,85,90,95,100 \
  -n 40 -t 1 -G 900000 -f scaling_test.out \
  -M gcc/11.2.0/gcc-4.8.5,intel-mpi/2019.9.304/intel-20.0.4.304 \
  -s "-p cpu_scale --exclusive -J s_ec_s --time=0-01:00" \
  -a "15,reversed_n_iter=10,seed=0|16|step;erase_create" -o strong_ec_short_

./mpi_scaling.sh -N 20 \
  -n 1,2,5,10,20,40 -t 1,1,1,1,1,1 -G 18000000 -f scaling_test.out \
  -M gcc/11.2.0/gcc-4.8.5,intel-mpi/2019.9.304/intel-20.0.4.304 \
  -m "--mpi=pmi2" -s "-p cpu_short,cpu_med,cpu_prod,cpu_scale --exclusive -J s_ec_l --time=0-01:00" \
  -a "15,reversed_n_iter=10,seed=0|16|step;erase_create" -o strong_ec_long_
./mpi_scaling.sh -N 24,26,30,34,36,40,44,46,50 \
  -n 40 -t 1 -G 18000000 -f scaling_test.out \
  -M gcc/11.2.0/gcc-4.8.5,intel-mpi/2019.9.304/intel-20.0.4.304 \
  -s "-p cpu_prod,cpu_scale --exclusive -J s_ec_l --time=0-01:00" \
  -a "15,reversed_n_iter=10,seed=0|16|step;erase_create" -o strong_ec_long_
./mpi_scaling.sh -N 55,60,65,60,65,70,75,80,85,90,95,100 \
  -n 40 -t 1 -G 18000000 -f scaling_test.out \
  -M gcc/11.2.0/gcc-4.8.5,intel-mpi/2019.9.304/intel-20.0.4.304 \
  -s "-p cpu_scale --exclusive -J s_ec_l --time=0-01:00" \
  -a "15,reversed_n_iter=10,seed=0|16|step;erase_create" -o strong_ec_long_


# multi-node strong scaling (still inside src/omp_mpi_scaling_test)
./mpi_scaling.sh -N 1,2,4,6,8,10,14,16,20,24 \
  -n 40 -t 1 -G 20000000 -f scaling_test.out \
  -M gcc/11.2.0/gcc-4.8.5,intel-mpi/2019.9.304/intel-20.0.4.304 \
  -m "--mpi=pmi2" -s "-p cpu_short,cpu_med,cpu_prod,cpu_scale --exclusive -J s_sm_s --time=0-01:00" \
  -a "16,reversed_n_iter=10,seed=0|15|step;split_merge" -o strong_sm_short_
./mpi_scaling.sh -N 26,30,34,36,40,44,46,50 \
  -n 40 -t 1 -G 20000000 -f scaling_test.out \
  -M gcc/11.2.0/gcc-4.8.5,intel-mpi/2019.9.304/intel-20.0.4.304 \
  -s "-p cpu_prod,cpu_scale --exclusive -J s_sm_s --time=0-01:00" \
  -a "16,reversed_n_iter=10,seed=0|15|step;split_merge" -o strong_sm_short_
./mpi_scaling.sh -N 55,60,65,60,65,70,75,80,85,90,95,100 \
  -n 40 -t 1 -G 20000000 -f scaling_test.out \
  -M gcc/11.2.0/gcc-4.8.5,intel-mpi/2019.9.304/intel-20.0.4.304 \
  -s "-p cpu_scale --exclusive -J s_sm_s --time=0-01:00" \
  -a "16,reversed_n_iter=10,seed=0|15|step;split_merge" -o strong_sm_short_
  

./mpi_scaling.sh -N 20 \
  -n 1,2,5,10,20,40 -t 1,1,1,1,1,1 -G 400000000 -f scaling_test.out \
  -M gcc/11.2.0/gcc-4.8.5,intel-mpi/2019.9.304/intel-20.0.4.304 \
  -m "--mpi=pmi2" -s "-p cpu_short,cpu_med,cpu_prod,cpu_scale --exclusive -J s_sm_l --time=0-01:00" \
  -a "16,reversed_n_iter=10,seed=0|15|step;split_merge" -o strong_sm_long_
./mpi_scaling.sh -N 24,26,30,34,36,40,44,46,50 \
  -n 40 -t 1 -G 400000000 -f scaling_test.out \
  -M gcc/11.2.0/gcc-4.8.5,intel-mpi/2019.9.304/intel-20.0.4.304 \
  -s "-p cpu_prod,cpu_scale --exclusive -J s_sm_l --time=0-01:00" \
  -a "16,reversed_n_iter=10,seed=0|15|step;split_merge" -o strong_sm_long_
./mpi_scaling.sh -N 55,60,65,60,65,70,75,80,85,90,95,100 \
  -n 40 -t 1 -G 400000000 -f scaling_test.out \
  -M gcc/11.2.0/gcc-4.8.5,intel-mpi/2019.9.304/intel-20.0.4.304 \
  -s "-p cpu_scale --exclusive -J s_sm_l --time=0-01:00" \
  -a "16,reversed_n_iter=10,seed=0|15|step;split_merge" -o strong_sm_long_
  

# multi-node weak scaling (still inside src/omp_mpi_scaling_test)
./mpi_scaling.sh -N 1,2,4,6,8,10,14,16,20,24 \
  -n 40 -t 1 -G 0 -f scaling_test.out \
  -M gcc/11.2.0/gcc-4.8.5,intel-mpi/2019.9.304/intel-20.0.4.304 \
  -m "--mpi=pmi2" -s "-p cpu_short,cpu_med,cpu_prod,cpu_scale --exclusive -J w_ec --time=0-01:00" \
  -a "12,reversed_n_iter=7,seed=0|18|step;erase_create" -o weak_ec_
./mpi_scaling.sh -N 26,30,34,36,40,44,46,50 \
  -n 40 -t 1 -G 0 -f scaling_test.out \
  -M gcc/11.2.0/gcc-4.8.5,intel-mpi/2019.9.304/intel-20.0.4.304 \
  -s "-p cpu_prod,cpu_scale --exclusive -J w_ec --time=0-01:00" \
  -a "12,reversed_n_iter=7,seed=0|17|step;erase_create" -o weak_ec_
./mpi_scaling.sh -N 55,60,65,60,65,70,75,80,85,90,95,100 \
  -n 40 -t 1 -G 0 -f scaling_test.out \
  -M gcc/11.2.0/gcc-4.8.5,intel-mpi/2019.9.304/intel-20.0.4.304 \
  -s "-p cpu_scale --exclusive -J w_ec --time=0-01:00" \
  -a "12,reversed_n_iter=7,seed=0|17|step;erase_create" -o weak_ec_


# multi-node weak scaling (still inside src/omp_mpi_scaling_test)
./mpi_scaling.sh -N 1,2,4,6,8,10,14,16,20,24 \
  -n 40 -t 1 -G 0 -f scaling_test.out \
  -M gcc/11.2.0/gcc-4.8.5,intel-mpi/2019.9.304/intel-20.0.4.304 \
  -m "--mpi=pmi2" -s "-p cpu_short,cpu_med,cpu_prod,cpu_scale --exclusive -J w_sm --time=0-01:00" \
  -a "16,reversed_n_iter=10,seed=0|15|step;split_merge" -o weak_sm_
./mpi_scaling.sh -N 26,30,34,36,40,44,46,50 \
  -n 40 -t 1 -G 0 -f scaling_test.out \
  -M gcc/11.2.0/gcc-4.8.5,intel-mpi/2019.9.304/intel-20.0.4.304 \
  -s "-p cpu_prod,cpu_scale --exclusive -J w_sm --time=0-01:00" \
  -a "16,reversed_n_iter=10,seed=0|15|step;split_merge" -o weak_sm_
./mpi_scaling.sh -N 55,60,65,60,65,70,75,80,85,90,95,100 \
  -n 40 -t 1 -G 0 -f scaling_test.out \
  -M gcc/11.2.0/gcc-4.8.5,intel-mpi/2019.9.304/intel-20.0.4.304 \
  -s "-p cpu_scale --exclusive -J w_sm --time=0-01:00" \
  -a "16,reversed_n_iter=10,seed=0|15|step;split_merge" -o weak_sm_


# get results from multi-node (still inside src/omp_mpi_scaling_test)
./combine_output_scaling_test.py strong_ec_short_
./combine_output_scaling_test.py strong_ec_long_
./combine_output_scaling_test.py strong_sm_short_
./combine_output_scaling_test.py strong_sm_long_
./combine_output_scaling_test.py weak_ec_
./combine_output_scaling_test.py weak_sm_


# commands to plot (inside of Quantum-graph-simulation-plots/python_post_processing):
./plot_accuracy_weak_scaling.py ../data/weak_sm_combined.json.out scaling/weak_scaling/accuracy_sm.png
./plot_accuracy_weak_scaling.py ../data/weak_ec_combined.json.out scaling/weak_scaling/accuracy_ec.png
./plot_proportions.py ../data/weak_sm_combined.json.out output=scaling/weak_scaling/proportions_sm.png
./plot_proportions.py ../data/weak_ec_combined.json.out output=scaling/weak_scaling/proportions_ec.png
./plot_weak_scaling.py ../data/weak_sm_combined.json.out scaling/weak_scaling/weak_scaling_sm.png
./plot_weak_scaling.py ../data/weak_ec_combined.json.out scaling/weak_scaling/weak_scaling_ec.png
#./plot_proportions.py ../data/strong_sm_short_combined.json.out ../data/strong_sm_long_combined.json.out output=scaling/strong_scaling/proportions_combined_sm.png
#./plot_proportions.py ../data/strong_ec_short_combined.json.out ../data/strong_ec_long_combined.json.out output=scaling/strong_scaling/proportions_combined_ec.png
#./plot_strong_scaling.py ../data/strong_sm_short_combined.json.out ../data/strong_sm_long_combined.json.out output=scaling/strong_scaling/strong_scaling_combined_sm.png
#./plot_strong_scaling.py ../data/strong_ec_short_combined.json.out ../data/strong_ec_long_combined.json.out output=scaling/strong_scaling/strong_scaling_combined_ec.png
./plot_proportions.py ../data/strong_sm_short_combined.json.out output=scaling/strong_scaling/proportions_short_sm.png
./plot_proportions.py ../data/strong_sm_long_combined.json.out output=scaling/strong_scaling/proportions_long_sm.png
./plot_proportions.py ../data/strong_ec_short_combined.json.out output=scaling/strong_scaling/proportions_short_ec.png
./plot_proportions.py ../data/strong_ec_long_combined.json.out output=scaling/strong_scaling/proportions_long_ec.png
./plot_strong_scaling.py ../data/strong_sm_short_combined.json.out output=scaling/strong_scaling/strong_scaling_short_sm.png
./plot_strong_scaling.py ../data/strong_sm_long_combined.json.out output=scaling/strong_scaling/strong_scaling_long_sm.png
./plot_strong_scaling.py ../data/strong_ec_short_combined.json.out output=scaling/strong_scaling/strong_scaling_short_ec.png
./plot_strong_scaling.py ../data/strong_ec_long_combined.json.out output=scaling/strong_scaling/strong_scaling_long_ec.png


# ---------------------------
# memory usage test
# used for Fig 7 in the paper
# ---------------------------


./mpi_scaling.sh -N 10 \
  -n 40 -t 1 -G 0 -f scaling_test.out \
  -M gcc/11.2.0/gcc-4.8.5,intel-mpi/2019.9.304/intel-20.0.4.304 \
  -m "--mpi=pmi2" -s "-p cpu_short,cpu_med,cpu_prod,cpu_scale --exclusive -J mem --time=0-01:00" \
  -a "17,reversed_n_iter=0,seed=0|17|step;erase_create" -o memory_test_

./combine_output_scaling_test.py memory_test_
./plot_memory_usage.py ../data/memory_test_combined.json.out memory_usage/memory_usage.png
```