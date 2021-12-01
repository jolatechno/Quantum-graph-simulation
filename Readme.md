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

make CXX=mpic++ CFLAGS="-DMIN_EQUALIZE_SIZE=100 -DLOAD_FACTOR=3 -DMIN_VECTOR_SIZE=1000"

./mpi_scaling.sh -N 1 -n 1,2,4,8,16,32,64,1,2,4,8,16,32,1,2,4,8,16,1,2,4,8,1,2,4,1,2,1 \
  -t 64,32,16,8,4,2,1,32,16,8,4,2,1,16,8,4,2,1,8,4,2,1,4,2,1,2,1,1 \
  -s " -J erase_create -C zonda --exclusive --time=0-10:00" \
  -a 7,max_num_object=2000000,seed=0\|15\|step\;erase_create -ores_ec_

./mpi_scaling.sh -N 1 -n 1,2,4,8,16,32,64,1,2,4,8,16,32,1,2,4,8,16,1,2,4,8,1,2,4,1,2,1 \
  -t 64,32,16,8,4,2,1,32,16,8,4,2,1,16,8,4,2,1,8,4,2,1,4,2,1,2,1,1 \
  -s " -J split_merge -C zonda --exclusive --time=0-10:00" \
  -a 8,max_num_object=30000000,seed=0\|15\|step\;split_merge -ores_sm_
```