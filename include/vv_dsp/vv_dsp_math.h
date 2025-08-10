#ifndef VV_DSP_MATH_H
#define VV_DSP_MATH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vv_dsp/vv_dsp_types.h"
#include <math.h>

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
#  define VV_DSP_SIN(x) sin(x)
#  define VV_DSP_COS(x) cos(x)
#else
#  define VV_DSP_SIN(x) sinf(x)
#  define VV_DSP_COS(x) cosf(x)
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif // VV_DSP_MATH_H
