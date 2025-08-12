#include <math.h>
#include "vv_dsp/vv_dsp_math.h"
#include <stdlib.h>
#include <string.h>
#include "fft_backend.h"

// Minimal FFT backend
// Provides:
//  - Power-of-two iterative radix-2 Cooley-Tukey O(n log n)
//  - Fallback naive O(n^2) DFT when size not power-of-two
// Forward transform: no scaling
// Backward transform: 1/n scaling (to match existing semantics)

static int is_power_of_two(size_t n) {
    return n && ((n & (n - 1)) == 0);
}

static size_t reverse_bits(size_t x, unsigned int bits) {
    size_t r = 0;
    for (unsigned int i = 0; i < bits; ++i) {
        r = (r << 1) | (x & 1);
        x >>= 1;
    }
    return r;
}

static void fft_iterative_radix2(vv_dsp_cpx* data, size_t n, int sign) {
    // sign: +1 forward, -1 backward (angle uses -sign to match naive implementation)
    unsigned int levels = 0;
    size_t temp = n;
    while (temp > 1) { levels++; temp >>= 1; }

    // Bit-reversal permutation
    for (size_t i = 0; i < n; ++i) {
        size_t j = reverse_bits(i, levels);
        if (j > i) {
            vv_dsp_cpx t = data[i];
            data[i] = data[j];
            data[j] = t;
        }
    }

    const vv_dsp_real PI = VV_DSP_PI;
    for (size_t size = 2; size <= n; size <<= 1) {
        vv_dsp_real theta = (vv_dsp_real)(-sign) * (vv_dsp_real)2.0 * PI / (vv_dsp_real)size;
        vv_dsp_real wpr = VV_DSP_COS(theta);
        vv_dsp_real wpi = VV_DSP_SIN(theta);
        for (size_t start = 0; start < n; start += size) {
            vv_dsp_real wr = (vv_dsp_real)1.0;
            vv_dsp_real wi = (vv_dsp_real)0.0;
            size_t half = size >> 1;
            for (size_t k = 0; k < half; ++k) {
                vv_dsp_cpx* even = &data[start + k];
                vv_dsp_cpx* odd  = &data[start + k + half];
                vv_dsp_real tr = wr * odd->re - wi * odd->im;
                vv_dsp_real ti = wr * odd->im + wi * odd->re;
                odd->re = even->re - tr;
                odd->im = even->im - ti;
                even->re += tr;
                even->im += ti;
                // Update twiddle (complex multiply by w)
                vv_dsp_real new_wr = wr * wpr - wi * wpi;
                vv_dsp_real new_wi = wr * wpi + wi * wpr;
                wr = new_wr; wi = new_wi;
            }
        }
    }

    // Backward: scale by 1/n
    if (sign < 0) {
        vv_dsp_real invn = (vv_dsp_real)1.0 / (vv_dsp_real)n;
        for (size_t i = 0; i < n; ++i) { data[i].re *= invn; data[i].im *= invn; }
    }
}

static void dft_naive(const vv_dsp_cpx* in, vv_dsp_cpx* out, size_t n, int sign) {
    const vv_dsp_real PI = VV_DSP_PI;
    const vv_dsp_real one = (vv_dsp_real)1.0;
    const vv_dsp_real two = (vv_dsp_real)2.0;
    const vv_dsp_real scale = (sign < 0) ? (one/(vv_dsp_real)n) : one; // backward scales by 1/n
    for (size_t k = 0; k < n; ++k) {
        vv_dsp_real sum_re = 0, sum_im = 0;
        for (size_t t = 0; t < n; ++t) {
            vv_dsp_real ang = (vv_dsp_real)(-sign) * two * PI * (vv_dsp_real)(k * t) / (vv_dsp_real)n;
            vv_dsp_real c = VV_DSP_COS(ang), s = VV_DSP_SIN(ang);
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
        int sign = (spec->dir == VV_DSP_FFT_FORWARD) ? +1 : -1;
        if (is_power_of_two(n)) {
            // Copy input to out, operate in-place
            const vv_dsp_cpx* cin = (const vv_dsp_cpx*)in;
            vv_dsp_cpx* cout = (vv_dsp_cpx*)out;
            if (cout != cin) memcpy(cout, cin, sizeof(vv_dsp_cpx)*n);
            fft_iterative_radix2(cout, n, sign);
        } else {
            dft_naive((const vv_dsp_cpx*)in, (vv_dsp_cpx*)out, n, sign);
        }
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
