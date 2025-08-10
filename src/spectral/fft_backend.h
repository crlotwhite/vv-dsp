#ifndef VV_DSP_INTERNAL_FFT_BACKEND_H
#define VV_DSP_INTERNAL_FFT_BACKEND_H

#include <stddef.h>
#include "vv_dsp/vv_dsp_types.h"
#include "vv_dsp/spectral/fft.h"

#ifdef __cplusplus
extern "C" {
#endif

struct vv_dsp_fft_plan {
    size_t n;
    vv_dsp_fft_type type;
    vv_dsp_fft_dir dir;
};

// Backend hooks (initially KISS placeholder; can be extended for FFTW/FFTS)
vv_dsp_status vv_dsp_fft_backend_make(const struct vv_dsp_fft_plan* spec, void** backend);
vv_dsp_status vv_dsp_fft_backend_exec(const struct vv_dsp_fft_plan* spec, void* backend, const void* in, void* out);
void vv_dsp_fft_backend_free(void* backend);

#ifdef __cplusplus
}
#endif

#endif // VV_DSP_INTERNAL_FFT_BACKEND_H
