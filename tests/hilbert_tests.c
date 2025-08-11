#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "vv_dsp/spectral.h"

static void gen_sine(vv_dsp_real* x, size_t N, double fs, double f, double phase) {
    for (size_t n=0;n<N;++n) {
        double t = (double)n / fs;
        x[n] = (vv_dsp_real)sin(2.0*M_PI*f*t + phase);
    }
}

static int test_plain_sine(void) {
    const size_t N = 256;
    const double fs = 1000.0;
    // Use a bin-centered frequency to minimize leakage: f0 = k * fs / N
    const size_t kbin = 31; // 31 -> ~121.09375 Hz at fs=1000, N=256
    const double f0 = (double)kbin * fs / (double)N;
    vv_dsp_real* x = (vv_dsp_real*)malloc(N*sizeof(vv_dsp_real));
    vv_dsp_cpx* xa = (vv_dsp_cpx*)malloc(N*sizeof(vv_dsp_cpx));
    vv_dsp_real* phi = (vv_dsp_real*)malloc(N*sizeof(vv_dsp_real));
    vv_dsp_real* instf = (vv_dsp_real*)malloc(N*sizeof(vv_dsp_real));
    if (!x||!xa||!phi||!instf) return 1;
    gen_sine(x, N, fs, f0, 0.0);

    if (vv_dsp_hilbert_analytic(x, N, xa) != VV_DSP_OK) return 2;
    // Real part should match x within tolerance
    double max_abs = 0.0;
    for (size_t i=0;i<N;++i) {
        double err = fabs((double)xa[i].re - (double)x[i]);
        if (err > max_abs) max_abs = err;
    }
    if (max_abs > 1e-3) { fprintf(stderr, "re mismatch max_abs=%g\n", max_abs); return 3; }

    if (vv_dsp_instantaneous_phase(xa, N, phi) != VV_DSP_OK) return 4;
    if (vv_dsp_instantaneous_frequency(phi, N, fs, instf) != VV_DSP_OK) return 5;
    // Frequency should be around f0 except at index 0
    double avg = 0.0; size_t cnt=0;
    for (size_t i=1;i<N;++i) { avg += (double)instf[i]; cnt++; }
    avg /= (double)cnt;
    if (fabs(avg - f0) > 0.5) { fprintf(stderr, "freq avg mismatch avg=%g expected=%g\n", avg, f0); return 6; }

    free(x); free(xa); free(phi); free(instf);
    return 0;
}

int main(void) {
    int rc = 0;
    rc |= test_plain_sine();
    if (rc != 0) {
        fprintf(stderr, "hilbert_tests failed with rc=%d\n", rc);
        return 1;
    }
    printf("hilbert_tests passed\n");
    return 0;
}
