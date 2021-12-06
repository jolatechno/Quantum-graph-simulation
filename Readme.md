# Quantum-graph-simulation

## Compilation

In any directory with compilable code, you can compile the code using the `make` command (the targets of make are the main file name without any extensions).

# Compilable files

The different programs are at:

-   [/v6/grapher/quantum_iterations.cpp][]: simulating iterations and printing out average values to json.
-   [/v6/grapher/probabilist_iterations.cpp][]: simulating probabilistic iterations and printing out average values to json.
-   [/v6/test/state_test.cpp][] and [/v6/scaling_test/scaling_test.cpp][]: simple iterations test.
-   [/v6/test/injectivity_test.sh][]: injectivity test on a large number of starting graph and for all rules to validate simulation.
-   [/v6/test/imperfect_injectivity_test.py][]: imperfect injectivity test, similar to [/v6/test/injectivity_test.sh][] but for simulations where deleting graphs is necessary.
-   [/v6/scaling_test/multithreading_test.sh][]: Test multithreading scaling.
-   [/v6/scaling_test/numa_test.sh][]: Test numa-aware allocation.
-   [/validation/selection/selector_benchmark.cpp][]: Compare the perfomences of different methodes of duplicate elimination.

All of the above programs have help ("-h") sections that gives precision
on the parameters.

There are also "grapher" scripts that are used to obtain the figures
shown here :

-   [/v6/grapher/grapher.py][]: grapher for json file (default is res.json) obtained from [quantum_iterations.cpp] or [probabilist_iterations.cpp]. Output directories are inside [/v6/grapher/plots/].
-   [/v6/scaling_test/grapher.py][]: grapher for json file (default is res.json) obtained from [multithreading_test.sh]. Output directories are inside [/v6/scaling_test/plots].
-   [/validation/random_selector/repartition.py] is also a grapher responsible for creating the random selection plots, and outputs to [/validation/random_selector/plots/].
-   [/v6/test/grapher.py][]: grapher for [/v6/test/imperfect_injectivity_test.py]. Outputs to [/v6/test/plots/].

  [/v6/grapher/grapher.py]: ./v6/grapher/grapher.py
  [quantum_iterations.cpp]: ./v6/grapher/quantum_iterations.cpp
  [probabilist_iterations.cpp]: ./v6/grapher/probabilist_iterations.cpp
  [/v6/grapher/plots/]: ./v6/grapher/plots/
  [/v6/scaling_test/grapher.py]: ./v6/scaling_test/grapher.py
  [multithreading_test.sh]: ./v6/scaling_test/multithreading_test.sh
  [/v6/scaling_test/plots]: ./v6/scaling_test/plots/
  [/validation/random_selector/repartition.py]: ./validation/random_selector/repartition.py
  [/validation/random_selector/plots/]: ./validation/random_selector/plots/
  [/v6/test/grapher.py]: ./v6/test/grapher.py
  [/v6/test/imperfect_injectivity_test.py]: ./v6/test/imperfect_injectivity_test.py
  [/v6/test/plots/]: ./v6/test/plots/
  [/v6/grapher/quantum_iterations.cpp]: ./v6/grapher/quantum_iterations.cpp
  [/v6/grapher/probabilist_iterations.cpp]: ./v6/grapher/probabilist_iterations.cpp
  [/v6/test/state_test.cpp]: ./v6/test/state_test.cpp
  [/v6/scaling_test/scaling_test.cpp]: ./v6/scaling_test/scaling_test.cpp
  [/v6/test/injectivity_test.sh]: ./v6/test/injectivity_test.sh
  [/v6/test/imperfect_injectivity_test.py]: ./v6/test/imperfect_injectivity_test.py
  [/v6/scaling_test/multithreading_test.sh]: ./v6/scaling_test/multithreading_test.sh
  [/v6/scaling_test/numa_test.sh]: ./v6/scaling_test/numa_test.sh
  [/validation/selection/selector_benchmark.cpp]: ./validation/selection/selector_benchmark.cpp

  ```bash
srun --time=0-10:00 -C zonda --pty bash -i

cd Quantum-graph-simulation/v7/omp_mpi_scalling_test/
(cd ../IQS && git pull origin dev)

module load compiler/gcc/11.2.0
module load mpi/openmpi/3.1.4

make CFLAGS="-DMIN_EQUALIZE_SIZE=100 -DMIN_VECTOR_SIZE=1000 -march=skylake -obora_scling_test.out" CXX=mpic++
make CFLAGS="-DMIN_EQUALIZE_SIZE=100 -DMIN_VECTOR_SIZE=1000 -march=znver2 -ozonda_scling_test.out" CXX=mpic++

./mpi_scaling.sh -N 1 -n 2,1,4,8,16,32,64,1,2,4,8,16,32,1,2,4,8,16,1,2,4,8,1,2,4,1,2,1 \
  -t 32,64,16,8,4,2,1,32,16,8,4,2,1,16,8,4,2,1,8,4,2,1,4,2,1,2,1,1 \
  -s " -J erase_create -C zonda --exclusive --time=0-10:00" \
  -a 7,max_num_object=2000000,seed=0\|15\|step\;erase_create -ores_ec_

./mpi_scaling.sh -N 1 -n 2,1,4,8,16,32,64,1,2,4,8,16,32,1,2,4,8,16,1,2,4,8,1,2,4,1,2,1 \
  -t 32,64,16,8,4,2,1,32,16,8,4,2,1,16,8,4,2,1,8,4,2,1,4,2,1,2,1,1 \
  -s " -J split_merge -C zonda --exclusive --time=0-10:00" \
  -a 8,max_num_object=30000000,seed=0\|15\|step\;split_merge -ores_sm_

./mpi_scaling.sh -N 1,2,4,6,8,10,12,14,16,18,20,23,26,29,32,35,38,41 \
  -n 1,2,4,9,18,36 -t 36,18,9,4,2,1 \
  -f bora_scaling_test.out \
  -s "-C bora --exclusive -J erase_create" \
  -a 9,safety_margin=0.3,seed=0\|14\|step\;erase_create -oec_bora_

./mpi_scaling.sh -N 1,2,4,6,8,10,12,14,16,18,20,23,26,29,32,35,38,41 \
  -n 1,2,4,9,18,36 -t 36,18,9,4,2,1 \
  -f bora_scaling_test.out \
  -s "-C bora --exclusive -J split_merge" \
  -a 9,safety_margin=0.3,seed=0\|15\|step\;split_merge -osm_bora_

./convert-json-to-csv.py .res_ec_local.json,_local \
  .res_sm_local.json,_local \
  .res_ec_cluster.json,_cluster \
  .res_sm_cluster.json,_cluster \
  .res_ec_local.json,_local_omp



make CFLAGS="-DMIN_EQUALIZE_SIZE=100 -DMIN_VECTOR_SIZE=1000 -march=znver2" CXX=mpic++ mpi_ping_pong_test

salloc -N 3 --time=0-01:00 --exclusive -N 3 -C zonda
srun hostname
srun -N 1 hostname
srun --pty bash -i

./mpi_injectivity_test.sh -v -p 4 -t 16 -R 10 -n 7 -s 4 -S 13
```