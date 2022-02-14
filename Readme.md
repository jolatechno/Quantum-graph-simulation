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
squeue -u $USER | awk '{print $1}' | tail -n+2 | xargs scancel

cd Quantum-graph-simulation/v7/omp_mpi_scalling_test/
(cd ../IQS && git pull origin dev)



module load compiler/gcc/11.2.0
module load mpi/openmpi/4.0.1
make CFLAGS="-march=skylake -DSIMPLE_TRUNCATION -DCOLLISION_TEST_PROPORTION=0 -DLOAD_BALANCING_BUCKET_PER_THREAD=32 -obora_scaling_test.out" CXX=mpic++
make CFLAGS="-march=znver2 -DSIMPLE_TRUNCATION -DCOLLISION_TEST_PROPORTION=0 -DLOAD_BALANCING_BUCKET_PER_THREAD=32 -ozonda_scaling_test.out" CXX=mpic++



./mpi_scaling.sh -n 1,2,4,8,16,32,64,1,2,4,8,16,32,1,2,4,8,16,1,2,4,8,1,2,4,1,2,1 \
  -t 64,32,16,8,4,2,1,32,16,8,4,2,1,16,8,4,2,1,8,4,2,1,4,2,1,2,1,1 \
  -f zonda_scaling_test.out \
  -s " -J erase_create -C zonda --exclusive --time=0-10:00" \
  -a 7,max_num_object=2000000,seed=0\|15\|step\;erase_create -ores_ec_

./mpi_scaling.sh -n 1,2,4,8,16,32,64,1,2,4,8,16,32,1,2,4,8,16,1,2,4,8,1,2,4,1,2,1 \
  -t 64,32,16,8,4,2,1,32,16,8,4,2,1,16,8,4,2,1,8,4,2,1,4,2,1,2,1,1 \
  -f zonda_scaling_test.out \
  -s " -J split_merge -C zonda --exclusive --time=0-10:00" \
  -a 8,max_num_object=30000000,seed=0\|15\|step\;split_merge -ores_sm_

./mpi_scaling.sh -n 1,1,1,1,1,1,1 \
  -t 64,32,16,8,4,2,1 \
  -f zonda_scaling_test.out \
  -s " -J erase_create -C zonda --exclusive --time=0-10:00" \
  -a 7,max_num_object=2000000,seed=0\|15\|step\;erase_create -ores_ec_omp_

./mpi_scaling.sh -n 1,1,1,1,1,1,1 \
  -t 64,32,16,8,4,2,1 \
  -f zonda_scaling_test.out \
  -s " -J split_merge -C zonda --exclusive --time=0-10:00" \
  -a 8,max_num_object=30000000,seed=0\|15\|step\;split_merge -ores_sm_omp_

./csv-from-tmp.py res_ec_
./csv-from-tmp.py res_sm_




./mpi_scaling.sh -N 44,41,38,35,32,29,26,23,20,18,16,14,12,10,8,4,2,1 \
  -n 1,2,4,6,12,18,36 -t 36,18,9,6,3,2,1 \
  -f bora_scaling_test.out \
  -s "-C bora --exclusive -J erase_create --time=0-00:15" \
  -a 9,safety_margin=0.3,seed=0\|14\|step\;erase_create -oec_bora_

./mpi_scaling.sh -N 44,41,38,35,32,29,26,23,20,18,16,14,12,10,8,4,2,1 \
  -n 1,2,4,6,12,18,36 -t 36,18,9,6,3,2,1 \
  -f bora_scaling_test.out \
  -s "-C bora --exclusive -J split_merge --time=0-00:15" \
  -a 9,safety_margin=0.3,seed=0\|15\|step\;split_merge -osm_bora_

./csv-from-tmp.py ec_bora_
./csv-from-tmp.py sm_bora_



make CFLAGS="-DMIN_VECTOR_SIZE=1000" ping_pong_test



module load compiler/gcc/11.2.0
module load mpi/openmpi/4.0.1
make CFLAGS="-march=skylake -DLESS_DEBUG -DCOLLISION_TEST_PROPORTION=0 -DLOAD_BALANCING_BUCKET_PER_THREAD=32" CXX=mpic++ mpi_ping_pong_test

n_per_node=4 n_threads=9 args="-v -R 10 -n 7 -s 4 -S 14" sbatch -N 40 --time=0-03:00:00 --output=test.out --error=test.err --exclusive -C bora slurm.sh

sed '/bora/d' test.err
```