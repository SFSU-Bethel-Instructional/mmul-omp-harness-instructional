//
// (C) 2021, E. Wes Bethel
// benchmark-* hardness for running different versions of matrix multiply
//    over different problem sizes
//
// usage: [-N problemSizeInt] [-B blockSizeInt]
// 
// On the command line you may optionally set the problem size (-N problemSizeInt),
// as well as optionally set the block size (-B blockSizeInt).
//
// If you specify nothing on the command line, the benchmark will iterate through a 
// prescribed set of problem sizes, which are defined in the code below.
//
// For the blocked version, if you don't specify a block size on the command line,
// then the benchmark will iterate of a prescribed set of block sizes, which are
// defined in the code below.

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

#include <cmath> // For: fabs

#include <cblas.h>
#include <string.h>
#include <unistd.h>
#include <omp.h>

#include "likwid-stuff.h"

// external definitions for mmul's
extern void square_dgemm(int, double*, double*, double*);
extern void square_dgemm_blocked(int, int, double*, double*, double*) ;
extern const char* dgemm_desc;

void reference_dgemm(int n, double alpha, double* A, double* B, double* C) {
    cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, n, n, n, alpha, A, n, B, n, 1., C, n);
}

void fill(double* p, int n) {
    static std::random_device rd;
    static std::default_random_engine gen(rd());
    static std::uniform_real_distribution<> dis(-1.0, 1.0);
    for (int i = 0; i < n; ++i)
        p[i] = 2 * dis(gen) - 1;
}

bool check_accuracy(double *A, double *Anot, int nvalues)
{
  double eps = 1e-5;
  for (size_t i = 0; i < nvalues; i++) 
  {
    if (fabsf(A[i] - Anot[i]) > eps) {
       return false;
    }
  }
  return true;
}


/* The benchmarking program */
int main(int argc, char** argv) 
{
   std::cout << "Description:\t" << dgemm_desc << std::endl << std::endl;

   // check to see if there is anything on the command line:
   // -N nnnn    to define the problem size
   // -B bbbb    to define the block size
   int cmdline_N = -1; 
   int cmdline_B = -1;
   int c;

   while ( (c = getopt(argc, argv, "N:B:")) != -1) {
      switch(c) {
         case 'N':
            cmdline_N = std::atoi(optarg == NULL ? "-999" : optarg);
            // std::cout << "Command line problem size: " << cmdline_N << std::endl;
            break;
#ifdef BLOCKED
         case 'B':
            cmdline_B = std::atoi(optarg == NULL ? "-999" : optarg);
            // std::cout << "Command line block size: " << cmdline_B << std::endl;
            break;
#endif
      }
   }

   // initialize the LIKWID marker API in a serial code region once in the beginning
   LIKWID_MARKER_INIT;

#pragma omp parallel
   {
      // ID of the thread in the current team
      int thread_id = omp_get_thread_num();
      // Number of threads in the current team
      int nthreads = omp_get_num_threads();

#pragma omp critical
      {
         std::cout << "Hello world, I'm thread " << thread_id << " out of " << nthreads << " total threads. " << std::endl; 
      }

      // Each thread must add itself to the Marker API, therefore must be
      // in parallel region
      LIKWID_MARKER_THREADINIT;
      // Register region name
      LIKWID_MARKER_REGISTER(MY_MARKER_REGION_NAME);
   }

   std::cout << std::fixed << std::setprecision(4);


   // set up the problem sizes
   int default_problem_sizes[] = {128, 512, 2048};
   std::vector<int> test_sizes;

   if (cmdline_N > 0)
      test_sizes.push_back(cmdline_N);
   else
   {
      for (int i : default_problem_sizes)
         test_sizes.push_back(i);
   }

   int n_problems = test_sizes.size();

#ifdef BLOCKED
   // set up the block sizes
   int default_block_sizes[] = {4, 16, 64};
   std::vector<int> block_sizes;

   if (cmdline_B > 0)
      block_sizes.push_back(cmdline_B);
   else
   {
      for (int i : default_block_sizes)
         block_sizes.push_back(i);
   }
#endif

   /* For each test size */
   for (int n : test_sizes) 
   {
      printf("Working on problem size N=%d \n", n);

#ifdef BLOCKED
      printf("Blocked DGEMM \n");
      for (int b : block_sizes)
      {
         printf(" Working on Block size = %d \n", b);
#endif

         // allocate memory for 6 NxN matrics
         std::vector<double> buf(6 * n * n);
         double* A = buf.data() + 0;
         double* B = A + n * n;
         double* C = B + n * n;
         double* Acopy = C + n * n;
         double* Bcopy = Acopy + n * n;
         double* Ccopy = Bcopy + n * n;

         // load up matrics with some random numbers
         fill(A, n * n);
         fill(B, n * n);
         fill(C, n * n);

         // make copies of A, B, C for use in verification of results
         memcpy((void *)Acopy, (const void *)A, sizeof(double)*n*n);
         memcpy((void *)Bcopy, (const void *)B, sizeof(double)*n*n);
         memcpy((void *)Ccopy, (const void *)C, sizeof(double)*n*n);

         // insert timer code here
         std::chrono::time_point<std::chrono::high_resolution_clock> start_time = std::chrono::high_resolution_clock::now();

#ifdef BLOCKED
         square_dgemm_blocked(n, b, A, B, C); 
#else
         square_dgemm(n, A, B, C); 
#endif

         // insert timer code here
         std::chrono::time_point<std::chrono::high_resolution_clock> end_time = std::chrono::high_resolution_clock::now();

         std::chrono::duration<double> elapsed = end_time - start_time;

         std::cout << " Elapsed time is : " << elapsed.count() << " (sec) " << std::endl;

         reference_dgemm(n, 1.0 , Acopy, Bcopy, Ccopy);

         // compare your C with that computed by BLAS
         if (check_accuracy(Ccopy, C, n*n) == false)
            printf(" Error: your answer is not the same as that computed by BLAS. \n");

#ifdef BLOCKED
      } // end loop over block sizes
#endif

   } // end loop over problem sizes

   // Close Marker API and write results to file for further evaluation done
   // by likwid-perfctr
   LIKWID_MARKER_CLOSE;

   return 0;
}

// EOF

// EOF
