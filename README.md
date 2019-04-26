# Parallel_mergesort_quicksort

In the Repo, I implement the mergesort and quicksort in parallel way with openmp.
In both sorting algorithm. The are two level of parallelism
- first level is the recursive call of the sort function.
- second level
   - for mergesort -> parallel the merge process
   - for quicksort -> parallel the partition process
