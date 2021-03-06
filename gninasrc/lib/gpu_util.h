#ifndef GPU_UTIL_H
#define GPU_UTIL_H

#include <stdio.h>

static inline void abort_on_gpu_err(void){
    cudaError err = cudaGetLastError();
    if (cudaSuccess != err)
    {
        fprintf(stderr, "cudaCheckError() failed at %s:%i : %s\n",
                __FILE__, __LINE__, cudaGetErrorString(err));
        exit(-1);
    }
}

// CUDA: various checks for different function calls.
#define CUDA_CHECK_GNINA(condition) \
  /* Code block avoids redefinition of cudaError_t error */ \
  do { \
    cudaError_t error = condition; \
    if(error != cudaSuccess) { std::cerr << " " << cudaGetErrorString(error); abort(); } \
  } while (0)


#define CUDA_THREADS_PER_BLOCK (512)

// CUDA: number of blocks for N threads with nthreads per block
inline int CUDA_GET_BLOCKS(const int N, const int nthreads) {
  return (N + nthreads - 1) / nthreads;
}

//round N up to a multiple of 32
inline int ROUND_TO_WARP(int N) {
	if(N % 32) {
		return ((N/32)+1)*32;
	}
	return N;
}

#endif
