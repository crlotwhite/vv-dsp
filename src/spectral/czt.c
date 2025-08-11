#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "vv_dsp/spectral.h"
#include "vv_dsp/spectral/fft.h"
#include "vv_dsp/vv_dsp_math.h"

// simple helpers
static VV_DSP_INLINE vv_dsp_cpx cpx_mul(vv_dsp_cpx a, vv_dsp_cpx b){
    vv_dsp_cpx r; r.re = a.re*b.re - a.im*b.im; r.im = a.re*b.im + a.im*b.re; return r;
}
/* Removed unused helper to silence -Wunused-function and keep TU minimal */
/* static VV_DSP_INLINE vv_dsp_cpx cpx_conj(vv_dsp_cpx a){ vv_dsp_cpx r; r.re=a.re; r.im=-a.im; return r; } */
static VV_DSP_INLINE vv_dsp_cpx cpx_from(vv_dsp_real re, vv_dsp_real im){ vv_dsp_cpx z; z.re=re; z.im=im; return z; }

static size_t next_pow2(size_t v){
    size_t n = 1; while(n < v) n <<= 1; return n;
}

vv_dsp_status vv_dsp_czt_params_for_freq_range(
    vv_dsp_real f_start,
    vv_dsp_real f_end,
    size_t M,
    vv_dsp_real fs,
    vv_dsp_real* W_real, vv_dsp_real* W_imag,
    vv_dsp_real* A_real, vv_dsp_real* A_imag)
{
    if (!W_real || !W_imag || !A_real || !A_imag) return VV_DSP_ERROR_NULL_POINTER;
    if (M == 0 || fs <= (vv_dsp_real)0) return VV_DSP_ERROR_INVALID_SIZE;
    vv_dsp_real delta = (f_end - f_start) / (vv_dsp_real)M;
    vv_dsp_real theta = (vv_dsp_real)(-2.0 * VV_DSP_PI_D * (double)delta / (double)fs);
    *W_real = VV_DSP_COS(theta);
    *W_imag = VV_DSP_SIN(theta);
    vv_dsp_real phi0 = (vv_dsp_real)(-2.0 * VV_DSP_PI_D * (double)f_start / (double)fs);
    *A_real = VV_DSP_COS(phi0);
    *A_imag = VV_DSP_SIN(phi0);
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_czt_exec_real(
    const vv_dsp_real* x,
    size_t N,
    size_t M,
    vv_dsp_real W_re, vv_dsp_real W_im,
    vv_dsp_real A_re, vv_dsp_real A_im,
    vv_dsp_cpx* X)
{
    if (!x || !X) return VV_DSP_ERROR_NULL_POINTER;
    // pack to complex and call complex variant
    vv_dsp_cpx* xc = (vv_dsp_cpx*)malloc(N * sizeof(vv_dsp_cpx));
    if (!xc) return VV_DSP_ERROR_INTERNAL;
    for (size_t n=0;n<N;++n){ xc[n].re = x[n]; xc[n].im = (vv_dsp_real)0; }
    vv_dsp_status st = vv_dsp_czt_exec_cpx(xc, N, M, W_re, W_im, A_re, A_im, X);
    free(xc);
    return st;
}

vv_dsp_status vv_dsp_czt_exec_cpx(
    const vv_dsp_cpx* x,
    size_t N,
    size_t M,
    vv_dsp_real W_re, vv_dsp_real W_im,
    vv_dsp_real A_re, vv_dsp_real A_im,
    vv_dsp_cpx* X)
{
    if (!x || !X) return VV_DSP_ERROR_NULL_POINTER;
    if (N == 0 || M == 0) return VV_DSP_ERROR_INVALID_SIZE;

    // Precompute chirp sequences (Bluestein):
    // g[n] = A^{-n} * W^{+n^2/2}
    // h[m] = W^{-m^2/2}
    vv_dsp_cpx A = cpx_from(A_re, A_im);

    vv_dsp_cpx *g = (vv_dsp_cpx*)malloc(N * sizeof(vv_dsp_cpx));
    vv_dsp_cpx *h = (vv_dsp_cpx*)malloc(M * sizeof(vv_dsp_cpx));
    if (!g || !h) { free(g); free(h); return VV_DSP_ERROR_INTERNAL; }

    // powers via recurrence for stability/speed
    vv_dsp_cpx A_inv = cpx_from(A.re, -A.im);
    // compute A^{-1}
    vv_dsp_real denom = (A.re*A.re + A.im*A.im);
    if (denom != (vv_dsp_real)0) { A_inv.re =  A.re/denom; A_inv.im = -A.im/denom; }

    // Precompute W^(n^2/2) and W^(-k^2/2) using magnitude/angle decomposition
    vv_dsp_real argW = (vv_dsp_real)atan2((double)W_im, (double)W_re);
    vv_dsp_real magW = (vv_dsp_real)hypot((double)W_re, (double)W_im);

    vv_dsp_cpx A_pow = cpx_from(1,0);
    vv_dsp_cpx A_inv_pow = cpx_from(1,0);

    for (size_t n=0;n<N;++n){
    // exponent e = +0.5 * n^2
    vv_dsp_real e = (vv_dsp_real)0.5 * (vv_dsp_real)((double)n*(double)n);
    vv_dsp_real ang = e * argW;
    vv_dsp_real mag = (vv_dsp_real)pow((double)magW, (double)e);
    vv_dsp_cpx W_n2 = cpx_from(mag * VV_DSP_COS(ang), mag * VV_DSP_SIN(ang));
        if (n==0){ A_pow = cpx_from(1,0); A_inv_pow = cpx_from(1,0); }
        else {
            // A_pow = A_pow * A
            vv_dsp_cpx t = A_pow; A_pow = cpx_mul(t, A);
            t = A_inv_pow; A_inv_pow = cpx_mul(t, A_inv);
        }
        // g[n] = A^{-n} * W^{n^2/2}
        g[n] = cpx_mul(A_inv_pow, W_n2);
    }
    for (size_t k=0;k<M;++k){
        vv_dsp_real e = (vv_dsp_real)0.5 * (vv_dsp_real)((double)k*(double)k);
        vv_dsp_real ang = -e * argW;
        vv_dsp_real mag = (vv_dsp_real)pow((double)magW, (double)(-e));
        h[k] = cpx_from(mag * VV_DSP_COS(ang), mag * VV_DSP_SIN(ang));
    }

    // Convolution length L = N+M-1; use FFT length P >= next_pow2(L)
    size_t L = N + M - 1;
    size_t P = next_pow2(L);

    // Prepare buffers
    vv_dsp_cpx *a = (vv_dsp_cpx*)calloc(P, sizeof(vv_dsp_cpx)); // a[n] = x[n]*g[n]
    vv_dsp_cpx *b = (vv_dsp_cpx*)calloc(P, sizeof(vv_dsp_cpx)); // b = h with zero-pad
    vv_dsp_cpx *A_fft = (vv_dsp_cpx*)malloc(P * sizeof(vv_dsp_cpx));
    vv_dsp_cpx *B_fft = (vv_dsp_cpx*)malloc(P * sizeof(vv_dsp_cpx));
    vv_dsp_cpx *C_fft = (vv_dsp_cpx*)malloc(P * sizeof(vv_dsp_cpx));
    if (!a || !b || !A_fft || !B_fft || !C_fft){
        free(g); free(h); free(a); free(b); free(A_fft); free(B_fft); free(C_fft);
        return VV_DSP_ERROR_INTERNAL;
    }

    for (size_t n=0;n<N;++n) a[n] = cpx_mul(x[n], g[n]);
    // Build b[i] = v[i-(N-1)] for i=0..L-1, where v[m] = W^{-m^2/2}
    for (size_t i=0; i<L; ++i){
        long m = (long)i - (long)(N - 1);
        vv_dsp_real dm = (vv_dsp_real)m;
        vv_dsp_real e = (vv_dsp_real)0.5 * dm * dm;
        vv_dsp_real ang = -e * argW;
        vv_dsp_real mag = (vv_dsp_real)pow((double)magW, (double)(-e));
        b[i] = cpx_from(mag * VV_DSP_COS(ang), mag * VV_DSP_SIN(ang));
    }

    // FFT(a), FFT(b)
    vv_dsp_fft_plan *pf = NULL, *pg = NULL, *pi = NULL;
    if (vv_dsp_fft_make_plan(P, VV_DSP_FFT_C2C, VV_DSP_FFT_FORWARD, &pf) != VV_DSP_OK) goto cleanup_err;
    if (vv_dsp_fft_make_plan(P, VV_DSP_FFT_C2C, VV_DSP_FFT_FORWARD, &pg) != VV_DSP_OK) goto cleanup_err;
    if (vv_dsp_fft_make_plan(P, VV_DSP_FFT_C2C, VV_DSP_FFT_BACKWARD, &pi) != VV_DSP_OK) goto cleanup_err;

    if (vv_dsp_fft_execute(pf, a, A_fft) != VV_DSP_OK) goto cleanup_err;
    if (vv_dsp_fft_execute(pg, b, B_fft) != VV_DSP_OK) goto cleanup_err;

    // element-wise multiply in freq domain
    for (size_t i=0;i<P;++i){
        C_fft[i] = cpx_mul(A_fft[i], B_fft[i]);
    }

    // IFFT to get convolution result c = a (*) b
    if (vv_dsp_fft_execute(pi, C_fft, a) != VV_DSP_OK) goto cleanup_err; // reuse a as time-domain buffer

    // Extract k=0..M-1 from indices n = (N-1) .. (N-1 + M-1)
    for (size_t k=0;k<M;++k){
        size_t idx = (N - 1) + k;
        // Final multiply by W^{+k^2/2}
        vv_dsp_real e = (vv_dsp_real)0.5 * (vv_dsp_real)(k*(double)k);
        vv_dsp_real ang = e * argW;
        vv_dsp_real mag = (vv_dsp_real)pow((double)magW, (double)(e));
        vv_dsp_cpx wk = cpx_from(mag * VV_DSP_COS(ang), mag * VV_DSP_SIN(ang));
        vv_dsp_cpx yk = cpx_mul(a[idx], wk);
        X[k] = yk;
    }

    vv_dsp_fft_destroy(pf); vv_dsp_fft_destroy(pg); vv_dsp_fft_destroy(pi);
    free(g); free(h); free(a); free(b); free(A_fft); free(B_fft); free(C_fft);
    return VV_DSP_OK;

cleanup_err:
    if (pf) vv_dsp_fft_destroy(pf);
    if (pg) vv_dsp_fft_destroy(pg);
    if (pi) vv_dsp_fft_destroy(pi);
    free(g); free(h); free(a); free(b); free(A_fft); free(B_fft); free(C_fft);
    return VV_DSP_ERROR_INTERNAL;
}
