/*
This file is part of vv-dsp

FFTS backend implementation for the vv-dsp FFT abstraction layer.
Provides high-performance FFT operations using the FFTS library.

This backend integrates the FFTS (Fastest Fourier Transform in the South) library
into the vv-dsp FFT system, supporting complex-to-complex and real FFT transforms.
*/

#include "fft_backend.h"

#ifdef VV_DSP_BACKEND_FFT_ffts

#include <stdlib.h>
#include <string.h>
#include <ffts.h>

// FFTS plan wrapper structure
typedef struct {
    ffts_plan_t* plan;
    size_t n;
    vv_dsp_fft_type type;
    vv_dsp_fft_dir dir;

    // Temporary buffers for R2C/C2R transforms
    float* temp_buffer;
    size_t temp_size;
} ffts_plan_wrapper;

static vv_dsp_status ffts_make_plan(const struct vv_dsp_fft_plan* spec, void** backend_data) {
    if (!spec || !backend_data) return VV_DSP_ERROR_NULL_POINTER;

    ffts_plan_wrapper* wrapper = (ffts_plan_wrapper*)malloc(sizeof(ffts_plan_wrapper));
    if (!wrapper) return VV_DSP_ERROR_INTERNAL;

    wrapper->n = spec->n;
    wrapper->type = spec->type;
    wrapper->dir = spec->dir;
    wrapper->temp_buffer = NULL;
    wrapper->temp_size = 0;

    // FFTS uses -1 for forward, +1 for backward (opposite of our convention)
    int ffts_sign = (spec->dir == VV_DSP_FFT_FORWARD) ? -1 : 1;

    if (spec->type == VV_DSP_FFT_C2C) {
        // Complex-to-complex transform
        wrapper->plan = ffts_init_1d(spec->n, ffts_sign);
        if (!wrapper->plan) {
            free(wrapper);
            return VV_DSP_ERROR_INTERNAL;
        }
    } else if (spec->type == VV_DSP_FFT_R2C) {
        // Real-to-complex: use complex FFT with real input
        // We'll create a complex plan and handle R2C manually
        wrapper->plan = ffts_init_1d(spec->n, -1); // Always forward for R2C
        if (!wrapper->plan) {
            free(wrapper);
            return VV_DSP_ERROR_INTERNAL;
        }

        // Allocate temporary buffer for complex input
        wrapper->temp_size = spec->n * 2; // Complex numbers (re,im pairs)
        wrapper->temp_buffer = (float*)malloc(wrapper->temp_size * sizeof(float));
        if (!wrapper->temp_buffer) {
            ffts_free(wrapper->plan);
            free(wrapper);
            return VV_DSP_ERROR_INTERNAL;
        }
    } else if (spec->type == VV_DSP_FFT_C2R) {
        // Complex-to-real: use complex FFT with complex input
        wrapper->plan = ffts_init_1d(spec->n, 1); // Always backward for C2R
        if (!wrapper->plan) {
            free(wrapper);
            return VV_DSP_ERROR_INTERNAL;
        }

        // Allocate temporary buffer for full complex spectrum
        wrapper->temp_size = spec->n * 2; // Complex numbers (re,im pairs)
        wrapper->temp_buffer = (float*)malloc(wrapper->temp_size * sizeof(float));
        if (!wrapper->temp_buffer) {
            ffts_free(wrapper->plan);
            free(wrapper);
            return VV_DSP_ERROR_INTERNAL;
        }
    } else {
        free(wrapper);
        return VV_DSP_ERROR_OUT_OF_RANGE;
    }

    *backend_data = wrapper;
    return VV_DSP_OK;
}

static vv_dsp_status ffts_execute(const struct vv_dsp_fft_plan* spec, void* backend_data, const void* in, void* out) {
    if (!spec || !backend_data || !in || !out) return VV_DSP_ERROR_NULL_POINTER;

    ffts_plan_wrapper* wrapper = (ffts_plan_wrapper*)backend_data;

    if (spec->type == VV_DSP_FFT_C2C) {
        // Direct complex-to-complex execution
        // FFTS expects interleaved float arrays [re0,im0,re1,im1,...]
        ffts_execute(wrapper->plan, in, out);
        return VV_DSP_OK;
    } else if (spec->type == VV_DSP_FFT_R2C) {
        // Real-to-complex: convert real input to complex, execute, extract output
        const vv_dsp_real* real_in = (const vv_dsp_real*)in;
        vv_dsp_cpx* cpx_out = (vv_dsp_cpx*)out;

        // Convert real input to complex (zero imaginary parts)
        for (size_t i = 0; i < spec->n; ++i) {
            wrapper->temp_buffer[2*i] = (float)real_in[i];     // Real part
            wrapper->temp_buffer[2*i+1] = 0.0f;               // Imaginary part
        }

        // Execute FFT
        float* temp_out = (float*)malloc(spec->n * 2 * sizeof(float));
        if (!temp_out) return VV_DSP_ERROR_INTERNAL;

        ffts_execute(wrapper->plan, wrapper->temp_buffer, temp_out);

        // Extract Hermitian-packed output (first n/2+1 complex values)
        size_t nh = spec->n/2 + 1;
        for (size_t i = 0; i < nh; ++i) {
            cpx_out[i].re = (vv_dsp_real)temp_out[2*i];
            cpx_out[i].im = (vv_dsp_real)temp_out[2*i+1];
        }

        free(temp_out);
        return VV_DSP_OK;
    } else if (spec->type == VV_DSP_FFT_C2R) {
        // Complex-to-real: expand Hermitian input to full spectrum, execute, extract real
        const vv_dsp_cpx* cpx_in = (const vv_dsp_cpx*)in;
        vv_dsp_real* real_out = (vv_dsp_real*)out;
        size_t nh = spec->n/2 + 1;

        // Expand Hermitian-packed input to full complex spectrum
        // k=0..nh-1: copy directly
        for (size_t k = 0; k < nh; ++k) {
            wrapper->temp_buffer[2*k] = (float)cpx_in[k].re;
            wrapper->temp_buffer[2*k+1] = (float)cpx_in[k].im;
        }

        // k=nh..n-1: conjugate symmetry
        for (size_t k = nh; k < spec->n; ++k) {
            size_t mirror_idx = spec->n - k;
            if (mirror_idx < nh) {
                wrapper->temp_buffer[2*k] = (float)cpx_in[mirror_idx].re;    // Real part
                wrapper->temp_buffer[2*k+1] = -(float)cpx_in[mirror_idx].im; // Conjugate imaginary
            }
        }

        // Execute inverse FFT
        float* temp_out = (float*)malloc(spec->n * 2 * sizeof(float));
        if (!temp_out) return VV_DSP_ERROR_INTERNAL;

        ffts_execute(wrapper->plan, wrapper->temp_buffer, temp_out);

        // Extract real parts and apply 1/n scaling (FFTS doesn't scale)
        vv_dsp_real scale = (vv_dsp_real)1.0 / (vv_dsp_real)spec->n;
        for (size_t i = 0; i < spec->n; ++i) {
            real_out[i] = (vv_dsp_real)temp_out[2*i] * scale;
        }

        free(temp_out);
        return VV_DSP_OK;
    }

    return VV_DSP_ERROR_OUT_OF_RANGE;
}

static void ffts_free_plan(void* backend_data) {
    if (!backend_data) return;

    ffts_plan_wrapper* wrapper = (ffts_plan_wrapper*)backend_data;

    if (wrapper->plan) {
        ffts_free(wrapper->plan);
    }

    if (wrapper->temp_buffer) {
        free(wrapper->temp_buffer);
    }

    free(wrapper);
}

static int ffts_is_available(void) {
    return 1; // FFTS is available if this file is compiled
}

// FFTS vtable
const vv_dsp_fft_backend_vtable vv_dsp_fft_ffts_vtable = {
    .make_plan = ffts_make_plan,
    .execute = ffts_execute,
    .free_plan = ffts_free_plan,
    .is_available = ffts_is_available,
    .name = "FFTS"
};

#else

// FFTS not available - provide dummy vtable
static int ffts_not_available(void) {
    return 0;
}

const vv_dsp_fft_backend_vtable vv_dsp_fft_ffts_vtable = {
    .make_plan = NULL,
    .execute = NULL,
    .free_plan = NULL,
    .is_available = ffts_not_available,
    .name = "FFTS (not available)"
};

#endif // VV_DSP_BACKEND_FFT_ffts
