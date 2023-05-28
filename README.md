# mmul-omp instructional test harness

This directory contains a benchmark harness for testing different OpenMP-enabled parallel
implementations of matrix multiply for varying problem sizes and, for the blocked mmul 
implmentation, at varying block sizes. The harness includes a benchmark for a reference
implementation that uses CBLAS dgemm for matrix-matrix multiply. 

The main code is benchmark.cpp, which sets up the problem, iterates over problem
(and block) sizes, sets up the matrices, executes the mmul call, and tests the
result for accuracy by comparing your result against a reference implementation (CBLAS).

The benchmark.cpp code also makes calls to LIKWID's Marker API. You will need to add
LIKWID Marker API instrumentation to your implementation of basic and blocked matrix
multiply. There are some comments in dgemm-basic-omp.cpp and dgemm-blocked-omp.cpp
to remind you of this fact. See dgemm-blas.cpp for example usage of the per-thread
LIKWID Marker API, which is valid even if running a serial code.

Note that cmake needs to be able to find the CBLAS package. For CSC 746 Fall 2021,
this condition is true on Perlmutter@NERSC and on the class VM. It is also true for some
other platforms, but you are on your own if using a platform other than Perlmutter@NERSC
or the class VM.

Cmake also needs to be able to find LIKWID.  
This condition is satisfied on  the VM when you load the likwid-5.2.0 module:    
```
module load likwid-5.2.0
```  

To load the LIKWID module on Perlmutter, you need to run the below commands (please refer to build instructions section before running these commands):
```
module load e4s/22.05
spack env activate cuda
spack load likwid
```

<br></br>
# Build instructions - general

After downloading, cd into the main source directly, then:
```
mkdir build
cd build  
cmake ../ -Wno-dev
make
```

When building on Perlmutter, make sure you are on a CPU node when doing the compilation. The simplest way to do this is
grab an interactive KNL node:  
`salloc --nodes 1 --qos interactive --time 01:00:00 --constraint cpu --account m3930`

Before building your code on the CPU node, make sure to load the LIKWID module as mentioned in the previous section. Below is a quick overview of all the steps you need to take to build your code:  
* Grab a CPU node using the `salloc` command above
* Load the LIKWID module using the commands from the previous section
* Build your code using the cmake and make commands mentioned at the start of this section

# Platforms

Due to the dependency on LIKWID and LIKWID's dependency on the Linux MSR kernel
module, the best bet for this assignment is to use Perlmutter@NERSC.

On the VM, the codes will compile and run with likwid-perfctr in serial, but parallel
runs will fail.

Other Linux platforms are possible, but you are on your own to get LIKWID installed,
built, and working.

# Adding your code

For matrix multiplication:

There are stub routines inside degemm-basic-omp.cpp and dgemm-blocked-omp.cpp where you can
add your code for doing OpenMP-enabled basic and blocked matrix multiply, respectively.

For blocked matrix multiply, in this implementation, the block size is being passed in as
a parameter from the main benchmark.cpp code. You should write your blocked matrix multiply
with the block size parameterized in this fashion (rather than being a hard-coded thing). 

# Changes to benchmark.cpp from HW2

This version of benchmark.cpp is modified to accept two command line arguments:

* -N nn   # sets the problem size  
* -B bb   # sets the block size

This change was made to make it possible to control these parameters from the command
line, which is handy for scripting up the test suite.

# Running the benchmarks

When you run cmake, it generates three bash script files that you may use on Perlmutter to
run the test battery for HW4. Some will require some modifications and customizations:

## Requesting specific  hardware performance counters

All configurations will require modification to set the specific LIKWID hardware performance
counter group you want to collect. 

The default group is PERF_COUNTER_GROUP=FLOPS_DP, which
is documented here: https://github.com/RRZE-HPC/likwid/blob/master/groups/zen3/FLOPS_DP.txt

If you want to collect different hardware performance counters, replace FLOPS_DP with the
name of the performance counter group you want to collect. likwid-perfctr -a will give
you a list of all the supported performance counter groups on the platform.

##  Problem configuration loops

* job-basic-omp : requires no modification, this one will run over the problem sizes and
OpenMP concurrency levels requested by HW4.

* job-blas : will require modification to change the loop over thread concurrency. After
running cmake, look inside job-blas for more details.

* job-blocked-openmp: requires 2 modifications to enable the loop over block sizes and
to pass the block size argument in to the benckmark-blocked-omp program. After running
cmake, look insize job-blocked-omp for more details.


#eof
