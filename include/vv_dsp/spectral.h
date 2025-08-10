#ifndef VV_DSP_SPECTRAL_H
#define VV_DSP_SPECTRAL_H

#ifdef __cplusplus
extern "C" {
#endif

int vv_dsp_spectral_dummy(void);

// FFT public API
#include "vv_dsp/spectral/fft.h"

// Spectral utilities
#include "vv_dsp/spectral/utils.h"

#ifdef __cplusplus
}
#endif

#endif // VV_DSP_SPECTRAL_H
