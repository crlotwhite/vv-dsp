#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "vv_dsp/vv_dsp.h"

static int approx_equal(vv_dsp_real a, vv_dsp_real b, vv_dsp_real tol) {
    vv_dsp_real d = (a > b) ? (a - b) : (b - a);
    return d <= tol;
}

static int test_interpolate_linear_basic(void) {
    vv_dsp_real x[4] = {0.0f, 1.0f, 3.0f, 6.0f};
    vv_dsp_real y = 0;
    // midpoint between 1 and 3 -> 2
    if (vv_dsp_interpolate_linear_real(x, 4, 2.0f - 0.5f, &y) != VV_DSP_OK) return 0;
    if (!approx_equal(y, (vv_dsp_real)2.0, (vv_dsp_real)1e-5)) return 0;
    // clamp edges
    if (vv_dsp_interpolate_linear_real(x, 4, -10.0f, &y) != VV_DSP_OK) return 0;
    if (!approx_equal(y, (vv_dsp_real)0.0, (vv_dsp_real)1e-5)) return 0;
    if (vv_dsp_interpolate_linear_real(x, 4, 100.0f, &y) != VV_DSP_OK) return 0;
    if (!approx_equal(y, (vv_dsp_real)6.0, (vv_dsp_real)1e-5)) return 0;
    return 1;
}

static int test_resampler_up_down_roundtrip(void) {
    const unsigned int Fs = 48000;
    const double f = 1000.0; // 1 kHz tone
    const size_t N = 480;    // 10ms
    vv_dsp_real x[N];
    for (size_t n = 0; n < N; ++n) {
    x[n] = (vv_dsp_real)sin(VV_DSP_TWO_PI_D * f * (double)n / (double)Fs);
    }

    // Upsample by 2 -> ratio 2/1
    vv_dsp_resampler* up = vv_dsp_resampler_create(2, 1);
    if (!up) return 0;
    vv_dsp_resampler_set_quality(up, 1, 32);
    size_t up_cap = (size_t)floor((N - 1) * 2.0) + 1;
    vv_dsp_real* xu = (vv_dsp_real*)malloc(sizeof(vv_dsp_real) * up_cap);
    if (!xu) { vv_dsp_resampler_destroy(up); return 0; }
    size_t up_len = 0;
    if (vv_dsp_resampler_process_real(up, x, N, xu, up_cap, &up_len) != VV_DSP_OK) {
        free(xu); vv_dsp_resampler_destroy(up); return 0;
    }
    if (up_len != up_cap) { free(xu); vv_dsp_resampler_destroy(up); return 0; }

    // Downsample by 2 -> ratio 1/2
    vv_dsp_resampler* down = vv_dsp_resampler_create(1, 2);
    if (!down) { free(xu); vv_dsp_resampler_destroy(up); return 0; }
    vv_dsp_resampler_set_quality(down, 1, 32);
    size_t dn_cap = (size_t)floor((up_len - 1) * 0.5) + 1;
    vv_dsp_real* xd = (vv_dsp_real*)malloc(sizeof(vv_dsp_real) * dn_cap);
    if (!xd) { free(xu); vv_dsp_resampler_destroy(up); vv_dsp_resampler_destroy(down); return 0; }
    size_t dn_len = 0;
    if (vv_dsp_resampler_process_real(down, xu, up_len, xd, dn_cap, &dn_len) != VV_DSP_OK) {
        free(xu); free(xd); vv_dsp_resampler_destroy(up); vv_dsp_resampler_destroy(down); return 0;
    }
    if (dn_len != N) { free(xu); free(xd); vv_dsp_resampler_destroy(up); vv_dsp_resampler_destroy(down); return 0; }

    // Compare round-trip to original (allow small error)
    vv_dsp_real err = 0;
    for (size_t n = 0; n < N; ++n) {
        vv_dsp_real d = xd[n] - x[n];
        err += (vv_dsp_real)fabs((double)d);
    }
    err /= (vv_dsp_real)N;

    free(xu);
    free(xd);
    vv_dsp_resampler_destroy(up);
    vv_dsp_resampler_destroy(down);

    // Expect average absolute error reasonably small for a simple sinc window
    return (err < (vv_dsp_real)0.1) ? 1 : 0;
}

int main(void) {
    int ok = 1;
    ok &= test_interpolate_linear_basic();
    ok &= test_resampler_up_down_roundtrip();
    if (!ok) {
        fprintf(stderr, "resample tests failed\n");
        return 1;
    }
    printf("resample tests passed\n");
    return 0;
}
