/**
 *  \file mergesort.cc
 *
 *  \brief Implement your mergesort in this file.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <omp.h>
#include <stdlib.h>



#include "sort.hh"

static int COUNT;


/**
 *
 * @param N
 * @param index_m: split the array into two arrays, the first array has index_m elements
 * @param A
 */
void merge(int N, int index_m, keytype* A, keytype* p)
{
    // keytype *p = newCopy(N, A);
    memcpy(p, A, N*sizeof(keytype));
    // keytype *p_r = newCopy(N-index_m, A+index_m);
    int nl = index_m;
    int nr = N - index_m;
    int index_l = 0;
    int index_r = 0;

    //merge begin
    for(int i = 0; i <= N; i++)
    {
        // if all the elements of the left part has been merged, then just merge the right hand
        if(index_l == nl)
        {
            memcpy(A+i, p+index_m+index_r, (nr-index_r)*sizeof(keytype));
            // while(index_r < nr)
            // {
            //     A[i] = p[index_m+index_r];
            //     i++;
            //     index_r++;
            // }
            break;
        }
        // if all the elements of the right part has been merged, then just merge the left hand
        else if(index_r == nr)
        {
            memcpy(A+i, p+index_l, (nl-index_l)*sizeof(keytype));
            // while(index_l < nl)
            // {
            //     A[i] = p[index_l];
            //     i++;
            //     index_l++;
            // }
            break;
        }
        else if(p[index_l] <= p[index_m+index_r])
        {
            A[i] = p[index_l];
            index_l++;
        }
        else if(p[index_l] > p[index_m+index_r])
        {
            A[i] = p[index_m+index_r];
            index_r++;
        }
    }
    // free(p);
    // p = NULL;
}


/**
 *
 * @param N
 * @param pivot
 * @param A
 * @return return the index that the first k elements less or equal than pivot
 */
int binarysearch(int N, int pivot, keytype * A)
{
    // loop ending condition: only has one element
    if(N == 1)
    {
        // if the only one element is less or equal than the pivot, then increment the index.
        if(A[0] <= pivot)
            return 1;
        return 0;
    }
    //split the array into 2 parts: the first part has mid elements; the second has N-mid elements
    int mid = N/2;
    if(pivot <= A[mid-1])
    {
        return  binarysearch(mid, pivot, A);
    }
    else if(pivot > A[mid-1])
    {
        // if in the second array, index need to add mid
        return binarysearch(N-mid, pivot, A+mid)+mid;
    }
}

/**
 *
 * @param N:
 * @param index_m: first array has index_m elements, the second has N-index_m elements
 * @param A
 */
void parallelMerge(int N, int index_m, keytype* A, keytype* p)
{
    /**
     * consider the condition that if one array of the left and right array is empty
     */
    // if(index_m==0 || N==index_m)
    //     return;
    // /**
    //  * consider the condition that if the left array just has one element, and the elements of the right array
    //  * are all less or equal than it, if not specify this condition, the loop will not end
    //  */
    // else if(index_m == 1 && N > index_m)
    // {
    //     if(A[0] >= A[N-1])
    //     {
    //         keytype *p = newCopy(N, A);
    //         memcpy(A, p+1, (N-1) * sizeof(keytype));
    //         memcpy(A+N-1, p, sizeof(keytype));
    //         free(p);
    //         p = NULL;
    //         return;
    //     }
    // }

    // in case of the mass of the context switching time to accumulate, stop recersively call the function if the number of the element is less than 1/8 of total elements.
    if(N < COUNT/8)
    {
        merge(N, index_m, A, p);
        return;
    }

    /**
     * index_m_l: the first index_m_l elements of left array with value less or equal to pivot
     * if there is just 1 element in the first array, let it be the pivot
     */
    int index_m_l = (index_m/2==0 ? 1 : index_m/2);
    keytype pivot = A[index_m_l-1];

    /**
     * index_m_r: the first index_m_r elements of right array with value less or equal to pivot
     */
    int index_m_r = binarysearch(N-index_m, pivot, A+index_m);

//    printf("\n\nprevious array\n");
//    printArray(A, N);
//    printf("pivot is %d\n", pivot);
//    printf("N is %d, index_l is %d, index_m is %d, index_r is %d\n", N, index_m_l, index_m, index_m_r);

    // keytype *p = newCopy(N, A);
    memcpy(p+index_m_l, A+index_m_l, (index_m - index_m_l)*sizeof(keytype));
    memcpy(A+index_m_l, A+index_m, sizeof(keytype) * index_m_r);
    memcpy(A+index_m_l+index_m_r, p+index_m_l, sizeof(keytype) * (index_m - index_m_l));
    // free(p);
//    printf("after array\n");
//    printArray(A, N);
    #pragma omp task
    {
        parallelMerge(index_m_l+index_m_r, index_m_l, A, p);
    }
    parallelMerge(N-index_m_l-index_m_r, index_m-index_m_l, A+index_m_l+index_m_r, p+index_m_l+index_m_r);
    #pragma omp taskwait
}




void mergeSort (int N, keytype* A, keytype* p)
{
    const int G = 1024; /* base case size, a tuning parameter */
    if (N < G)
        sequentialSort (N, A);
    else {
        // index_m: split the array into 2 arrays, the first array has index_m elements, the second has N-index_m elements
        int index_m = N/2;
        #pragma omp task shared(A, p, index_m)
        {
            mergeSort(index_m, A, p);
        }
        mergeSort(N-index_m, A+index_m, p+index_m);
        #pragma omp taskwait
        parallelMerge(N, index_m, A, p);
        // merge(N, index_m, A, p);
    }
}

void mySort (int N, keytype* A)
{
   
 /* Lucky you, you get to start from scratch */
    keytype  *p = newKeys(N);

    COUNT = N;

    omp_set_num_threads(8);
    #pragma omp parallel
    {
        #pragma omp single nowait
        {
            mergeSort(N, A, p);
        }
    }
    free(p);
    p = NULL;
}

/* eof */
