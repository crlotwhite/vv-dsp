#include <stdio.h>
#include <math.h>
#include "vv_dsp/vv_dsp.h"

static int almost_equal(vv_dsp_real a, vv_dsp_real b, vv_dsp_real tol) {
    vv_dsp_real d = a - b; if (d<0) d=-d; return d <= tol;
}

int main(void) {
    int fails = 0;
    const int N = 9;
    vv_dsp_real x[N];
    for (int i=0;i<N;++i) x[i] = (vv_dsp_real)i; // linear ramp
    vv_dsp_real y[N];

    // Smoothing with polyorder=1, window=5 should reproduce linear exactly (deriv=0)
    vv_dsp_status s = vv_dsp_savgol(x, N, 5, 1, 0, (vv_dsp_real)1, VV_DSP_SAVGOL_MODE_REFLECT, y);
    if (s != VV_DSP_OK) { fprintf(stderr, "savgol smoothing status=%d\n", (int)s); return 1; }
    // Exact on interior (edges may be affected by padding)
    for (int i=2;i<N-2;++i) if (!almost_equal(y[i], x[i], (vv_dsp_real)1e-4)) { fprintf(stderr, "smoothing mismatch at %d: got %f expect %f\n", i, (double)y[i], (double)x[i]); fails++; break; }

    // First derivative of quadratic x^2 should be 2x; we test around center using polyorder>=2
    for (int i=0;i<N;++i) x[i] = (vv_dsp_real)(i*i);
    s = vv_dsp_savgol(x, N, 5, 2, 1, (vv_dsp_real)1, VV_DSP_SAVGOL_MODE_REFLECT, y);
    if (s != VV_DSP_OK) { fprintf(stderr, "savgol deriv1 status=%d\n", (int)s); return 1; }
    for (int i=2;i<N-2;++i) {
        vv_dsp_real expect = (vv_dsp_real)(2*i);
        if (!almost_equal(y[i], expect, (vv_dsp_real)1e-2)) { fprintf(stderr, "deriv1 mismatch at %d: got %f expect %f\n", i, (double)y[i], (double)expect); fails++; break; }
    }

    if (fails) { fprintf(stderr, "savgol_basic_func_tests: %d failures\n", fails); return 1; }
    printf("savgol_basic_func_tests: OK\n");
    return 0;
}
