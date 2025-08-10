#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "fft_backend.h"

// Minimal portable radix-2 Cooley-Tukey C2C fallback (O(n log n))
// Not optimized; serves as a baseline backend when no external libs used.
// Supports forward (no scaling) and backward (1/n scaling).

static void dft_naive(const vv_dsp_cpx* in, vv_dsp_cpx* out, size_t n, int sign) {
    const vv_dsp_real PI = (vv_dsp_real)3.14159265358979323846264338327950288;
    const vv_dsp_real scale = (sign < 0) ? (1.0/(vv_dsp_real)n) : 1.0; // backward scales by 1/n
    for (size_t k = 0; k < n; ++k) {
        vv_dsp_real sum_re = 0, sum_im = 0;
        for (size_t t = 0; t < n; ++t) {
            vv_dsp_real ang = (vv_dsp_real)(-sign) * 2.0 * PI * (vv_dsp_real)(k * t) / (vv_dsp_real)n;
            vv_dsp_real c = cos(ang), s = sin(ang);
            sum_re += in[t].re * c - in[t].im * s;
            sum_im += in[t].re * s + in[t].im * c;
        }
        out[k].re = sum_re * scale;
        out[k].im = sum_im * scale;
    }
}

vv_dsp_status vv_dsp_fft_backend_make(const struct vv_dsp_fft_plan* spec, void** backend) {
    (void)spec; (void)backend; // stateless baseline
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_fft_backend_exec(const struct vv_dsp_fft_plan* spec, void* backend, const void* in, void* out) {
    (void)backend;
    if (!spec || !in || !out) return VV_DSP_ERROR_NULL_POINTER;
    const size_t n = spec->n;

    if (spec->type == VV_DSP_FFT_C2C) {
        dft_naive((const vv_dsp_cpx*)in, (vv_dsp_cpx*)out, n, (spec->dir == VV_DSP_FFT_FORWARD) ? +1 : -1);
        return VV_DSP_OK;
    }

    if (spec->type == VV_DSP_FFT_R2C) {
        // Build complex input from real, run C2C forward, then keep first n/2+1 bins
        const vv_dsp_real* rin = (const vv_dsp_real*)in;
        vv_dsp_cpx* tmp_in = (vv_dsp_cpx*)malloc(sizeof(vv_dsp_cpx) * n);
        vv_dsp_cpx* tmp_out = (vv_dsp_cpx*)malloc(sizeof(vv_dsp_cpx) * n);
        if (!tmp_in || !tmp_out) { free(tmp_in); free(tmp_out); return VV_DSP_ERROR_INTERNAL; }
        for (size_t i = 0; i < n; ++i) tmp_in[i] = vv_dsp_cpx_make(rin[i], 0);
        dft_naive(tmp_in, tmp_out, n, +1);
        size_t nh = n/2 + 1;
        memcpy(out, tmp_out, sizeof(vv_dsp_cpx) * nh);
        free(tmp_in); free(tmp_out);
        return VV_DSP_OK;
    }

    if (spec->type == VV_DSP_FFT_C2R) {
        // Expand Hermitian-packed input to full spectrum, run inverse, which scales by 1/n
        vv_dsp_real* rout = (vv_dsp_real*)out;
        const size_t nh = n/2 + 1;
        const vv_dsp_cpx* inhp = (const vv_dsp_cpx*)in;
        vv_dsp_cpx* full = (vv_dsp_cpx*)malloc(sizeof(vv_dsp_cpx) * n);
        vv_dsp_cpx* time = (vv_dsp_cpx*)malloc(sizeof(vv_dsp_cpx) * n);
        if (!full || !time) { free(full); free(time); return VV_DSP_ERROR_INTERNAL; }
        // k=0..nh-1 copy, then reconstruct k=nh..n-1 from conjugate symmetry
        for (size_t k = 0; k < nh; ++k) full[k] = inhp[k];
        for (size_t k = nh; k < n; ++k) {
            size_t m = n - k; // mirror index
            vv_dsp_cpx v = inhp[m];
            full[k].re = v.re;
            full[k].im = -v.im;
        }
        dft_naive(full, time, n, -1);
        for (size_t i = 0; i < n; ++i) rout[i] = time[i].re; // imaginary should be ~0
        free(full); free(time);
        return VV_DSP_OK;
    }

    return VV_DSP_ERROR_OUT_OF_RANGE;
}

void vv_dsp_fft_backend_free(void* backend) {
    (void)backend;
}
