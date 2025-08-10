#ifndef VV_DSP_ENVELOPE_MINPHASE_H
#define VV_DSP_ENVELOPE_MINPHASE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "vv_dsp/vv_dsp_types.h"

// Construct minimum-phase spectrum from real cepstrum using homomorphic processing
// in: cepstrum c[n]
// out: complex spectrum H[k] of length n (full complex C2C), consistent with STFT/FFT API
vv_dsp_status vv_dsp_minphase_from_cepstrum(const vv_dsp_real* c, size_t n, vv_dsp_cpx* out_spec);

#ifdef __cplusplus
}
#endif

#endif // VV_DSP_ENVELOPE_MINPHASE_H
