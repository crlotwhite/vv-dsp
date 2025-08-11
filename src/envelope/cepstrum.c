#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "vv_dsp/envelope/cepstrum.h"
#include "vv_dsp/spectral/fft.h"

vv_dsp_status vv_dsp_cepstrum_real(const vv_dsp_real* x, size_t n, vv_dsp_real* out_cep) {
    if (!x || !out_cep) return VV_DSP_ERROR_NULL_POINTER;
    vv_dsp_fft_plan* pf = NULL; vv_dsp_fft_plan* pb = NULL;
    if (vv_dsp_fft_make_plan(n, VV_DSP_FFT_C2C, VV_DSP_FFT_FORWARD, &pf) != VV_DSP_OK) return VV_DSP_ERROR_INTERNAL;
    if (vv_dsp_fft_make_plan(n, VV_DSP_FFT_C2C, VV_DSP_FFT_BACKWARD, &pb) != VV_DSP_OK) { vv_dsp_fft_destroy(pf); return VV_DSP_ERROR_INTERNAL; }

    vv_dsp_cpx* xin = (vv_dsp_cpx*)malloc(sizeof(vv_dsp_cpx)*n);
    vv_dsp_cpx* X = (vv_dsp_cpx*)malloc(sizeof(vv_dsp_cpx)*n);
    vv_dsp_cpx* Y = (vv_dsp_cpx*)malloc(sizeof(vv_dsp_cpx)*n);
    if (!xin || !X || !Y) { free(xin); free(X); free(Y); vv_dsp_fft_destroy(pf); vv_dsp_fft_destroy(pb); return VV_DSP_ERROR_INTERNAL; }
    for (size_t i=0;i<n;++i) { xin[i].re = x[i]; xin[i].im = 0; }
    vv_dsp_status s = vv_dsp_fft_execute(pf, xin, X);
    if (s != VV_DSP_OK) { free(xin); free(X); free(Y); vv_dsp_fft_destroy(pf); vv_dsp_fft_destroy(pb); return s; }
    for (size_t k=0;k<n;++k) {
        vv_dsp_real mag;
        #if defined(VV_DSP_USE_DOUBLE)
        mag = (vv_dsp_real)sqrt(X[k].re*X[k].re + X[k].im*X[k].im);
        vv_dsp_real lm = (vv_dsp_real)log(mag + (vv_dsp_real)1e-12);
        #else
        mag = (vv_dsp_real)sqrtf(X[k].re*X[k].re + X[k].im*X[k].im);
        vv_dsp_real lm = (vv_dsp_real)logf(mag + (vv_dsp_real)1e-12f);
        #endif
        Y[k].re = lm; Y[k].im = 0;
    }
    vv_dsp_cpx* cpx = (vv_dsp_cpx*)malloc(sizeof(vv_dsp_cpx)*n);
    if (!cpx) { free(xin); free(X); free(Y); vv_dsp_fft_destroy(pf); vv_dsp_fft_destroy(pb); return VV_DSP_ERROR_INTERNAL; }
    s = vv_dsp_fft_execute(pb, Y, cpx);
    if (s != VV_DSP_OK) { free(xin); free(X); free(Y); free(cpx); vv_dsp_fft_destroy(pf); vv_dsp_fft_destroy(pb); return s; }
    for (size_t i=0;i<n;++i) out_cep[i] = cpx[i].re; // imag ~ 0
    free(xin); free(X); free(Y); free(cpx);
    vv_dsp_fft_destroy(pf); vv_dsp_fft_destroy(pb);
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_icepstrum_minphase(const vv_dsp_real* c, size_t n, vv_dsp_real* out_x) {
    if (!c || !out_x) return VV_DSP_ERROR_NULL_POINTER;
    // Build minimum-phase spectrum via real cepstrum homomorphic property:
    // H(z) = exp( FFT( windowed_cepstrum ) ), then IFFT to time domain
    vv_dsp_fft_plan* pf = NULL; vv_dsp_fft_plan* pb = NULL;
    if (vv_dsp_fft_make_plan(n, VV_DSP_FFT_C2C, VV_DSP_FFT_FORWARD, &pf) != VV_DSP_OK) return VV_DSP_ERROR_INTERNAL;
    if (vv_dsp_fft_make_plan(n, VV_DSP_FFT_C2C, VV_DSP_FFT_BACKWARD, &pb) != VV_DSP_OK) { vv_dsp_fft_destroy(pf); return VV_DSP_ERROR_INTERNAL; }

    vv_dsp_cpx* C = (vv_dsp_cpx*)malloc(sizeof(vv_dsp_cpx)*n);
    vv_dsp_cpx* H = (vv_dsp_cpx*)malloc(sizeof(vv_dsp_cpx)*n);
    vv_dsp_cpx* h = (vv_dsp_cpx*)malloc(sizeof(vv_dsp_cpx)*n);
    if (!C || !H || !h) { free(C); free(H); free(h); vv_dsp_fft_destroy(pf); vv_dsp_fft_destroy(pb); return VV_DSP_ERROR_INTERNAL; }

    // Window cepstrum to keep only causal (min-phase) part: c[0], c[1..n/2-1]*2, c[n/2]=0, rest 0
    size_t nh = n/2;
    for (size_t i=0;i<n;++i) { C[i].re = 0; C[i].im = 0; }
    if (n>0) C[0].re = c[0];
    for (size_t i=1;i<nh; ++i) C[i].re = 2*c[i];
    if (n%2==0 && nh < n) C[nh].re = 0; // Nyquist set to 0

    vv_dsp_status s = vv_dsp_fft_execute(pf, C, H);
    if (s != VV_DSP_OK) { free(C); free(H); free(h); vv_dsp_fft_destroy(pf); vv_dsp_fft_destroy(pb); return s; }
    // Exponentiate: exp of complex H -> magnitude envelope; but here H is real (imag 0)
    for (size_t k=0;k<n;++k) {
        #if defined(VV_DSP_USE_DOUBLE)
        vv_dsp_real er = (vv_dsp_real)exp(H[k].re);
        #else
        vv_dsp_real er = (vv_dsp_real)expf(H[k].re);
        #endif
        H[k].re = er; H[k].im = 0;
    }
    s = vv_dsp_fft_execute(pb, H, h);
    if (s != VV_DSP_OK) { free(C); free(H); free(h); vv_dsp_fft_destroy(pf); vv_dsp_fft_destroy(pb); return s; }
    for (size_t i=0;i<n;++i) out_x[i] = h[i].re; // min-phase time signal
    free(C); free(H); free(h);
    vv_dsp_fft_destroy(pf); vv_dsp_fft_destroy(pb);
    return VV_DSP_OK;
}
