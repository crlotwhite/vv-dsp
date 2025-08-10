#ifndef VV_DSP_RESAMPLE_INTERPOLATE_H
#define VV_DSP_RESAMPLE_INTERPOLATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vv_dsp/vv_dsp_types.h"

// Linear interpolation over a real-valued sequence.
// Given an input array x[0..n-1] and a fractional index pos in [0, n-1],
// computes y = (1-t)*x[i] + t*x[i+1], where i = floor(pos), t = pos - i.
// Edge cases:
//  - If pos <= 0, returns x[0]
//  - If pos >= n-1, returns x[n-1]
// Returns VV_DSP_OK on success, or an error code on invalid arguments.
vv_dsp_status vv_dsp_interpolate_linear_real(const vv_dsp_real* x,
                                             size_t n,
                                             vv_dsp_real pos,
                                             vv_dsp_real* out);

// Cubic interpolation over a real-valued sequence using a Catmull-Rom style
// scheme with clamped boundary handling. Uses samples around floor(pos):
// x[i-1], x[i], x[i+1], x[i+2] (clamped within bounds). Returns VV_DSP_OK on
// success.
vv_dsp_status vv_dsp_interpolate_cubic_real(const vv_dsp_real* x,
                                            size_t n,
                                            vv_dsp_real pos,
                                            vv_dsp_real* out);

#ifdef __cplusplus
}
#endif

#endif // VV_DSP_RESAMPLE_INTERPOLATE_H
