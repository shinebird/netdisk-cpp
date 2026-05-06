#pragma once

#define FUNC_CONST_INPUT
#define FUNC_OUTPUT
#define NETDISK_ARRAY_ARG

// [[likely]], [[unlikely]] is supported since C++20

// macro PURE_FUNCTION conflicts with libtorch

#if !(defined(_MSC_VER) && !defined(__clang__))
    #define NETDISK_PURE_FUNCTION __attribute__((pure))
    #define LIKELY(expr) __builtin_expect(!!(expr), true)
    #define UNLIKELY(expr) __builtin_expect(!!(expr), false)
    #define NO_RETURNS_ALIAS __attribute__((malloc))
    #define PRETTY_FUNCTION_NAME __PRETTY_FUNCTION__
    #define FLATTEN_FUNCTION __attribute__((flatten))
    #define HOT_FUNCTION __attribute__((hot))
    #define COLD_FUNCTION __attribute__((cold))
#else
    #define NETDISK_PURE_FUNCTION __declspec(noalias)
    #define LIKELY(expr) (expr)
    #define UNLIKELY(expr) (expr)
    #define NO_RETURNS_ALIAS __declspec(restrict)
    #define PRETTY_FUNCTION_NAME __FUNCSIG__
    #define FLATTEN_FUNCTION [[msvc::flatten]]
    #define HOT_FUNCTION
    #define COLD_FUNCTION
#endif

#if defined(_MSC_VER)
    // __attribute__((always_inline)) seems to be no effect in clang-cl
    #define ALWAYS_INLINE __forceinline
#else
    #define ALWAYS_INLINE __attribute__((always_inline))
#endif

// Pre-C++23 [[assume(expr)]]

#if __cplusplus >= 202302L
    #define ASSUME(expr) [[assume(expr)]]
#elif defined(__clang__)
    #define ASSUME(expr) __builtin_assume(expr)
#elif defined(_MSC_VER)
    #define ASSUME(expr) __assume(expr)
#else
    #define ASSUME(expr) __attribute__((assume(expr)))
#endif

#define OPENMP_4_0 201307
#if _OPENMP > OPENMP_4_0
    #define NETDISK_OMP_PARALLEL_FOR_SIMD _Pragma("omp parallel for simd")
    #define NETDISK_OMP_FOR_SIMD _Pragma("omp for simd")
#else
    #define NETDISK_OMP_PARALLEL_FOR_SIMD _Pragma("omp parallel for")
    #define NETDISK_OMP_FOR_SIMD
#endif

#define NETDISK_CUDA_INLINE

#ifndef __CUDACC__
    #define __host__
    #define __device__
#endif
