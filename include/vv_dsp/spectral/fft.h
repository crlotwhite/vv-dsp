#ifndef VV_DSP_SPECTRAL_FFT_H
#define VV_DSP_SPECTRAL_FFT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vv_dsp/vv_dsp_types.h"
#include <stddef.h>

// Opaque plan handle
typedef struct vv_dsp_fft_plan vv_dsp_fft_plan;

// Direction: forward (time->freq) or backward (freq->time)
typedef enum vv_dsp_fft_dir {
    VV_DSP_FFT_FORWARD = +1,
    VV_DSP_FFT_BACKWARD = -1
} vv_dsp_fft_dir;

// Transform type
typedef enum vv_dsp_fft_type {
    VV_DSP_FFT_C2C = 0, // complex -> complex
    VV_DSP_FFT_R2C = 1, // real -> complex (Hermitian-packed output, size n/2+1)
    VV_DSP_FFT_C2R = 2  // complex (Hermitian-packed, size n/2+1) -> real
} vv_dsp_fft_type;

// Create a plan for 1D FFT of length n
// For R2C: in: real[n], out: complex[n/2+1]
// For C2R: in: complex[n/2+1] (Hermitian packed), out: real[n]
// For C2C: in/out: complex[n]
// Semantics: forward transforms are unscaled; backward (inverse) divides by n
VV_DSP_NODISCARD vv_dsp_status vv_dsp_fft_make_plan(size_t n,
                                                    vv_dsp_fft_type type,
                                                    vv_dsp_fft_dir dir,
                                                    vv_dsp_fft_plan** out_plan);

// Execute the transform described by plan.
// The caller must provide correctly typed buffers as described above.
VV_DSP_NODISCARD vv_dsp_status vv_dsp_fft_execute(const vv_dsp_fft_plan* plan,
                                                  const void* in,
                                                  void* out);

// Destroy a plan and free its resources
vv_dsp_status vv_dsp_fft_destroy(vv_dsp_fft_plan* plan);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // VV_DSP_SPECTRAL_FFT_H
