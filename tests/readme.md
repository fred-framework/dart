# How to run the tests

 - setup a workstation capable of compiling and running DART;
 - run 'cd ./build; ./run.sh' for compiling DART for different configurations;
 - run 'cd ./minimal; ./run.sh' for running DART for the previously built configurations with partitioning mode OFF;
 - build a FRED/DART Linux image for each target board using [meta-fred](https://fred-framework-docs.readthedocs.io/en/latest/docs/07_getting-started/index.html);
 - TO BE DONE: run configuration on their boards;
 - TO BE DONE: repeat the process for minimal with a test with multiple reconfigration regions.

# Tests to be developed (TODO)

 - minimal_part: same as 'minimal', but with partitioning mode ON;
 - two_rr: Two reconfigurable regions with partitioning mode OFF;
 - two_rr_part: Two reconfigurable regions with partitioning mode ON;
 - debug: Two reconfigurable regions with partitioning mode OFF and debug mode (ILA) ON;

# Challenge

 - how to run DART in a CI environment since it required Xilinx and Gurobi licenses ?!?!
  