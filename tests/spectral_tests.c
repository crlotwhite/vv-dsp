#include <stdio.h>
#include <math.h>
#include <string.h>
#include "vv_dsp/vv_dsp.h"

#define N8 8

static int nearly_equal(vv_dsp_real a, vv_dsp_real b, vv_dsp_real tol) {
    vv_dsp_real d = a - b;
    if (d < 0) d = -d;
    return d <= tol;
}

static int test_fft_c2c_basic(void) {
    const size_t n = N8;
    vv_dsp_fft_plan* plan_f = NULL;
    if (vv_dsp_fft_make_plan(n, VV_DSP_FFT_C2C, VV_DSP_FFT_FORWARD, &plan_f) != VV_DSP_OK) return 0;
    vv_dsp_cpx x[N8];
    vv_dsp_cpx X[N8];
    memset(X, 0, sizeof(X));

    // Impulse at t=0 => flat spectrum (all ones)
    for (size_t i = 0; i < n; ++i) x[i] = vv_dsp_cpx_make((vv_dsp_real)0, (vv_dsp_real)0);
    x[0] = vv_dsp_cpx_make((vv_dsp_real)1, (vv_dsp_real)0);

    if (vv_dsp_fft_execute(plan_f, x, X) != VV_DSP_OK) { vv_dsp_fft_destroy(plan_f); return 0; }

    int ok = 1;
    for (size_t k = 0; k < n; ++k) {
        if (!nearly_equal(X[k].re, (vv_dsp_real)1.0, (vv_dsp_real)1e-4) || !nearly_equal(X[k].im, (vv_dsp_real)0.0, (vv_dsp_real)1e-4)) { ok = 0; break; }
    }

    vv_dsp_fft_destroy(plan_f);
    return ok;
}

static int test_fft_r2c_c2r_roundtrip(void) {
    const size_t n = N8;
    vv_dsp_fft_plan *pf = NULL, *pb = NULL;
    if (vv_dsp_fft_make_plan(n, VV_DSP_FFT_R2C, VV_DSP_FFT_FORWARD, &pf) != VV_DSP_OK) return 0;
    if (vv_dsp_fft_make_plan(n, VV_DSP_FFT_C2R, VV_DSP_FFT_BACKWARD, &pb) != VV_DSP_OK) { vv_dsp_fft_destroy(pf); return 0; }

    vv_dsp_real xr[N8];
    const double PI = 3.14159265358979323846264338327950288;
    for (size_t i = 0; i < n; ++i)
#if defined(VV_DSP_USE_DOUBLE)
    xr[i] = (vv_dsp_real)sin(2.0*PI*(double)i/(double)n);
#else
    xr[i] = (vv_dsp_real)sinf((vv_dsp_real)2.0*(vv_dsp_real)PI*(vv_dsp_real)i/(vv_dsp_real)n);
#endif

    vv_dsp_cpx Xh[N8/2+1];
    vv_dsp_real xr2[N8];

    if (vv_dsp_fft_execute(pf, xr, Xh) != VV_DSP_OK) { vv_dsp_fft_destroy(pf); vv_dsp_fft_destroy(pb); return 0; }
    if (vv_dsp_fft_execute(pb, Xh, xr2) != VV_DSP_OK) { vv_dsp_fft_destroy(pf); vv_dsp_fft_destroy(pb); return 0; }

    int ok = 1;
    for (size_t i = 0; i < n; ++i) {
        if (!nearly_equal(xr[i], xr2[i], (vv_dsp_real)1e-3)) { ok = 0; break; }
    }

    vv_dsp_fft_destroy(pf);
    vv_dsp_fft_destroy(pb);
    return ok;
}

int main(void) {
    int ok1 = test_fft_c2c_basic();
    int ok2 = test_fft_r2c_c2r_roundtrip();
    // fftshift/ifftshift
    vv_dsp_real a[5] = {(vv_dsp_real)0,(vv_dsp_real)1,(vv_dsp_real)2,(vv_dsp_real)3,(vv_dsp_real)4};
    vv_dsp_real s[5], si[5], r[5];
    vv_dsp_fftshift_real(a, s, 5);
    vv_dsp_ifftshift_real(s, si, 5);
    int ok3 = 1;
    for (int i=0;i<5;++i) r[i]=a[i];
    for (int i=0;i<5;++i) if (!nearly_equal(si[i], r[i], (vv_dsp_real)1e-6)) { ok3 = 0; break; }
    if (!ok1) { fprintf(stderr, "FFT C2C basic test failed\n"); return 1; }
    if (!ok2) { fprintf(stderr, "FFT R2C/C2R roundtrip test failed\n"); return 1; }
    if (!ok3) { fprintf(stderr, "fftshift/ifftshift test failed\n"); return 1; }
    // STFT/ISTFT roundtrip (simple)
    {
        // Use compile-time constants to avoid VLA (MSVC compatibility)
        enum { N_STFT = 256, FFT_SZ = 64, HOP_SZ = 32, TAIL = FFT_SZ };
        vv_dsp_real x[N_STFT];
    const double PI = 3.14159265358979323846264338327950288;
    for (size_t i=0;i<N_STFT;++i)
#if defined(VV_DSP_USE_DOUBLE)
        x[i] = (vv_dsp_real)sin(2.0*PI*(double)i/32.0);
#else
        x[i] = (vv_dsp_real)sinf((vv_dsp_real)2.0*(vv_dsp_real)PI*(vv_dsp_real)i/(vv_dsp_real)32.0f);
#endif

        vv_dsp_stft_params p; p.fft_size = FFT_SZ; p.hop_size = HOP_SZ; p.window = VV_DSP_STFT_WIN_HANN;
        vv_dsp_stft* st = NULL;
        if (vv_dsp_stft_create(&p, &st) != VV_DSP_OK) { fprintf(stderr, "stft create failed\n"); return 1; }
        vv_dsp_cpx X[FFT_SZ];
        vv_dsp_real y[N_STFT+TAIL]; memset(y, 0, sizeof(y));
        vv_dsp_real norm[N_STFT+TAIL]; memset(norm, 0, sizeof(norm));
        for (size_t start=0; start + p.fft_size <= N_STFT + (p.fft_size - p.hop_size); start += p.hop_size) {
            vv_dsp_real frame[FFT_SZ];
            for (size_t i=0;i<p.fft_size;++i) {
                size_t idx = start + i;
                frame[i] = (idx < N_STFT) ? x[idx] : (vv_dsp_real)0;
            }
            if (vv_dsp_stft_process(st, frame, X) != VV_DSP_OK) { fprintf(stderr, "stft process failed\n"); vv_dsp_stft_destroy(st); return 1; }
            if (vv_dsp_stft_reconstruct(st, X, y + start, norm + start) != VV_DSP_OK) { fprintf(stderr, "istft failed\n"); vv_dsp_stft_destroy(st); return 1; }
        }
        for (size_t i=0;i<N_STFT+TAIL;++i) if (norm[i] > (vv_dsp_real)1e-12) y[i] /= norm[i];
        // Compare central n samples (ignore start/end transient)
        vv_dsp_real mse = 0;
        size_t count = 0;
        for (size_t i=0;i<N_STFT; ++i) {
            vv_dsp_real d = x[i] - y[i];
            mse += d*d; count++;
        }
        mse /= (vv_dsp_real)count;
        vv_dsp_stft_destroy(st);
        if (!(mse < (vv_dsp_real)1e-2)) { fprintf(stderr, "STFT roundtrip MSE too high: %f\n", (double)mse); return 1; }
    }

    printf("spectral tests passed\n");
    return 0;
}
