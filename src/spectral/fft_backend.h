#ifndef VV_DSP_INTERNAL_FFT_BACKEND_H
#define VV_DSP_INTERNAL_FFT_BACKEND_H

#include <stddef.h>
#include "vv_dsp/vv_dsp_types.h"
#include "vv_dsp/spectral/fft.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations for backend-specific plan structures
typedef struct kiss_fft_plan_s* kiss_fft_plan;
typedef struct fftw_plan_s* fftw_plan;
typedef struct ffts_plan_s* ffts_plan;

struct vv_dsp_fft_plan {
    size_t n;
    vv_dsp_fft_type type;
    vv_dsp_fft_dir dir;
    vv_dsp_fft_backend backend;

    // Backend-specific plan data
    union {
        void* generic;          // For KissFFT (stateless)
        fftw_plan fftw;         // FFTW plan handle
        ffts_plan ffts;         // FFTS plan handle
    } backend_plan;
};

// Backend function table structure
typedef struct vv_dsp_fft_backend_vtable {
    vv_dsp_status (*make_plan)(const struct vv_dsp_fft_plan* spec, void** backend_data);
    vv_dsp_status (*execute)(const struct vv_dsp_fft_plan* spec, void* backend_data, const void* in, void* out);
    void (*free_plan)(void* backend_data);
    int (*is_available)(void);
    const char* name;
} vv_dsp_fft_backend_vtable;

// Global backend management
extern vv_dsp_fft_backend g_current_fft_backend;
extern const vv_dsp_fft_backend_vtable* g_fft_backends[3];

// Backend implementations
extern const vv_dsp_fft_backend_vtable vv_dsp_fft_kiss_vtable;
#ifdef VV_DSP_BACKEND_FFT_fftw
extern const vv_dsp_fft_backend_vtable vv_dsp_fft_fftw_vtable;
#endif
#ifdef VV_DSP_BACKEND_FFT_ffts
extern const vv_dsp_fft_backend_vtable vv_dsp_fft_ffts_vtable;
#endif

// Internal backend interface functions
vv_dsp_status vv_dsp_fft_backend_make(const struct vv_dsp_fft_plan* spec, void** backend);
vv_dsp_status vv_dsp_fft_backend_exec(const struct vv_dsp_fft_plan* spec, void* backend, const void* in, void* out);
void vv_dsp_fft_backend_free(void* backend);

#ifdef __cplusplus
}
#endif

#endif // VV_DSP_INTERNAL_FFT_BACKEND_H
