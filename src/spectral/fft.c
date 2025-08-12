#include <stdlib.h>
#include <string.h>
#include "fft_backend.h"

// Global backend state
vv_dsp_fft_backend g_current_fft_backend = VV_DSP_FFT_BACKEND_KISS;

// Backend dispatch table (initialized at the bottom of this file)
const vv_dsp_fft_backend_vtable* g_fft_backends[3] = {NULL, NULL, NULL};

// Backend management API implementations
VV_DSP_NODISCARD vv_dsp_status vv_dsp_fft_set_backend(vv_dsp_fft_backend backend) {
    if (backend >= 3) return VV_DSP_ERROR_OUT_OF_RANGE;
    if (!g_fft_backends[backend] || !g_fft_backends[backend]->is_available()) {
        return VV_DSP_ERROR_UNSUPPORTED;
    }
    g_current_fft_backend = backend;
    return VV_DSP_OK;
}

vv_dsp_fft_backend vv_dsp_fft_get_backend(void) {
    return g_current_fft_backend;
}

int vv_dsp_fft_is_backend_available(vv_dsp_fft_backend backend) {
    if (backend >= 3) return 0;
    return g_fft_backends[backend] && g_fft_backends[backend]->is_available();
}

// FFTW-specific configuration (only available if FFTW backend is compiled in)
#ifdef VV_DSP_BACKEND_FFT_fftw
// These functions are implemented in fft_fftw.c
extern VV_DSP_NODISCARD vv_dsp_status vv_dsp_fft_set_fftw_flag_impl(vv_dsp_fftw_flag flag);
extern VV_DSP_NODISCARD vv_dsp_status vv_dsp_fft_flush_fftw_cache_impl(void);

VV_DSP_NODISCARD vv_dsp_status vv_dsp_fft_set_fftw_flag(vv_dsp_fftw_flag flag) {
    return vv_dsp_fft_set_fftw_flag_impl(flag);
}
VV_DSP_NODISCARD vv_dsp_status vv_dsp_fft_flush_fftw_cache(void) {
    return vv_dsp_fft_flush_fftw_cache_impl();
}
#else
VV_DSP_NODISCARD vv_dsp_status vv_dsp_fft_set_fftw_flag(vv_dsp_fftw_flag flag) {
    (void)flag;
    return VV_DSP_ERROR_UNSUPPORTED;
}
VV_DSP_NODISCARD vv_dsp_status vv_dsp_fft_flush_fftw_cache(void) {
    return VV_DSP_ERROR_UNSUPPORTED;
}
#endif

VV_DSP_NODISCARD vv_dsp_status vv_dsp_fft_make_plan(size_t n,
                                                    vv_dsp_fft_type type,
                                                    vv_dsp_fft_dir dir,
                                                    vv_dsp_fft_plan** out_plan) {
    if (!out_plan) return VV_DSP_ERROR_NULL_POINTER;
    *out_plan = NULL;
    if (n == 0) return VV_DSP_ERROR_INVALID_SIZE;
    if (!(type == VV_DSP_FFT_C2C || type == VV_DSP_FFT_R2C || type == VV_DSP_FFT_C2R)) return VV_DSP_ERROR_OUT_OF_RANGE;
    if (!(dir == VV_DSP_FFT_FORWARD || dir == VV_DSP_FFT_BACKWARD)) return VV_DSP_ERROR_OUT_OF_RANGE;

    vv_dsp_fft_plan* plan = (vv_dsp_fft_plan*)malloc(sizeof(vv_dsp_fft_plan));
    if (!plan) return VV_DSP_ERROR_INTERNAL;

    plan->n = n;
    plan->type = type;
    plan->dir = dir;
    plan->backend = g_current_fft_backend;
    plan->backend_plan.generic = NULL;

    vv_dsp_status st = vv_dsp_fft_backend_make(plan, &plan->backend_plan.generic);
    if (st != VV_DSP_OK) {
        free(plan);
        return st;
    }

    *out_plan = plan;
    return VV_DSP_OK;
}

VV_DSP_NODISCARD vv_dsp_status vv_dsp_fft_execute(const vv_dsp_fft_plan* plan,
                                                  const void* in,
                                                  void* out) {
    if (!plan || !in || !out) return VV_DSP_ERROR_NULL_POINTER;
    return vv_dsp_fft_backend_exec(plan, plan->backend_plan.generic, in, out);
}

vv_dsp_status vv_dsp_fft_destroy(vv_dsp_fft_plan* plan) {
    if (!plan) return VV_DSP_ERROR_NULL_POINTER;
    vv_dsp_fft_backend_free(plan->backend_plan.generic);
    free(plan);
    return VV_DSP_OK;
}

// Initialize backend dispatch table
static void __attribute__((constructor)) vv_dsp_fft_init_backends(void) {
    g_fft_backends[VV_DSP_FFT_BACKEND_KISS] = &vv_dsp_fft_kiss_vtable;
#ifdef VV_DSP_BACKEND_FFT_fftw
    g_fft_backends[VV_DSP_FFT_BACKEND_FFTW] = &vv_dsp_fft_fftw_vtable;
#endif
#ifdef VV_DSP_BACKEND_FFT_ffts
    g_fft_backends[VV_DSP_FFT_BACKEND_FFTS] = &vv_dsp_fft_ffts_vtable;
#endif
}
