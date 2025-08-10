#ifndef VV_DSP_SPECTRAL_STFT_H
#define VV_DSP_SPECTRAL_STFT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "vv_dsp/vv_dsp_types.h"
#include "vv_dsp/spectral/fft.h"

// Opaque STFT handle
typedef struct vv_dsp_stft vv_dsp_stft;

// Window type for STFT analysis/synthesis
typedef enum vv_dsp_stft_window {
    VV_DSP_STFT_WIN_BOXCAR = 0,
    VV_DSP_STFT_WIN_HANN   = 1,
    VV_DSP_STFT_WIN_HAMMING= 2
} vv_dsp_stft_window;

// STFT configuration parameters
typedef struct vv_dsp_stft_params {
    size_t fft_size;   // FFT size (frame size)
    size_t hop_size;   // Hop size (frame advance)
    vv_dsp_stft_window window; // analysis/synthesis window
} vv_dsp_stft_params;

// Create/destroy
VV_DSP_NODISCARD vv_dsp_status vv_dsp_stft_create(const vv_dsp_stft_params* params, vv_dsp_stft** out);
vv_dsp_status vv_dsp_stft_destroy(vv_dsp_stft* h);

// Process a single frame (analysis):
//  - in: real time-domain frame of length fft_size (will be windowed internally)
//  - out: complex spectrum of length fft_size (full complex). For real-input efficient path use R2C via params->fft_size constraints in future.
VV_DSP_NODISCARD vv_dsp_status vv_dsp_stft_process(vv_dsp_stft* h,
                                                   const vv_dsp_real* in,
                                                   vv_dsp_cpx* out);

// Reconstruct a single frame (synthesis):
//  - in: complex spectrum of length fft_size
//  - out_add: overlap-add into an output buffer of at least fft_size samples beginning at current synthesis position
// Reconstruct frame with overlap-add and optional normalization accumulation:
//  - out_add[i] += time[i] * w[i]
//  - if norm_add != NULL: norm_add[i] += w[i]^2 (for later per-sample division)
VV_DSP_NODISCARD vv_dsp_status vv_dsp_stft_reconstruct(vv_dsp_stft* h,
                                                       const vv_dsp_cpx* in,
                                                       vv_dsp_real* out_add,
                                                       vv_dsp_real* norm_add);

// Convenience: process entire signal into magnitude spectrogram (rows=time frames, cols=fft_size)
VV_DSP_NODISCARD vv_dsp_status vv_dsp_stft_spectrogram(vv_dsp_stft* h,
                                                       const vv_dsp_real* signal,
                                                       size_t n,
                                                       vv_dsp_real* out_mag, // size: n_frames * fft_size
                                                       size_t* out_frames);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // VV_DSP_SPECTRAL_STFT_H
