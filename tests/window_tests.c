#include <stdio.h>
#include <math.h>
#include <string.h>
#include "vv_dsp/vv_dsp.h"

static int almost_equal(vv_dsp_real a, vv_dsp_real b, vv_dsp_real tol) {
    vv_dsp_real da = a - b;
    if (da < 0) da = -da;
    return da <= tol;
}

static void compute_ref_hann(size_t N, vv_dsp_real* out) {
    if (N == 0) return;
    if (N == 1) { out[0] = (vv_dsp_real)1.0; return; }
    vv_dsp_real denom = (vv_dsp_real)(N - 1);
    for (size_t n = 0; n < N; ++n) {
    out[n] = (vv_dsp_real)0.5 - (vv_dsp_real)0.5 * (vv_dsp_real)VV_DSP_COS((vv_dsp_real)(VV_DSP_TWO_PI) * (vv_dsp_real)n / denom);
    }
}

static void compute_ref_hamming(size_t N, vv_dsp_real* out) {
    if (N == 0) return;
    if (N == 1) { out[0] = (vv_dsp_real)1.0; return; }
    vv_dsp_real denom = (vv_dsp_real)(N - 1);
    for (size_t n = 0; n < N; ++n) {
    out[n] = (vv_dsp_real)0.54 - (vv_dsp_real)0.46 * (vv_dsp_real)VV_DSP_COS((vv_dsp_real)(VV_DSP_TWO_PI) * (vv_dsp_real)n / denom);
    }
}

static void compute_ref_blackman(size_t N, vv_dsp_real* out) {
    if (N == 0) return;
    if (N == 1) { out[0] = (vv_dsp_real)1.0; return; }
    vv_dsp_real denom = (vv_dsp_real)(N - 1);
    for (size_t n = 0; n < N; ++n) {
    vv_dsp_real x = (vv_dsp_real)(VV_DSP_TWO_PI) * (vv_dsp_real)n / denom;
    out[n] = (vv_dsp_real)0.42 - (vv_dsp_real)0.5*(vv_dsp_real)VV_DSP_COS(x) + (vv_dsp_real)0.08*(vv_dsp_real)VV_DSP_COS((vv_dsp_real)2.0*x);
    }
}

static void compute_ref_bharris(size_t N, vv_dsp_real* out) {
    if (N == 0) return;
    if (N == 1) { out[0] = (vv_dsp_real)1.0; return; }
    const vv_dsp_real a0 = (vv_dsp_real)0.35875;
    const vv_dsp_real a1 = (vv_dsp_real)0.48829;
    const vv_dsp_real a2 = (vv_dsp_real)0.14128;
    const vv_dsp_real a3 = (vv_dsp_real)0.01168;
    vv_dsp_real denom = (vv_dsp_real)(N - 1);
    for (size_t n = 0; n < N; ++n) {
    vv_dsp_real x = (vv_dsp_real)(VV_DSP_TWO_PI) * (vv_dsp_real)n / denom;
    out[n] = a0 - a1*(vv_dsp_real)VV_DSP_COS(x) + a2*(vv_dsp_real)VV_DSP_COS((vv_dsp_real)2.0*x) - a3*(vv_dsp_real)VV_DSP_COS((vv_dsp_real)3.0*x);
    }
}

static void compute_ref_nuttall(size_t N, vv_dsp_real* out) {
    if (N == 0) return;
    if (N == 1) { out[0] = (vv_dsp_real)1.0; return; }
    const vv_dsp_real a0 = (vv_dsp_real)0.3635819;
    const vv_dsp_real a1 = (vv_dsp_real)0.4891775;
    const vv_dsp_real a2 = (vv_dsp_real)0.1365995;
    const vv_dsp_real a3 = (vv_dsp_real)0.0106411;
    vv_dsp_real denom = (vv_dsp_real)(N - 1);
    for (size_t n = 0; n < N; ++n) {
    vv_dsp_real x = (vv_dsp_real)(VV_DSP_TWO_PI) * (vv_dsp_real)n / denom;
    out[n] = a0 - a1*(vv_dsp_real)VV_DSP_COS(x) + a2*(vv_dsp_real)VV_DSP_COS((vv_dsp_real)2.0*x) - a3*(vv_dsp_real)VV_DSP_COS((vv_dsp_real)3.0*x);
    }
}

static void compute_ref_bartlett(size_t N, vv_dsp_real* out) {
    if (N == 0) return;
    if (N == 1) { out[0] = (vv_dsp_real)1.0; return; }
    const vv_dsp_real half_N_minus_1 = (vv_dsp_real)(N - 1) / (vv_dsp_real)2.0;
    for (size_t n = 0; n < N; ++n) {
        vv_dsp_real diff = (vv_dsp_real)n - half_N_minus_1;
        if (diff < 0) diff = -diff; // abs(diff)
        out[n] = (vv_dsp_real)1.0 - diff / half_N_minus_1;
    }
}

static void compute_ref_bohman(size_t N, vv_dsp_real* out) {
    if (N == 0) return;
    if (N == 1) { out[0] = (vv_dsp_real)1.0; return; }
    const vv_dsp_real denom = (vv_dsp_real)(N - 1);
    for (size_t n = 0; n < N; ++n) {
        vv_dsp_real x = ((vv_dsp_real)n / denom - (vv_dsp_real)0.5) * (vv_dsp_real)2.0;
        if (x < 0) x = -x; // abs(x)
        if (x <= (vv_dsp_real)1.0) {
            vv_dsp_real pi_x = VV_DSP_PI * x;
            out[n] = ((vv_dsp_real)1.0 - x) * (vv_dsp_real)VV_DSP_COS(pi_x) + 
                     (vv_dsp_real)VV_DSP_SIN(pi_x) / VV_DSP_PI;
        } else {
            out[n] = (vv_dsp_real)0.0;
        }
    }
}

static void compute_ref_cosine(size_t N, vv_dsp_real* out) {
    if (N == 0) return;
    if (N == 1) { out[0] = (vv_dsp_real)1.0; return; }
    const vv_dsp_real denom = (vv_dsp_real)(N - 1);
    const vv_dsp_real pi_over = VV_DSP_PI / denom;
    for (size_t n = 0; n < N; ++n) {
        out[n] = (vv_dsp_real)VV_DSP_SIN(pi_over * (vv_dsp_real)n);
    }
}

static int test_validation(void) {
    vv_dsp_status st;
    st = vv_dsp_window_boxcar(0, NULL); // undefined precedence between errors; only check later exact ones
    (void)st;
    st = vv_dsp_window_boxcar(0, (vv_dsp_real*)0x1);
    if (st != VV_DSP_ERROR_INVALID_SIZE) return 1;
    st = vv_dsp_window_boxcar(4, NULL);
    if (st != VV_DSP_ERROR_NULL_POINTER) return 2;
    return 0;
}

static int test_symmetry_and_values(void) {
    const vv_dsp_real tol = (vv_dsp_real)1e-5f;
    const size_t N = 17;
    vv_dsp_real got[17], ref[17];

    memset(got, 0, sizeof(got));
    if (vv_dsp_window_hann(N, got) != VV_DSP_OK) return 10;
    compute_ref_hann(N, ref);
    for (size_t n = 0; n < N; ++n) {
        if (!almost_equal(got[n], ref[n], tol)) return 11;
        if (!almost_equal(got[n], got[N-1-n], tol)) return 12;
    }

    memset(got, 0, sizeof(got));
    if (vv_dsp_window_hamming(N, got) != VV_DSP_OK) return 13;
    compute_ref_hamming(N, ref);
    for (size_t n = 0; n < N; ++n) {
        if (!almost_equal(got[n], ref[n], tol)) return 14;
        if (!almost_equal(got[n], got[N-1-n], tol)) return 15;
    }

    memset(got, 0, sizeof(got));
    if (vv_dsp_window_blackman(N, got) != VV_DSP_OK) return 16;
    compute_ref_blackman(N, ref);
    for (size_t n = 0; n < N; ++n) {
        if (!almost_equal(got[n], ref[n], tol)) return 17;
        if (!almost_equal(got[n], got[N-1-n], tol)) return 18;
    }

    memset(got, 0, sizeof(got));
    if (vv_dsp_window_blackman_harris(N, got) != VV_DSP_OK) return 19;
    compute_ref_bharris(N, ref);
    for (size_t n = 0; n < N; ++n) {
        if (!almost_equal(got[n], ref[n], tol)) return 20;
        if (!almost_equal(got[n], got[N-1-n], tol)) return 21;
    }

    memset(got, 0, sizeof(got));
    if (vv_dsp_window_nuttall(N, got) != VV_DSP_OK) return 22;
    compute_ref_nuttall(N, ref);
    for (size_t n = 0; n < N; ++n) {
        if (!almost_equal(got[n], ref[n], tol)) return 23;
        if (!almost_equal(got[n], got[N-1-n], tol)) return 24;
    }

    memset(got, 0, sizeof(got));
    if (vv_dsp_window_boxcar(N, got) != VV_DSP_OK) return 25;
    for (size_t n = 0; n < N; ++n) {
        if (!almost_equal(got[n], (vv_dsp_real)1.0, tol)) return 26;
    }

    // Test new non-parameterized window functions
    memset(got, 0, sizeof(got));
    if (vv_dsp_window_bartlett(N, got) != VV_DSP_OK) return 27;
    compute_ref_bartlett(N, ref);
    for (size_t n = 0; n < N; ++n) {
        if (!almost_equal(got[n], ref[n], tol)) return 28;
        if (!almost_equal(got[n], got[N-1-n], tol)) return 29;
    }

    memset(got, 0, sizeof(got));
    if (vv_dsp_window_bohman(N, got) != VV_DSP_OK) return 30;
    compute_ref_bohman(N, ref);
    for (size_t n = 0; n < N; ++n) {
        if (!almost_equal(got[n], ref[n], tol)) return 31;
        if (!almost_equal(got[n], got[N-1-n], tol)) return 32;
    }

    memset(got, 0, sizeof(got));
    if (vv_dsp_window_cosine(N, got) != VV_DSP_OK) return 33;
    compute_ref_cosine(N, ref);
    for (size_t n = 0; n < N; ++n) {
        if (!almost_equal(got[n], ref[n], tol)) return 34;
        if (!almost_equal(got[n], got[N-1-n], tol)) return 35;
    }

    // Test basic symmetry for planck_taper (no reference implementation for now)
    memset(got, 0, sizeof(got));
    if (vv_dsp_window_planck_taper(N, got) != VV_DSP_OK) return 36;
    for (size_t n = 0; n < N; ++n) {
        if (!almost_equal(got[n], got[N-1-n], tol)) return 37;
    }
    
    return 0;
}

static int test_N_eq_1(void) {
    vv_dsp_real w[1];
    if (vv_dsp_window_hann(1, w) != VV_DSP_OK || !almost_equal(w[0], (vv_dsp_real)1.0, (vv_dsp_real)1e-6)) return 30;
    if (vv_dsp_window_hamming(1, w) != VV_DSP_OK || !almost_equal(w[0], (vv_dsp_real)1.0, (vv_dsp_real)1e-6)) return 31;
    if (vv_dsp_window_blackman(1, w) != VV_DSP_OK || !almost_equal(w[0], (vv_dsp_real)1.0, (vv_dsp_real)1e-6)) return 32;
    if (vv_dsp_window_blackman_harris(1, w) != VV_DSP_OK || !almost_equal(w[0], (vv_dsp_real)1.0, (vv_dsp_real)1e-6)) return 33;
    if (vv_dsp_window_nuttall(1, w) != VV_DSP_OK || !almost_equal(w[0], (vv_dsp_real)1.0, (vv_dsp_real)1e-6)) return 34;
    if (vv_dsp_window_boxcar(1, w) != VV_DSP_OK || !almost_equal(w[0], (vv_dsp_real)1.0, (vv_dsp_real)1e-6)) return 35;
    
    // Test new non-parameterized window functions for N=1
    if (vv_dsp_window_bartlett(1, w) != VV_DSP_OK || !almost_equal(w[0], (vv_dsp_real)1.0, (vv_dsp_real)1e-6)) return 36;
    if (vv_dsp_window_bohman(1, w) != VV_DSP_OK || !almost_equal(w[0], (vv_dsp_real)1.0, (vv_dsp_real)1e-6)) return 37;
    if (vv_dsp_window_cosine(1, w) != VV_DSP_OK || !almost_equal(w[0], (vv_dsp_real)1.0, (vv_dsp_real)1e-6)) return 38;
    if (vv_dsp_window_planck_taper(1, w) != VV_DSP_OK || !almost_equal(w[0], (vv_dsp_real)1.0, (vv_dsp_real)1e-6)) return 39;
    
    return 0;
}

static int test_parameterized_windows(void) {
    const vv_dsp_real tol = (vv_dsp_real)1e-5f;
    const size_t N = 17;
    vv_dsp_real got[17];

    // Test flattop window
    memset(got, 0, sizeof(got));
    if (vv_dsp_window_flattop(N, got) != VV_DSP_OK) return 40;
    // Basic symmetry test
    for (size_t n = 0; n < N; ++n) {
        if (!almost_equal(got[n], got[N-1-n], tol)) return 41;
    }

    // Test kaiser window with different beta values
    memset(got, 0, sizeof(got));
    if (vv_dsp_window_kaiser(N, (vv_dsp_real)0.0, got) != VV_DSP_OK) return 42;
    // With beta=0, should be close to rectangular window
    for (size_t n = 0; n < N; ++n) {
        if (!almost_equal(got[n], got[N-1-n], tol)) return 43;
    }

    memset(got, 0, sizeof(got));
    if (vv_dsp_window_kaiser(N, (vv_dsp_real)5.0, got) != VV_DSP_OK) return 44;
    for (size_t n = 0; n < N; ++n) {
        if (!almost_equal(got[n], got[N-1-n], tol)) return 45;
    }

    // Test tukey window with different alpha values
    memset(got, 0, sizeof(got));
    if (vv_dsp_window_tukey(N, (vv_dsp_real)0.0, got) != VV_DSP_OK) return 46;
    // With alpha=0, should be rectangular
    for (size_t n = 0; n < N; ++n) {
        if (!almost_equal(got[n], (vv_dsp_real)1.0, tol)) return 47;
    }

    memset(got, 0, sizeof(got));
    if (vv_dsp_window_tukey(N, (vv_dsp_real)1.0, got) != VV_DSP_OK) return 48;
    // With alpha=1, should be close to Hann window
    for (size_t n = 0; n < N; ++n) {
        if (!almost_equal(got[n], got[N-1-n], tol)) return 49;
    }

    memset(got, 0, sizeof(got));
    if (vv_dsp_window_tukey(N, (vv_dsp_real)0.5, got) != VV_DSP_OK) return 50;
    for (size_t n = 0; n < N; ++n) {
        if (!almost_equal(got[n], got[N-1-n], tol)) return 51;
    }

    // Test N=1 for parameterized functions
    vv_dsp_real w[1];
    if (vv_dsp_window_flattop(1, w) != VV_DSP_OK || !almost_equal(w[0], (vv_dsp_real)1.0, (vv_dsp_real)1e-6)) return 52;
    if (vv_dsp_window_kaiser(1, (vv_dsp_real)5.0, w) != VV_DSP_OK || !almost_equal(w[0], (vv_dsp_real)1.0, (vv_dsp_real)1e-6)) return 53;
    if (vv_dsp_window_tukey(1, (vv_dsp_real)0.5, w) != VV_DSP_OK || !almost_equal(w[0], (vv_dsp_real)1.0, (vv_dsp_real)1e-6)) return 54;

    return 0;
}

int main(void) {
    int rc;
    if ((rc = test_validation()) != 0) { printf("validation failed: %d\n", rc); return rc; }
    if ((rc = test_N_eq_1()) != 0) { printf("N==1 failed: %d\n", rc); return rc; }
    if ((rc = test_symmetry_and_values()) != 0) { printf("symmetry/values failed: %d\n", rc); return rc; }
    if ((rc = test_parameterized_windows()) != 0) { printf("parameterized windows failed: %d\n", rc); return rc; }
    printf("window tests passed\n");
    return 0;
}
