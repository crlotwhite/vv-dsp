#ifndef VV_DSP_ENVELOPE_H
#define VV_DSP_ENVELOPE_H

#ifdef __cplusplus
extern "C" {
#endif

// Public Envelope APIs
// Real cepstrum, inverse cepstrum (minimum-phase reconstruction),
// and LPC-based spectral envelope estimation

#include "vv_dsp/envelope/cepstrum.h"
#include "vv_dsp/envelope/minphase.h"
#include "vv_dsp/envelope/lpc.h"

// Legacy dummy function kept for compatibility with existing sanity test
int vv_dsp_envelope_dummy(void);

#ifdef __cplusplus
}
#endif

#endif // VV_DSP_ENVELOPE_H
