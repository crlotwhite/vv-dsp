#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "vv_dsp/spectral/fft.h"
#include "vv_dsp/spectral/utils.h"
#include "vv_dsp/spectral/hilbert.h"

// Helper: allocate zeroed memory for complex array
static vv_dsp_cpx* cpx_calloc(size_t n) {
    vv_dsp_cpx* p = (vv_dsp_cpx*)calloc(n, sizeof(vv_dsp_cpx));
    return p;
}

vv_dsp_status vv_dsp_hilbert_analytic(const vv_dsp_real* input, size_t N, vv_dsp_cpx* analytic_output) {
    if (!input || !analytic_output) return VV_DSP_ERROR_NULL_POINTER;
    if (N == 0) return VV_DSP_ERROR_INVALID_SIZE;

    vv_dsp_status st;

    // Step 1: R2C FFT
    vv_dsp_fft_plan* plan_r2c = NULL;
    st = vv_dsp_fft_make_plan(N, VV_DSP_FFT_R2C, VV_DSP_FFT_FORWARD, &plan_r2c);
    if (st != VV_DSP_OK || !plan_r2c) return (st == VV_DSP_OK ? VV_DSP_ERROR_INTERNAL : st);

    size_t Nh = N/2 + 1; // Hermitian packed bins
    vv_dsp_cpx* Xh = (vv_dsp_cpx*)malloc(Nh * sizeof(vv_dsp_cpx));
    if (!Xh) { vv_dsp_fft_destroy(plan_r2c); return VV_DSP_ERROR_INTERNAL; }
    st = vv_dsp_fft_execute(plan_r2c, input, Xh);
    if (st != VV_DSP_OK) { free(Xh); vv_dsp_fft_destroy(plan_r2c); return st; }

    // Step 2: Build full spectrum Xfull from Hermitian-packed Xh
    vv_dsp_cpx* Xfull = cpx_calloc(N);
    if (!Xfull) { free(Xh); vv_dsp_fft_destroy(plan_r2c); return VV_DSP_ERROR_INTERNAL; }
    // k=0..Nh-1 copy
    for (size_t k=0;k<Nh;++k) Xfull[k] = Xh[k];
    // k=Nh..N-1 from conjugate symmetry
    for (size_t k=Nh; k<N; ++k) {
        size_t m = N - k; // mirror index in 0..Nh-1
        Xfull[k].re = Xh[m].re;
        Xfull[k].im = (vv_dsp_real)(-Xh[m].im);
    }

    // Step 3: Apply Hilbert filter H to Xfull to obtain analytic spectrum Z
    vv_dsp_cpx* Z = cpx_calloc(N);
    if (!Z) { free(Xh); free(Xfull); vv_dsp_fft_destroy(plan_r2c); return VV_DSP_ERROR_INTERNAL; }
    if ((N%2)==0) {
        // even N: k=0 and k=N/2 pass with 1, k=1..N/2-1 *2, negatives zero
        if (N >= 1) Z[0] = Xfull[0];
        for (size_t k=1;k<(size_t)(N/2);++k) { Z[k].re = (vv_dsp_real)2.0 * Xfull[k].re; Z[k].im = (vv_dsp_real)2.0 * Xfull[k].im; }
        Z[N/2] = Xfull[N/2];
        // k>N/2 remain zero
    } else {
        // odd N: k=0 pass, k=1..(N-1)/2 *2, negatives zero
        Z[0] = Xfull[0];
        size_t kmax = (N-1)/2 + 1; // equals Nh
        for (size_t k=1;k<kmax;++k) { Z[k].re = (vv_dsp_real)2.0 * Xfull[k].re; Z[k].im = (vv_dsp_real)2.0 * Xfull[k].im; }
        // rest zero
    }

    // Step 4: IFFT C2C to get analytic signal directly
    vv_dsp_fft_plan* plan_c2c_inv = NULL;
    st = vv_dsp_fft_make_plan(N, VV_DSP_FFT_C2C, VV_DSP_FFT_BACKWARD, &plan_c2c_inv);
    if (st != VV_DSP_OK || !plan_c2c_inv) { free(Xh); free(Xfull); free(Z); vv_dsp_fft_destroy(plan_r2c); return (st == VV_DSP_OK ? VV_DSP_ERROR_INTERNAL : st); }
    vv_dsp_cpx* z_time = (vv_dsp_cpx*)malloc(N * sizeof(vv_dsp_cpx));
    if (!z_time) { free(Xh); free(Xfull); free(Z); vv_dsp_fft_destroy(plan_r2c); vv_dsp_fft_destroy(plan_c2c_inv); return VV_DSP_ERROR_INTERNAL; }
    st = vv_dsp_fft_execute(plan_c2c_inv, Z, z_time);
    if (st != VV_DSP_OK) { free(Xh); free(Xfull); free(Z); free(z_time); vv_dsp_fft_destroy(plan_r2c); vv_dsp_fft_destroy(plan_c2c_inv); return st; }

    for (size_t i=0;i<N;++i) analytic_output[i] = z_time[i];

    free(Xh); free(Xfull); free(Z); free(z_time);
    vv_dsp_fft_destroy(plan_r2c);
    vv_dsp_fft_destroy(plan_c2c_inv);
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_instantaneous_phase(const vv_dsp_cpx* analytic_input, size_t N, vv_dsp_real* phase_output) {
    if (!analytic_input || !phase_output) return VV_DSP_ERROR_NULL_POINTER;
    if (N == 0) return VV_DSP_ERROR_INVALID_SIZE;
    // Principal phase at 0
    double phi0 = atan2((double)analytic_input[0].im, (double)analytic_input[0].re);
    phase_output[0] = (vv_dsp_real)phi0;
    // Integrate phase increments via conjugate product to ensure continuity
    double acc = phi0;
    for (size_t i=1;i<N;++i) {
        double re = (double)analytic_input[i].re * (double)analytic_input[i-1].re + (double)analytic_input[i].im * (double)analytic_input[i-1].im; // Re(z_i * conj(z_{i-1}))
        double im = (double)analytic_input[i].im * (double)analytic_input[i-1].re - (double)analytic_input[i].re * (double)analytic_input[i-1].im; // Im(z_i * conj(z_{i-1}))
        double dphi = atan2(im, re); // in (-pi, pi]
        acc += dphi;
        phase_output[i] = (vv_dsp_real)acc;
    }
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_instantaneous_frequency(const vv_dsp_real* unwrapped_phase_input,
                                             size_t N,
                                             double sample_rate,
                                             vv_dsp_real* freq_output) {
    if (!unwrapped_phase_input || !freq_output) return VV_DSP_ERROR_NULL_POINTER;
    if (N == 0) return VV_DSP_ERROR_INVALID_SIZE;
    if (N == 1) {
        freq_output[0] = (vv_dsp_real)0;
        return VV_DSP_OK;
    }
    freq_output[0] = (vv_dsp_real)0;
    const double scale = sample_rate / (2.0 * VV_DSP_PI_D);
    for (size_t i=1;i<N;++i) {
        double dphi = (double)unwrapped_phase_input[i] - (double)unwrapped_phase_input[i-1];
        double hz = dphi * scale;
        freq_output[i] = (vv_dsp_real)hz;
    }
    return VV_DSP_OK;
}
