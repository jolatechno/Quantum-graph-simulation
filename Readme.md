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
  [/validation/random_selector/repartition.py]: ./validation/repartition.py
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