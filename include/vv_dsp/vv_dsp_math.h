#ifndef VV_DSP_MATH_H
#define VV_DSP_MATH_H

#include "vv_dsp/vv_dsp_types.h"
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

// Double-precision PI for internal calculations
#ifndef VV_DSP_PI_D
#define VV_DSP_PI_D 3.141592653589793238462643383279502884
#endif

// Real-typed PI matching vv_dsp_real
#ifndef VV_DSP_PI
#define VV_DSP_PI ((vv_dsp_real)VV_DSP_PI_D)
#endif

#ifndef VV_DSP_TWO_PI_D
#define VV_DSP_TWO_PI_D (2.0 * VV_DSP_PI_D)
#endif

#ifndef VV_DSP_TWO_PI
#define VV_DSP_TWO_PI ((vv_dsp_real)VV_DSP_TWO_PI_D)
#endif

// Type-aware trig wrappers to match vv_dsp_real
#if defined(VV_DSP_USE_DOUBLE)
// Use double precision standard library functions
#  define VV_DSP_SIN(x) sin(x)
#  define VV_DSP_COS(x) cos(x)
#  define VV_DSP_EXP(x) exp(x)
#  define VV_DSP_SQRT(x) sqrt(x)
#  define VV_DSP_LOG(x) log(x)
#  define VV_DSP_TAN(x) tan(x)
#  define VV_DSP_POW(x, y) pow(x, y)
#  define VV_DSP_ATAN2(y, x) atan2(y, x)
#else
// Use single precision standard library functions
#  define VV_DSP_SIN(x) sinf(x)
#  define VV_DSP_COS(x) cosf(x)
#  define VV_DSP_EXP(x) expf(x)
#  define VV_DSP_SQRT(x) sqrtf(x)
#  define VV_DSP_LOG(x) logf(x)
#  define VV_DSP_TAN(x) tanf(x)
#  define VV_DSP_POW(x, y) powf(x, y)
#  define VV_DSP_ATAN2(y, x) atan2f(y, x)
#endif

#ifdef __cplusplus
} // extern "C"
#endif

// C++ specific math approximation overrides
#ifdef __cplusplus
#ifdef VV_DSP_USE_MATH_APPROX
#include "math_approx/math_approx.hpp"

// Override the C macros with C++ approximations in C++ context
#undef VV_DSP_SIN
#undef VV_DSP_COS
#undef VV_DSP_EXP
#undef VV_DSP_LOG
#undef VV_DSP_TAN

#define VV_DSP_SIN(x) ((vv_dsp_real)math_approx::sin_approx<7>((float)(x)))
#define VV_DSP_COS(x) ((vv_dsp_real)math_approx::cos_approx<7>((float)(x)))
#define VV_DSP_EXP(x) ((vv_dsp_real)math_approx::exp_approx<7>((float)(x)))
#define VV_DSP_LOG(x) ((vv_dsp_real)math_approx::log_approx<7>((float)(x)))
#define VV_DSP_TAN(x) ((vv_dsp_real)math_approx::tan_approx<7>((float)(x)))

// Note: POW and ATAN2 kept as standard for now - need specialized implementations
#endif // VV_DSP_USE_MATH_APPROX
#endif // __cplusplus

#endif // VV_DSP_MATH_H
