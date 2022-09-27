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
this condition is true on Cori@NERSC and on the class VM. It is also true for some
other platforms, but you are on your own if using a platform other than Cori@NERSC
or the class VM.

Cmake also needs to be able to find LIKWID. This condition is satisfied on Cori when
you first load the likwid/5.2.0 module, or on the VM when you load the likwid-5.2.0
module:  
* module load likwid/5.2.0   # on cori  
* module load likwid-5.2.0   # on the VM

Cori@NERSC update 9/27/2022:

Prior to running the module load command, please manually modify your MODULEPATH environment variable as follows

   bash users:
	% export MODULEPATH=/project/projectdirs/m3930/modulefiles:$MODULEPATH

	 csh users
	% setenv MODULEPATH /project/projectdirs/m3930/modulefiles:$MODULEPATH


# Build instructions - general

After downloading, cd into the main source directly, then:

% mkdir build  
% cd build  
% cmake ../  -Wno-dev

When building on Cori, make sure you are on a KNL node when doing the compilation. The
Cori login nodes are *not* KNL nodes, the Cori login nodes have Intel Xeon E5-2698
processors, not the Intel Xeon Phi 7250 processors.  The simplest way to do this is
grab an interactive KNL node:
salloc --nodes 1 --qos interactive --time 01:00:00 --constraint knl --account m3930

# Platforms

Due to the dependency on LIKWID and LIKWID's dependency on the Linux MSR kernel
module, the best bet for this assignment is to use Cori@NERSC.

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

When you run cmake, it generates three bash script files that you may use on Cori to
run the test battery for HW4. Some will require some modifications and customizations:

## Requesting specific  hardware performance counters

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

salloc --nodes=1 --qos=interactive --time=01:00:00 --constraint=knl --account=m3930 --perf=likwid

There may be a similar option for use with sbatch commands, but documentation is elusive.
Suggest doing all interactive runs.

You may request and use an interactive node for up to 4 hours at a time.


#eof
