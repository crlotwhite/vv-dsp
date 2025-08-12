/**
 * @file simd_utils.h
 * @brief SIMD intrinsics abstraction and utilities for cross-platform optimization
 * @ingroup core
 *
 * This header provides a unified interface for SIMD operations across different
 * platforms and instruction sets. It abstracts platform-specific intrinsics
 * and provides fallback implementations for unsupported platforms.
 *
 * Supported instruction sets:
 * - x86/x64: SSE4.1, AVX2, AVX512
 * - ARM: NEON (future implementation)
 *
 * Memory alignment requirements:
 * - SSE4.1: 16-byte alignment
 * - AVX2: 32-byte alignment
 * - AVX512: 64-byte alignment
 */

#ifndef VV_DSP_SIMD_UTILS_H
#define VV_DSP_SIMD_UTILS_H

#include "../vv_dsp_types.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @name Platform Detection and Configuration */
/**@{*/

/* Platform and compiler detection */
#if defined(__GNUC__) || defined(__clang__)
    #define VV_DSP_SIMD_FORCE_INLINE __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
    #define VV_DSP_SIMD_FORCE_INLINE __forceinline
#else
    #define VV_DSP_SIMD_FORCE_INLINE inline
#endif

/* SIMD instruction set detection */
#ifdef VV_DSP_USE_SIMD

/* x86/x64 instruction sets */
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
    #define VV_DSP_SIMD_X86 1

    #if defined(VV_DSP_USE_SSE4_1) && (defined(__SSE4_1__) || (defined(_MSC_VER) && defined(__AVX__)))
        #define VV_DSP_SIMD_SSE41 1
        #include <smmintrin.h>
    #endif

    #if defined(VV_DSP_USE_AVX2) && (defined(__AVX2__) || (defined(_MSC_VER) && defined(__AVX2__)))
        #define VV_DSP_SIMD_AVX2 1
        #include <immintrin.h>
    #endif

    #if defined(VV_DSP_USE_AVX512) && (defined(__AVX512F__) || (defined(_MSC_VER) && defined(__AVX512F__)))
        #define VV_DSP_SIMD_AVX512 1
        #include <immintrin.h>
    #endif
#endif

/* ARM NEON detection */
#if defined(VV_DSP_USE_NEON) && (defined(__ARM_NEON) || defined(__aarch64__))
    #define VV_DSP_SIMD_NEON 1
    #include <arm_neon.h>
#endif

#endif /* VV_DSP_USE_SIMD */

/**@}*/

/** @name Memory Alignment Constants */
/**@{*/

#define VV_DSP_SIMD_ALIGN_SSE    16    ///< SSE/NEON alignment (128-bit)
#define VV_DSP_SIMD_ALIGN_AVX2   32    ///< AVX2 alignment (256-bit)
#define VV_DSP_SIMD_ALIGN_AVX512 64    ///< AVX512 alignment (512-bit)

/* Select default alignment based on best available instruction set */
#ifdef VV_DSP_SIMD_AVX512
    #define VV_DSP_SIMD_ALIGN_DEFAULT VV_DSP_SIMD_ALIGN_AVX512
#elif defined(VV_DSP_SIMD_AVX2)
    #define VV_DSP_SIMD_ALIGN_DEFAULT VV_DSP_SIMD_ALIGN_AVX2
#elif defined(VV_DSP_SIMD_SSE41) || defined(VV_DSP_SIMD_NEON)
    #define VV_DSP_SIMD_ALIGN_DEFAULT VV_DSP_SIMD_ALIGN_SSE
#else
    #define VV_DSP_SIMD_ALIGN_DEFAULT sizeof(vv_dsp_real)
#endif

/**@}*/

/** @name Vector Type Abstractions */
/**@{*/

#ifdef VV_DSP_SIMD_AVX512
typedef __m512 vv_dsp_simd_f32x16;  ///< 16 single-precision floats
typedef __m512d vv_dsp_simd_f64x8;  ///< 8 double-precision floats
#define VV_DSP_SIMD_F32_WIDTH 16
#define VV_DSP_SIMD_F64_WIDTH 8
#endif

#ifdef VV_DSP_SIMD_AVX2
typedef __m256 vv_dsp_simd_f32x8;   ///< 8 single-precision floats
typedef __m256d vv_dsp_simd_f64x4;  ///< 4 double-precision floats
#ifndef VV_DSP_SIMD_F32_WIDTH
#define VV_DSP_SIMD_F32_WIDTH 8
#define VV_DSP_SIMD_F64_WIDTH 4
#endif
#endif

#ifdef VV_DSP_SIMD_SSE41
typedef __m128 vv_dsp_simd_f32x4;   ///< 4 single-precision floats
typedef __m128d vv_dsp_simd_f64x2;  ///< 2 double-precision floats
#ifndef VV_DSP_SIMD_F32_WIDTH
#define VV_DSP_SIMD_F32_WIDTH 4
#define VV_DSP_SIMD_F64_WIDTH 2
#endif
#endif

#ifdef VV_DSP_SIMD_NEON
typedef float32x4_t vv_dsp_simd_f32x4;  ///< 4 single-precision floats (NEON)
typedef float64x2_t vv_dsp_simd_f64x2;  ///< 2 double-precision floats (NEON)
#ifndef VV_DSP_SIMD_F32_WIDTH
#define VV_DSP_SIMD_F32_WIDTH 4
#define VV_DSP_SIMD_F64_WIDTH 2
#endif
#endif

/* Fallback for scalar operations */
#ifndef VV_DSP_SIMD_F32_WIDTH
#define VV_DSP_SIMD_F32_WIDTH 1
#define VV_DSP_SIMD_F64_WIDTH 1
#endif

/* Select preferred vector type based on vv_dsp_real */
#if VV_DSP_REAL_IS_DOUBLE
    #define VV_DSP_SIMD_WIDTH VV_DSP_SIMD_F64_WIDTH
#else
    #define VV_DSP_SIMD_WIDTH VV_DSP_SIMD_F32_WIDTH
#endif

/**@}*/

/** @name Aligned Memory Allocation */
/**@{*/

/**
 * @brief Allocate aligned memory for SIMD operations
 * @param size Number of bytes to allocate
 * @param alignment Alignment requirement in bytes (must be power of 2)
 * @return Pointer to aligned memory, or NULL on failure
 *
 * @note The returned pointer must be freed with vv_dsp_aligned_free()
 * @note Alignment must be a power of 2 and >= sizeof(void*)
 */
void* vv_dsp_aligned_malloc(size_t size, size_t alignment);

/**
 * @brief Free memory allocated with vv_dsp_aligned_malloc()
 * @param ptr Pointer to aligned memory (may be NULL)
 */
void vv_dsp_aligned_free(void* ptr);

/**
 * @brief Allocate aligned memory using default SIMD alignment
 * @param size Number of bytes to allocate
 * @return Pointer to aligned memory, or NULL on failure
 */
VV_DSP_SIMD_FORCE_INLINE void* vv_dsp_aligned_malloc_default(size_t size) {
    return vv_dsp_aligned_malloc(size, VV_DSP_SIMD_ALIGN_DEFAULT);
}

/**
 * @brief Check if a pointer is aligned to the specified boundary
 * @param ptr Pointer to check
 * @param alignment Alignment requirement in bytes (must be power of 2)
 * @return 1 if aligned, 0 if not aligned
 */
VV_DSP_SIMD_FORCE_INLINE int vv_dsp_is_aligned(const void* ptr, size_t alignment) {
    return ((uintptr_t)ptr & (alignment - 1)) == 0;
}

/**
 * @brief Check if a pointer has default SIMD alignment
 * @param ptr Pointer to check
 * @return 1 if aligned, 0 if not aligned
 */
VV_DSP_SIMD_FORCE_INLINE int vv_dsp_is_simd_aligned(const void* ptr) {
    return vv_dsp_is_aligned(ptr, VV_DSP_SIMD_ALIGN_DEFAULT);
}

/**@}*/

/** @name Vector Load/Store Abstractions */
/**@{*/

/* The following macros provide a unified interface for vector operations
 * They will be expanded in the implementation files based on available SIMD */

#ifdef VV_DSP_USE_SIMD

/**
 * @brief Load vector from aligned memory
 * @param ptr Pointer to aligned memory (must be SIMD-aligned)
 * @return Vector register containing loaded data
 */
#define VV_DSP_SIMD_LOAD_ALIGNED(ptr) /* Implementation specific */

/**
 * @brief Load vector from unaligned memory
 * @param ptr Pointer to memory (any alignment)
 * @return Vector register containing loaded data
 */
#define VV_DSP_SIMD_LOAD_UNALIGNED(ptr) /* Implementation specific */

/**
 * @brief Store vector to aligned memory
 * @param ptr Pointer to aligned memory (must be SIMD-aligned)
 * @param vec Vector register to store
 */
#define VV_DSP_SIMD_STORE_ALIGNED(ptr, vec) /* Implementation specific */

/**
 * @brief Store vector to unaligned memory
 * @param ptr Pointer to memory (any alignment)
 * @param vec Vector register to store
 */
#define VV_DSP_SIMD_STORE_UNALIGNED(ptr, vec) /* Implementation specific */

/**
 * @brief Set all vector elements to zero
 * @return Zero vector
 */
#define VV_DSP_SIMD_ZERO() /* Implementation specific */

/**
 * @brief Set all vector elements to the same value
 * @param value Value to broadcast to all elements
 * @return Vector with all elements set to value
 */
#define VV_DSP_SIMD_SET1(value) /* Implementation specific */

#endif /* VV_DSP_USE_SIMD */

/**@}*/

/** @name Utility Macros */
/**@{*/

/**
 * @brief Calculate number of SIMD vectors needed for given array size
 * @param size Number of elements in array
 * @return Number of vectors needed (rounded up)
 */
#define VV_DSP_SIMD_VECTOR_COUNT(size) \
    (((size) + VV_DSP_SIMD_WIDTH - 1) / VV_DSP_SIMD_WIDTH)

/**
 * @brief Calculate number of elements that fit in whole SIMD vectors
 * @param size Number of elements in array
 * @return Number of elements that can be processed with SIMD (rounded down)
 */
#define VV_DSP_SIMD_ALIGNED_SIZE(size) \
    ((size) & ~(VV_DSP_SIMD_WIDTH - 1))

/**
 * @brief Calculate number of remaining elements after SIMD processing
 * @param size Number of elements in array
 * @return Number of elements that need scalar processing
 */
#define VV_DSP_SIMD_REMAINDER(size) \
    ((size) & (VV_DSP_SIMD_WIDTH - 1))

/**@}*/

/** @name Debug and Testing Support */
/**@{*/

#ifdef NDEBUG
    #define VV_DSP_SIMD_ASSERT_ALIGNED(ptr, alignment) ((void)0)
#else
    #define VV_DSP_SIMD_ASSERT_ALIGNED(ptr, alignment) \
        assert(vv_dsp_is_aligned(ptr, alignment) && \
               "Pointer must be aligned to " #alignment " bytes")
#endif

/**
 * @brief Get a human-readable string describing available SIMD features
 * @return Static string describing SIMD capabilities
 */
const char* vv_dsp_simd_get_features(void);

/**@}*/

#ifdef __cplusplus
}
#endif

#endif /* VV_DSP_SIMD_UTILS_H */
