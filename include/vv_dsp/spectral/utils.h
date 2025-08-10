#ifndef VV_DSP_SPECTRAL_UTILS_H
#define VV_DSP_SPECTRAL_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "vv_dsp/vv_dsp_types.h"

// Out-of-place fftshift/ifftshift utilities (1D)
vv_dsp_status vv_dsp_fftshift_real(const vv_dsp_real* in, vv_dsp_real* out, size_t n);
vv_dsp_status vv_dsp_ifftshift_real(const vv_dsp_real* in, vv_dsp_real* out, size_t n);
vv_dsp_status vv_dsp_fftshift_cpx(const vv_dsp_cpx* in, vv_dsp_cpx* out, size_t n);
vv_dsp_status vv_dsp_ifftshift_cpx(const vv_dsp_cpx* in, vv_dsp_cpx* out, size_t n);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // VV_DSP_SPECTRAL_UTILS_H
