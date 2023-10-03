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

While it is possible to compile and run codes without explicitly making calls to LIKWID's
Marker API, doing so will result in obtaining performance counter data via likwid-perfctr 
for the entire application rather than just your MMUL.


# Build instructions - general

After downloading, cd into the main source directly, then:

    mkdir build  
    cd build  
    cmake ../  -Wno-dev

This type of build will NOT include LIKWID, and you will probably see information in the cmake
output about LIKWID not being found. The code should compile and run but will not make use of 
LIKWID's Marker API.

# Configuring to use LIKWID on the VM

Cmake also needs to be able to find LIKWID. This condition is satisfied on the VM 
when you load the likwid-5.2.2 module:  

    module load likwid-5.2.2   # on the VM

# Configufing to use LIKWID on Perlmutter

 **Note: As of the time of this writing, 3 Oct 2023, LIKWID on Perlmutter is broken. NERSC consultants have
been engaged and hopefully we will have a solution soon.**


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

When you run cmake, it generates three bash script files that you may use on Cori to
run the test battery for HW4. Some will require some modifications and customizations:

## Requesting specific  hardware performance counters

  **NOTE: 3 Oct 2023:  This section is presently out of date. Once LIKWID is working again
on Perlmutter, this section will be updated.**

All configurations will require modification to set the specific LIKWID hardware performance
counter group you want to collect. 


The default group is PERF_COUNTER_GROUP=HBM_CACHE, which
is documented here: https://github.com/RRZE-HPC/likwid/blob/master/groups/knl/HBM_CACHE.txt

If you want to collect different hardware performance counters, replace HBM_CACHE with the
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

## Requesting a KNL interactive node for doing runs

An "extra step" is required for setting up the kernel environment for running LIKWID-enabled codes to collect hardware performance counters.

Modify your salloc command when requesting a KNL node by adding "--perf=likwid" as follows:

 **Note 3 Oct 2023: the following salloc command is out of date. It will be updated once
LIKWID is working again on Perlmutter**

salloc --nodes=1 --qos=interactive --time=01:00:00 --constraint=knl --account=m3930 --perf=likwid

There may be a similar option for use with sbatch commands, but documentation is elusive.
Suggest doing all interactive runs.

You may request and use an interactive node for up to 4 hours at a time.


#eof
