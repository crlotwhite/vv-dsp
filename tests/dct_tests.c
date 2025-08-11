#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "vv_dsp/vv_dsp.h"

static int nearly_equal(vv_dsp_real a, vv_dsp_real b, vv_dsp_real tol) {
    vv_dsp_real d = a - b; if (d < 0) d = -d; return d <= tol;
}

static int test_dct2_iii_roundtrip(size_t n) {
    vv_dsp_real* x = (vv_dsp_real*)malloc(n * sizeof(vv_dsp_real));
    vv_dsp_real* X = (vv_dsp_real*)malloc(n * sizeof(vv_dsp_real));
    vv_dsp_real* xr = (vv_dsp_real*)malloc(n * sizeof(vv_dsp_real));
    if (!x || !X || !xr) return 0;
    for (size_t i=0;i<n;++i) x[i] = (vv_dsp_real)sin(2.0 * 3.14159265358979323846 * (vv_dsp_real)i / (vv_dsp_real)(n));
    if (vv_dsp_dct_forward(n, VV_DSP_DCT_II, x, X) != VV_DSP_OK) return 0;
    if (vv_dsp_dct_inverse(n, VV_DSP_DCT_II, X, xr) != VV_DSP_OK) return 0;
    int ok = 1; for (size_t i=0;i<n;++i) if (!nearly_equal(x[i], xr[i], (vv_dsp_real)1e-5)) { ok = 0; break; }
    free(x); free(X); free(xr); return ok;
}

static int test_dct4_involution(size_t n) {
    vv_dsp_real* x = (vv_dsp_real*)malloc(n * sizeof(vv_dsp_real));
    vv_dsp_real* X = (vv_dsp_real*)malloc(n * sizeof(vv_dsp_real));
    vv_dsp_real* xr = (vv_dsp_real*)malloc(n * sizeof(vv_dsp_real));
    if (!x || !X || !xr) return 0;
    for (size_t i=0;i<n;++i) x[i] = (vv_dsp_real)cos(2.0 * 3.14159265358979323846 * (vv_dsp_real)(i+0.3) / (vv_dsp_real)(n));
    if (vv_dsp_dct_forward(n, VV_DSP_DCT_IV, x, X) != VV_DSP_OK) return 0;
    if (vv_dsp_dct_inverse(n, VV_DSP_DCT_IV, X, xr) != VV_DSP_OK) return 0;
    int ok = 1; for (size_t i=0;i<n;++i) if (!nearly_equal(x[i], xr[i], (vv_dsp_real)1e-5)) { ok = 0; break; }
    free(x); free(X); free(xr); return ok;
}

int main(void) {
    if (!test_dct2_iii_roundtrip(8)) { fprintf(stderr, "DCT-II/III roundtrip failed\n"); return 1; }
    if (!test_dct4_involution(8)) { fprintf(stderr, "DCT-IV involution failed\n"); return 1; }
    printf("dct tests passed\n");
    return 0;
}
