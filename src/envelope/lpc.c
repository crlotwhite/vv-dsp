#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "vv_dsp/vv_dsp_math.h"
#include "vv_dsp/envelope/lpc.h"

vv_dsp_status vv_dsp_autocorr(const vv_dsp_real* x, size_t n, size_t order, vv_dsp_real* r_out) {
    if (!x || !r_out) return VV_DSP_ERROR_NULL_POINTER;
    if (order+1 > n) return VV_DSP_ERROR_INVALID_SIZE;
    for (size_t k=0;k<=order;++k) {
        vv_dsp_real acc = 0;
        for (size_t i=0;i<n-k;++i) acc += x[i]*x[i+k];
        r_out[k] = acc;
    }
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_levinson(const vv_dsp_real* r, size_t order, vv_dsp_real* a_out, vv_dsp_real* err_out) {
    if (!r || !a_out || !err_out) return VV_DSP_ERROR_NULL_POINTER;
    // Levinson-Durbin recursion
    vv_dsp_real* a = (vv_dsp_real*)calloc(order+1, sizeof(vv_dsp_real));
    vv_dsp_real* a_prev = (vv_dsp_real*)calloc(order+1, sizeof(vv_dsp_real));
    if (!a || !a_prev) { free(a); free(a_prev); return VV_DSP_ERROR_INTERNAL; }
    vv_dsp_real e = r[0];
    if (e <= 0) { free(a); free(a_prev); return VV_DSP_ERROR_INTERNAL; }
    for (size_t m=1; m<=order; ++m) {
        vv_dsp_real acc = r[m];
        for (size_t i=1;i<m;++i) acc += a_prev[i]*r[m-i];
        vv_dsp_real k = -acc / e;
        a[0] = 1;
        a[m] = k;
        for (size_t i=1;i<m;++i) a[i] = a_prev[i] + k * a_prev[m-i];
        e *= (1 - k*k);
        // swap
        for (size_t i=0;i<=m;++i) a_prev[i] = a[i];
    }
    for (size_t i=0;i<=order;++i) a_out[i] = a_prev[i];
    *err_out = e;
    free(a); free(a_prev);
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_lpc(const vv_dsp_real* x, size_t n, size_t order, vv_dsp_real* a_out, vv_dsp_real* err_out) {
    if (!x || !a_out || !err_out) return VV_DSP_ERROR_NULL_POINTER;
    if (order+1 > n) return VV_DSP_ERROR_INVALID_SIZE;
    vv_dsp_real* r = (vv_dsp_real*)malloc(sizeof(vv_dsp_real)*(order+1));
    if (!r) return VV_DSP_ERROR_INTERNAL;
    vv_dsp_status s = vv_dsp_autocorr(x, n, order, r);
    if (s != VV_DSP_OK) { free(r); return s; }
    s = vv_dsp_levinson(r, order, a_out, err_out);
    free(r);
    return s;
}

vv_dsp_status vv_dsp_lpspec(const vv_dsp_real* a, size_t order, vv_dsp_real gain, size_t nfft, vv_dsp_real* mag_out) {
    if (!a || !mag_out) return VV_DSP_ERROR_NULL_POINTER;
    for (size_t k=0;k<nfft;++k) {
        // Evaluate A(e^{j*2pi*k/nfft}) magnitude
        vv_dsp_real theta = (vv_dsp_real)(VV_DSP_TWO_PI * (vv_dsp_real)k / (vv_dsp_real)nfft);
        vv_dsp_real re = (vv_dsp_real)1.0, im = (vv_dsp_real)0.0;
        for (size_t m=1; m<=order; ++m) {
            vv_dsp_real ang = (vv_dsp_real)m * theta;
            vv_dsp_real cr = VV_DSP_COS(ang);
            vv_dsp_real sr = VV_DSP_SIN(ang);
            re += a[m]*(-cr);
            im += a[m]*(-sr);
        }
        vv_dsp_real den = (vv_dsp_real)sqrt((double)(re*re + im*im));
        mag_out[k] = (den > (vv_dsp_real)0) ? (gain/den) : (vv_dsp_real)0;
    }
    return VV_DSP_OK;
}
