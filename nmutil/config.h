#ifndef NMUTIL_CONFIG_H
#define NMUTIL_CONFIG_H

#ifdef __CUDA_ARCH__
#define NM_SPECIFIERS __device__ __host__
#define NM_VAR_SPECIFIER __device__
#else
#define NM_SPECIFIERS
#define NM_VAR_SPECIFIER
#endif

#endif // NMUTIL_CONFIG_H