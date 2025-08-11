#ifndef VV_DSP_SPECTRAL_H
#define VV_DSP_SPECTRAL_H

#ifdef __cplusplus
extern "C" {
#endif

int vv_dsp_spectral_dummy(void);

// FFT public API
#include "vv_dsp/spectral/fft.h"
// Spectral utilities (fftshift/ifftshift)
#include "vv_dsp/spectral/utils.h"

// Spectral utilities
#include "vv_dsp/spectral/utils.h"

// STFT/ISTFT public API
#include "vv_dsp/spectral/stft.h"

// DCT public API
#include "vv_dsp/spectral/dct.h"

// CZT public API
#include "vv_dsp/spectral/czt.h"

#ifdef __cplusplus
}
#endif

#endif // VV_DSP_SPECTRAL_H
