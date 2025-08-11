#include <stdio.h>
#include <math.h>
#include "vv_dsp/vv_dsp.h"

static int approx_equal(vv_dsp_real a, vv_dsp_real b) {
#ifdef VV_DSP_USE_DOUBLE
    const vv_dsp_real eps = (vv_dsp_real)1e-9;
#else
    const vv_dsp_real eps = (vv_dsp_real)1e-5f;
#endif
    vv_dsp_real d = a - b;
    if (d < 0) d = -d;
    return d <= eps;
}

int main(void) {
    int ok = 1;

    // Basic array ops
    vv_dsp_real x[5] = {1,2,3,4,5};
    vv_dsp_real out = 0; size_t idx = 999;
    ok &= (vv_dsp_sum(x, 5, &out) == VV_DSP_OK) && approx_equal(out, (vv_dsp_real)15);
    ok &= (vv_dsp_mean(x, 5, &out) == VV_DSP_OK) && approx_equal(out, (vv_dsp_real)3);
    ok &= (vv_dsp_var(x, 5, &out) == VV_DSP_OK) && approx_equal(out, (vv_dsp_real)2);
    ok &= (vv_dsp_min(x, 5, &out) == VV_DSP_OK) && approx_equal(out, (vv_dsp_real)1);
    ok &= (vv_dsp_max(x, 5, &out) == VV_DSP_OK) && approx_equal(out, (vv_dsp_real)5);
    ok &= (vv_dsp_argmin(x, 5, &idx) == VV_DSP_OK) && (idx == 0);
    ok &= (vv_dsp_argmax(x, 5, &idx) == VV_DSP_OK) && (idx == 4);

    vv_dsp_real y[5] = {0};
    vv_dsp_real d[4] = {0};
    ok &= (vv_dsp_cumsum(x, 5, y) == VV_DSP_OK) && approx_equal(y[4], (vv_dsp_real)15);
    ok &= (vv_dsp_diff(x, 5, d) == VV_DSP_OK) && approx_equal(d[0], (vv_dsp_real)1) && approx_equal(d[3], (vv_dsp_real)1);

    // Utilities
    ok &= approx_equal(vv_dsp_clamp((vv_dsp_real)2, (vv_dsp_real)-1, (vv_dsp_real)1), (vv_dsp_real)1);
    ok &= approx_equal(vv_dsp_clamp((vv_dsp_real)-5, (vv_dsp_real)-1, (vv_dsp_real)1), (vv_dsp_real)-1);
    ok &= approx_equal(vv_dsp_clamp((vv_dsp_real)0.5, (vv_dsp_real)-1, (vv_dsp_real)1), (vv_dsp_real)0.5);
    vv_dsp_flush_denormals(); // no-op, just ensure it doesn't crash

    // Complex helpers
    vv_dsp_cpx a = vv_dsp_cpx_make((vv_dsp_real)1, (vv_dsp_real)2);
    vv_dsp_cpx b = vv_dsp_cpx_make((vv_dsp_real)3, (vv_dsp_real)4);
    vv_dsp_cpx c = vv_dsp_cpx_add(a, b);
    ok &= approx_equal(c.re, (vv_dsp_real)4) && approx_equal(c.im, (vv_dsp_real)6);

    vv_dsp_cpx m = vv_dsp_cpx_mul(a, b); // (1+2i)*(3+4i) = -5 + 10i
    ok &= approx_equal(m.re, (vv_dsp_real)-5) && approx_equal(m.im, (vv_dsp_real)10);

    vv_dsp_cpx cj = vv_dsp_cpx_conj(a);
    ok &= approx_equal(cj.re, a.re) && approx_equal(cj.im, (vv_dsp_real)-2);

    vv_dsp_real r = vv_dsp_cpx_abs(a);
    vv_dsp_real th = vv_dsp_cpx_phase(a);
    ok &= approx_equal(r, (vv_dsp_real)sqrt((double)5));
    ok &= approx_equal(th, (vv_dsp_real)atan2((double)2, (double)1));

    vv_dsp_cpx p = vv_dsp_cpx_from_polar(r, th);
    ok &= approx_equal(p.re, (vv_dsp_real)1) && approx_equal(p.im, (vv_dsp_real)2);

    // Error handling
    vv_dsp_real tmp;
    ok &= (vv_dsp_sum(x, 0, &tmp) != VV_DSP_OK);
    ok &= (vv_dsp_var(x, 1, &tmp) != VV_DSP_OK);
    ok &= (vv_dsp_diff(x, 1, &tmp) != VV_DSP_OK);

    // --- Statistics utilities ---
    {
        vv_dsp_real xrms[4] = { (vv_dsp_real)1, (vv_dsp_real)-1, (vv_dsp_real)1, (vv_dsp_real)-1 };
        vv_dsp_real rmsv = 0;
        ok &= (vv_dsp_rms(xrms, 4, &rmsv) == VV_DSP_OK) && approx_equal(rmsv, (vv_dsp_real)1);

        vv_dsp_real mn = 0, mx = 0;
        ok &= (vv_dsp_peak(x, 5, &mn, &mx) == VV_DSP_OK) && approx_equal(mn, (vv_dsp_real)1) && approx_equal(mx, (vv_dsp_real)5);

        vv_dsp_real cf = 0;
        ok &= (vv_dsp_crest_factor(xrms, 4, &cf) == VV_DSP_OK) && approx_equal(cf, (vv_dsp_real)1);

        size_t zc = 999;
        ok &= (vv_dsp_zero_crossing_rate(xrms, 4, &zc) == VV_DSP_OK) && (zc == 3);

        vv_dsp_real skew = 0, kurt = 0;
        // For symmetric sequence around mean, skew ~ 0
        vv_dsp_real symm[5] = { (vv_dsp_real)-2, (vv_dsp_real)-1, (vv_dsp_real)0, (vv_dsp_real)1, (vv_dsp_real)2 };
        ok &= (vv_dsp_skewness(symm, 5, &skew) == VV_DSP_OK) && approx_equal(skew, (vv_dsp_real)0);
        // Excess kurtosis of this discrete set isn't zero necessarily, just ensure it computes
        ok &= (vv_dsp_kurtosis(symm, 5, &kurt) == VV_DSP_OK);

        // Autocorrelation simple check on xrms pattern: r[0] = mean(x^2) = 1, r[1] ~ -1, r[2] = 1, biased
        vv_dsp_real rauto[3] = {0};
        ok &= (vv_dsp_autocorrelation(xrms, 4, rauto, 3, 1) == VV_DSP_OK);
        ok &= approx_equal(rauto[0], (vv_dsp_real)1);

        // Cross-correlation with itself should match autocorr biased when using our normalized average definition
        vv_dsp_real rcross[3] = {0};
        ok &= (vv_dsp_cross_correlation(xrms, 4, xrms, 4, rcross, 3) == VV_DSP_OK);
        // Not strictly equal to biased auto since cross divides by overlap count; just ensure sane values
        ok &= approx_equal(rcross[0], (vv_dsp_real)1);
    }

    if (!ok) {
        fprintf(stderr, "core_tests failed\n");
        return 1;
    }
    printf("core_tests passed\n");
    return 0;
}
