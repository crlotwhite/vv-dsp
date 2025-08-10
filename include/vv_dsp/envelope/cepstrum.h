#ifndef VV_DSP_ENVELOPE_CEPSTRUM_H
#define VV_DSP_ENVELOPE_CEPSTRUM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "vv_dsp/vv_dsp_types.h"

// Compute real cepstrum from real signal of length n using spectral log magnitude
// out_cep length n (same length), simple naive implementation
vv_dsp_status vv_dsp_cepstrum_real(const vv_dsp_real* x, size_t n, vv_dsp_real* out_cep);

// Inverse real cepstrum (minimum phase reconstruction) from cepstrum array
// Writes a min-phase time-domain signal approximation into out_x (length n)
vv_dsp_status vv_dsp_icepstrum_minphase(const vv_dsp_real* c, size_t n, vv_dsp_real* out_x);

#ifdef __cplusplus
}
#endif

#endif // VV_DSP_ENVELOPE_CEPSTRUM_H
