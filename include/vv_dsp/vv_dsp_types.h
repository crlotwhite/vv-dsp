#ifndef VV_DSP_TYPES_H
#define VV_DSP_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

// Inline macro (portable)
#ifndef VV_DSP_INLINE
#  if defined(_MSC_VER)
#    define VV_DSP_INLINE __inline
#  else
#    define VV_DSP_INLINE inline
#  endif
#endif

// Warn-unused-result attribute (best-effort)
#ifndef VV_DSP_NODISCARD
#  if defined(__GNUC__) || defined(__clang__)
#    define VV_DSP_NODISCARD __attribute__((warn_unused_result))
#  else
#    define VV_DSP_NODISCARD
#  endif
#endif

// Real scalar type
#ifdef VV_DSP_USE_DOUBLE
typedef double vv_dsp_real;
#else
typedef float vv_dsp_real;
#endif

// Complex scalar type
typedef struct vv_dsp_cpx {
    vv_dsp_real re;
    vv_dsp_real im;
} vv_dsp_cpx;

// Small helper to construct complex value
static VV_DSP_INLINE vv_dsp_cpx vv_dsp_cpx_make(vv_dsp_real re, vv_dsp_real im) {
    vv_dsp_cpx z; z.re = re; z.im = im; return z;
}

// Status codes
typedef enum vv_dsp_status {
    VV_DSP_OK = 0,
    VV_DSP_ERROR_NULL_POINTER = 1,
    VV_DSP_ERROR_INVALID_SIZE = 2,
    VV_DSP_ERROR_OUT_OF_RANGE = 3,
    VV_DSP_ERROR_INTERNAL = 4
} vv_dsp_status;

// Static assert macro for C99/C11
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#  define VV_DSP_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#else
#  define VV_DSP_STATIC_ASSERT(cond, msg) typedef char vv_dsp_static_assert_##msg[(cond)?1:-1]
#endif

// Basic sanity size checks
VV_DSP_STATIC_ASSERT(sizeof(vv_dsp_cpx) == sizeof(vv_dsp_real)*2, cpx_size_must_be_2x_real);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // VV_DSP_TYPES_H
