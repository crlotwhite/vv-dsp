#include "vv_dsp/filter/fir.h"
#include "vv_dsp/vv_dsp_math.h"
#include "vv_dsp/spectral/fft.h"
#include "vv_dsp/core/vv_dsp_vectorized_math.h"
#include <stdlib.h>
#include <string.h>

static vv_dsp_real sinc_r(vv_dsp_real x) {
    if (x == (vv_dsp_real)0) return (vv_dsp_real)1;
#if defined(VV_DSP_USE_DOUBLE)
    return (vv_dsp_real)(VV_DSP_SIN(VV_DSP_PI_D * (double)x) / (VV_DSP_PI_D * (double)x));
#else
    return (vv_dsp_real)(VV_DSP_SIN(VV_DSP_PI * x) / (VV_DSP_PI * x));
#endif
}

static vv_dsp_status apply_window(vv_dsp_real* w, size_t N, vv_dsp_window_type type) {
    if (!w) return VV_DSP_ERROR_NULL_POINTER;
    if (N == 0) return VV_DSP_ERROR_INVALID_SIZE;
    switch (type) {
        case VV_DSP_WINDOW_RECTANGULAR:
            for (size_t n = 0; n < N; ++n) w[n] = (vv_dsp_real)1;
            break;
        case VV_DSP_WINDOW_HAMMING:
            for (size_t n = 0; n < N; ++n)
                w[n] = (vv_dsp_real)((vv_dsp_real)0.54 - (vv_dsp_real)0.46 * VV_DSP_COS(VV_DSP_TWO_PI * (vv_dsp_real)n / (vv_dsp_real)(N - 1)));
            break;
        case VV_DSP_WINDOW_HANNING:
            for (size_t n = 0; n < N; ++n)
                w[n] = (vv_dsp_real)((vv_dsp_real)0.5 - (vv_dsp_real)0.5 * VV_DSP_COS(VV_DSP_TWO_PI * (vv_dsp_real)n / (vv_dsp_real)(N - 1)));
            break;
        case VV_DSP_WINDOW_BLACKMAN: {
            const double a0 = 0.42, a1 = 0.5, a2 = 0.08;
            for (size_t n = 0; n < N; ++n) {
                double c1 = (vv_dsp_real)VV_DSP_COS((vv_dsp_real)(VV_DSP_TWO_PI_D * (double)n / (double)(N - 1)));
                double c2 = (vv_dsp_real)VV_DSP_COS((vv_dsp_real)(2.0 * VV_DSP_TWO_PI_D * (double)n / (double)(N - 1)));
                w[n] = (vv_dsp_real)(a0 - a1 * c1 + a2 * c2);
            }
            break;
        }
        default:
            return VV_DSP_ERROR_INTERNAL;
    }
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_fir_design_lowpass(vv_dsp_real* h,
                                        size_t M,
                                        vv_dsp_real fc,
                                        vv_dsp_window_type wt) {
    if (!h) return VV_DSP_ERROR_NULL_POINTER;
    if (M == 0) return VV_DSP_ERROR_INVALID_SIZE;
    if (!(fc > (vv_dsp_real)0 && fc < (vv_dsp_real)1)) return VV_DSP_ERROR_OUT_OF_RANGE;

    const size_t N = M; // number of taps
    const vv_dsp_real alpha = (vv_dsp_real)(N - 1) / (vv_dsp_real)2;

    // Ideal (windowless) low-pass (normalized to Nyquist=1.0)
    for (size_t n = 0; n < N; ++n) {
        vv_dsp_real m = (vv_dsp_real)n - alpha;
        h[n] = 2 * fc * sinc_r(2 * fc * m);
    }

    // Apply window
    vv_dsp_real* w = (vv_dsp_real*)malloc(N * sizeof(vv_dsp_real));
    if (!w) return VV_DSP_ERROR_INTERNAL;
    vv_dsp_status s = apply_window(w, N, wt);
    if (s != VV_DSP_OK) { free(w); return s; }
    for (size_t n = 0; n < N; ++n) h[n] *= w[n];
    free(w);

    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_fir_apply_fft(vv_dsp_fir_state* st,
                                   const vv_dsp_real* h,
                                   const vv_dsp_real* x,
                                   vv_dsp_real* y,
                                   size_t n) {
    if (!st || !h || !x || !y) return VV_DSP_ERROR_NULL_POINTER;
    if (st->num_taps == 0) return VV_DSP_ERROR_INVALID_SIZE;
    // Choose FFT size >= n + L -1, next power of two
    size_t L = st->num_taps;
    size_t lin_len = n + L - 1;
    size_t Nfft = 1; while (Nfft < lin_len) Nfft <<= 1;

    // Allocate buffers
    vv_dsp_real* xb = (vv_dsp_real*)calloc(Nfft, sizeof(vv_dsp_real));
    vv_dsp_real* hb = (vv_dsp_real*)calloc(Nfft, sizeof(vv_dsp_real));
    if (!xb || !hb) { free(xb); free(hb); return VV_DSP_ERROR_INTERNAL; }
    memcpy(xb, x, n * sizeof(vv_dsp_real));
    memcpy(hb, h, L * sizeof(vv_dsp_real));

    // R2C FFT sizes: Nfft -> Nfft/2+1 complex
    size_t Nc = Nfft / 2 + 1;
    vv_dsp_cpx* X = (vv_dsp_cpx*)calloc(Nc, sizeof(vv_dsp_cpx));
    vv_dsp_cpx* H = (vv_dsp_cpx*)calloc(Nc, sizeof(vv_dsp_cpx));
    vv_dsp_cpx* Y = (vv_dsp_cpx*)calloc(Nc, sizeof(vv_dsp_cpx));
    if (!X || !H || !Y) { free(xb); free(hb); free(X); free(H); free(Y); return VV_DSP_ERROR_INTERNAL; }

    vv_dsp_fft_plan* p_r2c = NULL; vv_dsp_fft_plan* p_c2r = NULL;
    if (vv_dsp_fft_make_plan(Nfft, VV_DSP_FFT_R2C, VV_DSP_FFT_FORWARD, &p_r2c) != VV_DSP_OK) {
        free(xb); free(hb); free(X); free(H); free(Y); return VV_DSP_ERROR_INTERNAL;
    }
    if (vv_dsp_fft_make_plan(Nfft, VV_DSP_FFT_C2R, VV_DSP_FFT_BACKWARD, &p_c2r) != VV_DSP_OK) {
        vv_dsp_fft_destroy(p_r2c); free(xb); free(hb); free(X); free(H); free(Y); return VV_DSP_ERROR_INTERNAL;
    }

    vv_dsp_status s;
    s = vv_dsp_fft_execute(p_r2c, xb, X); if (s!=VV_DSP_OK) goto cleanup;
    s = vv_dsp_fft_execute(p_r2c, hb, H); if (s!=VV_DSP_OK) goto cleanup;

    // Pointwise multiply using vectorized operations if available
    s = vv_dsp_vectorized_complex_multiply(X, H, Y, Nc);
    if (s != VV_DSP_OK) {
        // Fallback to scalar implementation
        for (size_t k = 0; k < Nc; ++k) {
            vv_dsp_real ar = X[k].re, ai = X[k].im;
            vv_dsp_real br = H[k].re, bi = H[k].im;
            Y[k].re = ar*br - ai*bi;
            Y[k].im = ar*bi + ai*br;
        }
    }

    // IFFT to time domain (backend inverse already scales by 1/Nfft)
    s = vv_dsp_fft_execute(p_c2r, Y, xb); if (s!=VV_DSP_OK) goto cleanup;
    // Copy first n samples for linear convolution result (zero initial conditions)
    for (size_t i = 0; i < n; ++i) y[i] = xb[i];

cleanup:
    vv_dsp_fft_destroy(p_r2c);
    vv_dsp_fft_destroy(p_c2r);
    free(xb); free(hb); free(X); free(H); free(Y);
    return s;
}

vv_dsp_status vv_dsp_fir_state_init(vv_dsp_fir_state* st, size_t num_taps) {
    if (!st) return VV_DSP_ERROR_NULL_POINTER;
    if (num_taps == 0) return VV_DSP_ERROR_INVALID_SIZE;
    memset(st, 0, sizeof(*st));
    st->num_taps = num_taps;
    st->history_size = (num_taps > 0) ? (num_taps - 1) : 0;
    if (st->history_size) {
        st->history = (vv_dsp_real*)calloc(st->history_size, sizeof(vv_dsp_real));
        if (!st->history) return VV_DSP_ERROR_INTERNAL;
    }
    st->history_idx = 0;
    return VV_DSP_OK;
}

void vv_dsp_fir_state_free(vv_dsp_fir_state* st) {
    if (!st) return;
    free(st->history);
    st->history = NULL;
    st->history_size = 0;
    st->history_idx = 0;
    st->num_taps = 0;
}

vv_dsp_status vv_dsp_fir_apply(vv_dsp_fir_state* st,
                               const vv_dsp_real* h,
                               const vv_dsp_real* x,
                               vv_dsp_real* y,
                               size_t n) {
    if (!st || !h || !x || !y) return VV_DSP_ERROR_NULL_POINTER;
    if (st->num_taps == 0) return VV_DSP_ERROR_INVALID_SIZE;

    size_t L = st->num_taps;
    // For each sample, we convolve using history + current input window
    for (size_t i = 0; i < n; ++i) {
        vv_dsp_real acc = 0;
        // h[0] multiplies current sample, h[k] multiplies sample k steps ago
        // Compose a virtual window: [x[i], x[i-1], ..., history]
        // Start with current input and previous history (do not push x[i] yet)
        size_t tap = 0;
        acc += h[tap++] * x[i];
        // Pull from history newest->oldest
        if (st->history_size) {
            size_t idx = (st->history_idx == 0) ? (st->history_size - 1) : (st->history_idx - 1);
            for (; tap < L; ++tap) {
                vv_dsp_real v = st->history[idx];
                acc += h[tap] * v;
                if (idx == 0) idx = st->history_size - 1; else idx--;
            }
        }
        y[i] = acc;

        // Now push current input into history for future samples
        if (st->history_size) {
            st->history[st->history_idx] = x[i];
            st->history_idx = (st->history_idx + 1) % st->history_size;
        }
    }

    return VV_DSP_OK;
}
