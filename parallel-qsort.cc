/**
 *  \file parallel-qsort.cc
 *
 *  \brief Implement your parallel quicksort algorithm in this file.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <omp.h>
#include "sort.hh"
/**
 *   Given a pivot value, this routine partitions a given input array
 *   into two sets: the set A_le, which consists of all elements less
 *   than or equal to the pivot, and the set A_gt, which consists of
 *   all elements strictly greater than the pivot.
 *
 *   This routine overwrites the original input array with the
 *   partitioned output. It also returns the index n_le such that
 *   (A[0:(k-1)] == A_le) and (A[k:(N-1)] == A_gt).
 */
 
static int COUNT;

int partition (keytype pivot, int N, keytype* A)
{
  int k = 0;
  for (int i = 0; i < N; ++i)
  {
    /* Invariant:
     * - A[0:(k-1)] <= pivot; and
     * - A[k:(i-1)] > pivot
     */
    const int ai = A[i];
    if (ai <= pivot) {
      /* Swap A[i] and A[k] */
      int ak = A[k];
      A[k++] = ai;
      A[i] = ak;
    }
  }
  return k;
}
 
 
int parallelPartition(keytype pivot, int N, keytype* A, keytype* p)
{
  // if(N == 1)
  // {
  //   if(A[0] <= pivot)
  //   {
  //     return 1;
  //   }
  //   else
  //     return 0;
  // }
  
  
  // in case of the mass of the context switching time to accumulate, stop recersively call the function if the number of the element is less than 1/8 of total elements.
  if(N < COUNT/8)
  {
    return partition(pivot, N, A);
  }

  int k = N/2;
  int n_less_1;
  int n_less_2;

  // printf("Total %d threads running now\n", omp_get_num_threads());
  #pragma omp task shared(n_less_1)
  {
    n_less_1 = parallelPartition(pivot, k, A, p);
    // printf("thread: %d\tvalue of n_less_l inside the opm task is %d\n", omp_get_thread_num(), n_less_1);
  }

  n_less_2 = parallelPartition(pivot, N-k, A+k, p+k);
  #pragma omp taskwait
  // printf("thread : %d\tvalue of n_less_l outside the opm task is %d\n", omp_get_thread_num(), n_less_1);
  memcpy(p+n_less_1, A+n_less_1, sizeof(keytype)*(k-n_less_1));
  memcpy(A+n_less_1, A+k, sizeof(keytype)*n_less_2);
  memcpy(A+n_less_1+n_less_2, p+n_less_1, sizeof(keytype)*(k-n_less_1));
  
  return n_less_1+n_less_2;
}




void quickSort (int N, keytype* A, keytype* p)
{
  const int G = 1024; /* base case size, a tuning parameter */
  if (N < G)
    sequentialSort (N, A);
  else {
    // Choose pivot at random
    keytype pivot = A[rand () % N];
    // Partition around the pivot. Upon completion, n_less, n_equal,
    // and n_greater should each be the number of keys less than,
    // equal to, or greater than the pivot, respectively. Moreover, the array
    int n_le;
    
    // n_le = partition(pivot, N, A);
   
    n_le = parallelPartition(pivot, N, A, p);
    
    #pragma omp task
    quickSort (n_le, A, p);
    quickSort (N-n_le, A + n_le, p+n_le);
    #pragma omp taskwait
  }
}

void mySort (int N, keytype* A)
{
  COUNT = N;
  keytype  *p = newKeys(N);

  omp_set_num_threads(8);

  #pragma omp parallel
  {
    #pragma omp single nowait
    {
      quickSort (N, A, p);
    }
  }

  free(p);
  p = NULL;
}

/* eof */
