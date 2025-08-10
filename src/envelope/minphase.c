#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "vv_dsp/envelope/minphase.h"
#include "vv_dsp/spectral/fft.h"

vv_dsp_status vv_dsp_minphase_from_cepstrum(const vv_dsp_real* c, size_t n, vv_dsp_cpx* out_spec) {
    if (!c || !out_spec) return VV_DSP_ERROR_NULL_POINTER;
    vv_dsp_fft_plan* pf = NULL;
    if (vv_dsp_fft_make_plan(n, VV_DSP_FFT_C2C, VV_DSP_FFT_FORWARD, &pf) != VV_DSP_OK) return VV_DSP_ERROR_INTERNAL;
    vv_dsp_cpx* C = (vv_dsp_cpx*)malloc(sizeof(vv_dsp_cpx)*n);
    vv_dsp_cpx* H = (vv_dsp_cpx*)malloc(sizeof(vv_dsp_cpx)*n);
    if (!C || !H) { free(C); free(H); vv_dsp_fft_destroy(pf); return VV_DSP_ERROR_INTERNAL; }

    size_t nh = n/2;
    for (size_t i=0;i<n;++i) { C[i].re = 0; C[i].im = 0; }
    if (n>0) C[0].re = c[0];
    for (size_t i=1;i<nh; ++i) C[i].re = 2*c[i];
    if (n%2==0 && nh < n) C[nh].re = 0;

    vv_dsp_status s = vv_dsp_fft_execute(pf, C, H);
    if (s != VV_DSP_OK) { free(C); free(H); vv_dsp_fft_destroy(pf); return s; }
    for (size_t k=0;k<n;++k) {
        vv_dsp_real er = (vv_dsp_real)exp((double)H[k].re);
        out_spec[k].re = er;
        out_spec[k].im = 0;
    }
    free(C); free(H);
    vv_dsp_fft_destroy(pf);
    return VV_DSP_OK;
}
