#include <stdio.h>
#include <math.h>
#include <string.h>
#include "vv_dsp/vv_dsp.h"

static int nearly_equal(vv_dsp_real a, vv_dsp_real b, vv_dsp_real tol) {
    vv_dsp_real d = a - b;
    if (d < 0) d = -d;
    return d <= tol;
}

static int test_fft_c2c_basic(void) {
    const size_t n = 8;
    vv_dsp_fft_plan* plan_f = NULL;
    vv_dsp_fft_make_plan(n, VV_DSP_FFT_C2C, VV_DSP_FFT_FORWARD, &plan_f);

    vv_dsp_cpx x[n];
    vv_dsp_cpx X[n];
    memset(X, 0, sizeof(X));

    // Impulse at t=0 => flat spectrum (all ones)
    for (size_t i = 0; i < n; ++i) x[i] = vv_dsp_cpx_make(0,0);
    x[0] = vv_dsp_cpx_make(1,0);

    vv_dsp_fft_execute(plan_f, x, X);

    int ok = 1;
    for (size_t k = 0; k < n; ++k) {
        if (!nearly_equal(X[k].re, 1.0, 1e-4) || !nearly_equal(X[k].im, 0.0, 1e-4)) { ok = 0; break; }
    }

    vv_dsp_fft_destroy(plan_f);
    return ok;
}

static int test_fft_r2c_c2r_roundtrip(void) {
    const size_t n = 8;
    vv_dsp_fft_plan *pf = NULL, *pb = NULL;
    vv_dsp_fft_make_plan(n, VV_DSP_FFT_R2C, VV_DSP_FFT_FORWARD, &pf);
    vv_dsp_fft_make_plan(n, VV_DSP_FFT_C2R, VV_DSP_FFT_BACKWARD, &pb);

    vv_dsp_real xr[n];
    for (size_t i = 0; i < n; ++i) xr[i] = (vv_dsp_real)sin(2*M_PI*i/n);

    vv_dsp_cpx Xh[n/2+1];
    vv_dsp_real xr2[n];

    vv_dsp_fft_execute(pf, xr, Xh);
    vv_dsp_fft_execute(pb, Xh, xr2);

    int ok = 1;
    for (size_t i = 0; i < n; ++i) {
        if (!nearly_equal(xr[i], xr2[i], 1e-3)) { ok = 0; break; }
    }

    vv_dsp_fft_destroy(pf);
    vv_dsp_fft_destroy(pb);
    return ok;
}

int main(void) {
    int ok1 = test_fft_c2c_basic();
    int ok2 = test_fft_r2c_c2r_roundtrip();
    // fftshift/ifftshift
    vv_dsp_real a[5] = {0,1,2,3,4};
    vv_dsp_real s[5], si[5], r[5];
    vv_dsp_fftshift_real(a, s, 5);
    vv_dsp_ifftshift_real(s, si, 5);
    int ok3 = 1;
    for (int i=0;i<5;++i) r[i]=a[i];
    for (int i=0;i<5;++i) if (!nearly_equal(si[i], r[i], 1e-6)) { ok3 = 0; break; }
    if (!ok1) { fprintf(stderr, "FFT C2C basic test failed\n"); return 1; }
    if (!ok2) { fprintf(stderr, "FFT R2C/C2R roundtrip test failed\n"); return 1; }
    if (!ok3) { fprintf(stderr, "fftshift/ifftshift test failed\n"); return 1; }
    printf("spectral tests passed\n");
    return 0;
}
