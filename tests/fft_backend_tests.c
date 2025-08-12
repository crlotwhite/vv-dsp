#include <stdio.h>
#include <math.h>
#include <string.h>
#include "vv_dsp/vv_dsp.h"

#define N 16

#if defined(VV_DSP_USE_DOUBLE)
#define TOL 1e-12
#else
#define TOL 1e-5f
#endif

static int nearly_equal(vv_dsp_real a, vv_dsp_real b, vv_dsp_real tol) {
    vv_dsp_real d = a - b;
    if (d < 0) d = -d;
    return d <= tol;
}

static int test_fft_backend_availability(void) {
    printf("Testing FFT backend availability:\n");
    
    // Test if KissFFT is available (should always be available)
    printf("  KissFFT: ");
#ifdef VV_DSP_BACKEND_FFT_kissfft
    printf("AVAILABLE\n");
#else
    printf("NOT AVAILABLE\n");
    return 0; // KissFFT should always be available
#endif

    // Test if FFTW is available
    printf("  FFTW3: ");
#ifdef VV_DSP_BACKEND_FFT_fftw
    printf("AVAILABLE\n");
#else
    printf("NOT AVAILABLE\n");
#endif

    // Test if FFTS is available
    printf("  FFTS: ");
#ifdef VV_DSP_BACKEND_FFT_ffts
    printf("AVAILABLE\n");
#else
    printf("NOT AVAILABLE\n");
#endif

    return 1;
}

static int test_fft_backend_basic_functionality(const char* backend_name) {
    printf("Testing basic FFT functionality with %s backend:\n", backend_name);
    
    const size_t n = N;
    vv_dsp_fft_plan* plan_f = NULL;
    vv_dsp_fft_plan* plan_b = NULL;
    
    // Create forward and backward plans
    if (vv_dsp_fft_make_plan(n, VV_DSP_FFT_C2C, VV_DSP_FFT_FORWARD, &plan_f) != VV_DSP_OK) {
        printf("  Failed to create forward plan\n");
        return 0;
    }
    
    if (vv_dsp_fft_make_plan(n, VV_DSP_FFT_C2C, VV_DSP_FFT_BACKWARD, &plan_b) != VV_DSP_OK) {
        printf("  Failed to create backward plan\n");
        vv_dsp_fft_destroy(plan_f);
        return 0;
    }
    
    // Test data: impulse signal
    vv_dsp_cpx x[N], X[N], x_recovered[N];
    memset(x, 0, sizeof(x));
    memset(X, 0, sizeof(X));
    memset(x_recovered, 0, sizeof(x_recovered));
    
    // Create impulse at t=0
    x[0] = vv_dsp_cpx_make((vv_dsp_real)1.0, (vv_dsp_real)0.0);
    
    // Forward FFT
    if (vv_dsp_fft_execute(plan_f, x, X) != VV_DSP_OK) {
        printf("  Forward FFT failed\n");
        vv_dsp_fft_destroy(plan_f);
        vv_dsp_fft_destroy(plan_b);
        return 0;
    }
    
    // Check if forward FFT produces expected result (impulse -> flat spectrum)
    int forward_ok = 1;
    for (size_t k = 0; k < n; ++k) {
        if (!nearly_equal(X[k].re, (vv_dsp_real)1.0, TOL) || !nearly_equal(X[k].im, (vv_dsp_real)0.0, TOL)) {
#if defined(VV_DSP_USE_DOUBLE)
            printf("  Forward FFT: unexpected value at bin %zu: (%g, %g)\n", k, X[k].re, X[k].im);
#else
            printf("  Forward FFT: unexpected value at bin %zu: (%g, %g)\n", k, (double)X[k].re, (double)X[k].im);
#endif
            forward_ok = 0;
            break;
        }
    }
    
    if (!forward_ok) {
        vv_dsp_fft_destroy(plan_f);
        vv_dsp_fft_destroy(plan_b);
        return 0;
    }
    printf("  Forward FFT: PASS\n");
    
    // Backward FFT
    if (vv_dsp_fft_execute(plan_b, X, x_recovered) != VV_DSP_OK) {
        printf("  Backward FFT failed\n");
        vv_dsp_fft_destroy(plan_f);
        vv_dsp_fft_destroy(plan_b);
        return 0;
    }
    
    // Check roundtrip accuracy (no normalization needed - different backends handle this differently)
    int roundtrip_ok = 1;
    for (size_t i = 0; i < n; ++i) {
        vv_dsp_real expected_re = x[i].re;
        vv_dsp_real expected_im = x[i].im;
        vv_dsp_real actual_re = x_recovered[i].re;
        vv_dsp_real actual_im = x_recovered[i].im;
        
        // Try with and without normalization
        vv_dsp_real norm_actual_re = actual_re / (vv_dsp_real)n;
        vv_dsp_real norm_actual_im = actual_im / (vv_dsp_real)n;
        
        // Check if either normalized or unnormalized matches
        int matches_unnorm = nearly_equal(actual_re, expected_re, TOL) && nearly_equal(actual_im, expected_im, TOL);
        int matches_norm = nearly_equal(norm_actual_re, expected_re, TOL) && nearly_equal(norm_actual_im, expected_im, TOL);
        
        if (!matches_unnorm && !matches_norm) {
#if defined(VV_DSP_USE_DOUBLE)
            printf("  Roundtrip: mismatch at sample %zu: expected (%g, %g), got (%g, %g), normalized (%g, %g)\n", 
                   i, expected_re, expected_im, actual_re, actual_im, norm_actual_re, norm_actual_im);
#else
            printf("  Roundtrip: mismatch at sample %zu: expected (%g, %g), got (%g, %g), normalized (%g, %g)\n", 
                   i, (double)expected_re, (double)expected_im, (double)actual_re, (double)actual_im, (double)norm_actual_re, (double)norm_actual_im);
#endif
            roundtrip_ok = 0;
            break;
        }
    }
    
    vv_dsp_fft_destroy(plan_f);
    vv_dsp_fft_destroy(plan_b);
    
    if (roundtrip_ok) {
        printf("  Backward FFT & Roundtrip: PASS\n");
        return 1;
    } else {
        return 0;
    }
}

static int test_real_fft_backend(const char* backend_name) {
    printf("Testing real FFT functionality with %s backend:\n", backend_name);
    
    const size_t n = N;
    vv_dsp_fft_plan* plan_f = NULL;
    vv_dsp_fft_plan* plan_b = NULL;
    
    // Create R2C and C2R plans
    if (vv_dsp_fft_make_plan(n, VV_DSP_FFT_R2C, VV_DSP_FFT_FORWARD, &plan_f) != VV_DSP_OK) {
        printf("  Failed to create R2C plan\n");
        return 0;
    }
    
    if (vv_dsp_fft_make_plan(n, VV_DSP_FFT_C2R, VV_DSP_FFT_BACKWARD, &plan_b) != VV_DSP_OK) {
        printf("  Failed to create C2R plan\n");
        vv_dsp_fft_destroy(plan_f);
        return 0;
    }
    
    // Test data: sine wave
    vv_dsp_real xr[N], xr_recovered[N];
    vv_dsp_cpx X[N/2 + 1];
    
    const double PI = 3.14159265358979323846264338327950288;
    for (size_t i = 0; i < n; ++i) {
#if defined(VV_DSP_USE_DOUBLE)
        xr[i] = (vv_dsp_real)sin(2.0*PI*(double)i/(double)n);
#else
        xr[i] = (vv_dsp_real)sinf((vv_dsp_real)2.0*(vv_dsp_real)PI*(vv_dsp_real)i/(vv_dsp_real)n);
#endif
    }
    
    // Forward R2C FFT
    if (vv_dsp_fft_execute(plan_f, xr, X) != VV_DSP_OK) {
        printf("  R2C FFT failed\n");
        vv_dsp_fft_destroy(plan_f);
        vv_dsp_fft_destroy(plan_b);
        return 0;
    }
    printf("  R2C FFT: PASS\n");
    
    // Backward C2R FFT
    if (vv_dsp_fft_execute(plan_b, X, xr_recovered) != VV_DSP_OK) {
        printf("  C2R FFT failed\n");
        vv_dsp_fft_destroy(plan_f);
        vv_dsp_fft_destroy(plan_b);
        return 0;
    }
    
    // Check roundtrip accuracy
    int roundtrip_ok = 1;
    for (size_t i = 0; i < n; ++i) {
        vv_dsp_real expected = xr[i];
        vv_dsp_real actual = xr_recovered[i];
        vv_dsp_real norm_actual = actual / (vv_dsp_real)n;
        
        // Check if either normalized or unnormalized matches
        int matches_unnorm = nearly_equal(actual, expected, TOL);
        int matches_norm = nearly_equal(norm_actual, expected, TOL);
        
        if (!matches_unnorm && !matches_norm) {
#if defined(VV_DSP_USE_DOUBLE)
            printf("  Real roundtrip: mismatch at sample %zu: expected %g, got %g, normalized %g\n", 
                   i, expected, actual, norm_actual);
#else
            printf("  Real roundtrip: mismatch at sample %zu: expected %g, got %g, normalized %g\n", 
                   i, (double)expected, (double)actual, (double)norm_actual);
#endif
            roundtrip_ok = 0;
            break;
        }
    }
    
    vv_dsp_fft_destroy(plan_f);
    vv_dsp_fft_destroy(plan_b);
    
    if (roundtrip_ok) {
        printf("  C2R FFT & Real Roundtrip: PASS\n");
        return 1;
    } else {
        return 0;
    }
}

int main(void) {
    printf("VV-DSP FFT Backend Tests\n");
    printf("========================\n\n");
    
    int tests_passed = 0;
    int total_tests = 0;
    
    // Test backend availability
    total_tests++;
    if (test_fft_backend_availability()) {
        tests_passed++;
        printf("Backend availability test: PASS\n\n");
    } else {
        printf("Backend availability test: FAIL\n\n");
        return 1;
    }
    
    // Test each available backend
#ifdef VV_DSP_BACKEND_FFT_kissfft
    total_tests += 2;
    if (test_fft_backend_basic_functionality("KissFFT")) {
        tests_passed++;
    }
    if (test_real_fft_backend("KissFFT")) {
        tests_passed++;
    }
    printf("\n");
#endif

#ifdef VV_DSP_BACKEND_FFT_fftw
    total_tests += 2;
    if (test_fft_backend_basic_functionality("FFTW3")) {
        tests_passed++;
    }
    if (test_real_fft_backend("FFTW3")) {
        tests_passed++;
    }
    printf("\n");
#endif

#ifdef VV_DSP_BACKEND_FFT_ffts
    total_tests += 2;
    if (test_fft_backend_basic_functionality("FFTS")) {
        tests_passed++;
    }
    if (test_real_fft_backend("FFTS")) {
        tests_passed++;
    }
    printf("\n");
#endif
    
    printf("Summary: %d/%d tests passed\n", tests_passed, total_tests);
    
    if (tests_passed == total_tests) {
        printf("All FFT backend tests PASSED!\n");
        return 0;
    } else {
        printf("Some FFT backend tests FAILED!\n");
        return 1;
    }
}
