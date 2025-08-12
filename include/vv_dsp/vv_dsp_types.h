/**
 * @file vv_dsp_types.h
 * @brief Core type definitions and macros for VV-DSP library
 * @ingroup core_group
 *
 * This file contains fundamental type definitions, macros, and basic data structures
 * used throughout the VV-DSP library. It provides the foundation for all DSP operations.
 */

#ifndef VV_DSP_TYPES_H
#define VV_DSP_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/**
 * @defgroup core_group Core Types and Utilities
 * @brief Fundamental types, macros, and basic mathematical operations
 *
 * The core module provides essential data types, utility macros, and basic
 * mathematical operations that form the foundation of the VV-DSP library.
 *
 * @{
 */

/** @name Compiler Compatibility Macros
 * @{
 */

/**
 * @brief Portable inline function macro
 * @details Provides cross-platform inline function support for different compilers
 */
#ifndef VV_DSP_INLINE
#  if defined(_MSC_VER)
#    define VV_DSP_INLINE __inline
#  else
#    define VV_DSP_INLINE inline
#  endif
#endif

/**
 * @brief Warn-unused-result attribute macro
 * @details Enables compiler warnings when function return values are ignored
 * @note Only available on GCC and Clang compilers
 */
#ifndef VV_DSP_NODISCARD
#  if defined(__GNUC__) || defined(__clang__)
#    define VV_DSP_NODISCARD __attribute__((warn_unused_result))
#  else
#    define VV_DSP_NODISCARD
#  endif
#endif
/** @} */

/** @name Precision Configuration
 * @{
 */

/**
 * @brief Real scalar type for DSP operations
 * @details Can be either float (default) or double precision depending on VV_DSP_USE_DOUBLE
 *
 * When VV_DSP_USE_DOUBLE is defined, uses double precision arithmetic.
 * Otherwise defaults to single precision (float) for better performance.
 */
#ifdef VV_DSP_USE_DOUBLE
typedef double vv_dsp_real;
#else
typedef float vv_dsp_real;
#endif
/** @} */

/** @name Complex Number Support
 * @{
 */

/**
 * @brief Complex number representation
 * @details Simple structure representing a complex number with real and imaginary parts
 *
 * This structure provides a portable way to represent complex numbers across
 * different platforms without relying on C99 complex.h support.
 */
typedef struct vv_dsp_cpx {
    vv_dsp_real re; /**< Real part of the complex number */
    vv_dsp_real im; /**< Imaginary part of the complex number */
} vv_dsp_cpx;

/**
 * @brief Create a complex number from real and imaginary parts
 * @param re Real part
 * @param im Imaginary part
 * @return Complex number with specified real and imaginary components
 *
 * @code{.c}
 * vv_dsp_cpx z = vv_dsp_cpx_make(3.0, 4.0);  // Creates 3+4i
 * @endcode
 */
static VV_DSP_INLINE vv_dsp_cpx vv_dsp_cpx_make(vv_dsp_real re, vv_dsp_real im) {
    vv_dsp_cpx z; z.re = re; z.im = im; return z;
}
/** @} */

/** @name Error Handling
 * @{
 */

/**
 * @brief Status codes for VV-DSP operations
 * @details Enumeration of possible return values from VV-DSP functions
 *
 * All VV-DSP functions that can fail return one of these status codes.
 * VV_DSP_OK indicates successful operation, while other values indicate
 * specific error conditions.
 */
typedef enum vv_dsp_status {
    VV_DSP_OK = 0,                  /**< Operation completed successfully */
    VV_DSP_ERROR_NULL_POINTER = 1,  /**< One or more required pointers are NULL */
    VV_DSP_ERROR_INVALID_SIZE = 2,  /**< Invalid size parameter (e.g., zero or negative) */
    VV_DSP_ERROR_OUT_OF_RANGE = 3,  /**< Parameter value is outside valid range */
    VV_DSP_ERROR_INTERNAL = 4,      /**< Internal library error */
    VV_DSP_ERROR_NAN_INF = 5        /**< NaN or Inf encountered when policy is ERROR */
} vv_dsp_status;
/** @} */

/** @name Compile-time Assertions
 * @{
 */

/**
 * @brief Static assertion macro for compile-time checks
 * @param cond Condition to check at compile time
 * @param msg Message identifier for the assertion
 *
 * @details Provides compile-time assertions using C11 _Static_assert when available,
 * or falls back to a typedef trick for older C standards.
 */
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#  define VV_DSP_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#else
#  define VV_DSP_STATIC_ASSERT(cond, msg) typedef char vv_dsp_static_assert_##msg[(cond)?1:-1]
#endif

// Compile-time size validation
VV_DSP_STATIC_ASSERT(sizeof(vv_dsp_cpx) == sizeof(vv_dsp_real)*2, cpx_size_must_be_2x_real);
/** @} */

/** @} */ // End of core_group

#ifdef __cplusplus
} // extern "C"
#endif

#endif // VV_DSP_TYPES_H
