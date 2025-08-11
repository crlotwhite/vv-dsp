// Window function implementations
#include "vv_dsp/window.h"

#include <math.h>
#include "vv_dsp/vv_dsp_math.h"
#include <stddef.h>


// Internal helper for common validation
static VV_DSP_INLINE vv_dsp_status vv_dsp__validate_window_args(size_t N, const vv_dsp_real* out) {
    if (!out) return VV_DSP_ERROR_NULL_POINTER;
    if (N == 0) return VV_DSP_ERROR_INVALID_SIZE;
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_window_boxcar(size_t N, vv_dsp_real* out) {
    vv_dsp_status s = vv_dsp__validate_window_args(N, out);
    if (s != VV_DSP_OK) return s;
    for (size_t n = 0; n < N; ++n) {
        out[n] = (vv_dsp_real)1.0;
    }
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_window_hann(size_t N, vv_dsp_real* out) {
    vv_dsp_status s = vv_dsp__validate_window_args(N, out);
    if (s != VV_DSP_OK) return s;
    if (N == 1) { out[0] = (vv_dsp_real)1.0; return VV_DSP_OK; }
    const vv_dsp_real denom = (vv_dsp_real)(N - 1);
    const vv_dsp_real two_pi_over = (vv_dsp_real)(VV_DSP_TWO_PI) / denom;
    for (size_t n = 0; n < N; ++n) {
    vv_dsp_real c = (vv_dsp_real)VV_DSP_COS(two_pi_over * (vv_dsp_real)n);
        out[n] = (vv_dsp_real)0.5 - (vv_dsp_real)0.5 * c;
    }
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_window_hamming(size_t N, vv_dsp_real* out) {
    vv_dsp_status s = vv_dsp__validate_window_args(N, out);
    if (s != VV_DSP_OK) return s;
    if (N == 1) { out[0] = (vv_dsp_real)1.0; return VV_DSP_OK; }
    const vv_dsp_real denom = (vv_dsp_real)(N - 1);
    const vv_dsp_real two_pi_over = (vv_dsp_real)(VV_DSP_TWO_PI) / denom;
    for (size_t n = 0; n < N; ++n) {
    vv_dsp_real c = (vv_dsp_real)VV_DSP_COS(two_pi_over * (vv_dsp_real)n);
        out[n] = (vv_dsp_real)0.54 - (vv_dsp_real)0.46 * c;
    }
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_window_blackman(size_t N, vv_dsp_real* out) {
    vv_dsp_status s = vv_dsp__validate_window_args(N, out);
    if (s != VV_DSP_OK) return s;
    if (N == 1) { out[0] = (vv_dsp_real)1.0; return VV_DSP_OK; }
    const vv_dsp_real denom = (vv_dsp_real)(N - 1);
    const vv_dsp_real two_pi_over = (vv_dsp_real)(VV_DSP_TWO_PI) / denom;
    for (size_t n = 0; n < N; ++n) {
        vv_dsp_real x = two_pi_over * (vv_dsp_real)n;
    vv_dsp_real c1 = (vv_dsp_real)VV_DSP_COS(x);
    vv_dsp_real c2 = (vv_dsp_real)VV_DSP_COS((vv_dsp_real)2.0 * x);
        out[n] = (vv_dsp_real)0.42 - (vv_dsp_real)0.5 * c1 + (vv_dsp_real)0.08 * c2;
    }
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_window_blackman_harris(size_t N, vv_dsp_real* out) {
    vv_dsp_status s = vv_dsp__validate_window_args(N, out);
    if (s != VV_DSP_OK) return s;
    if (N == 1) { out[0] = (vv_dsp_real)1.0; return VV_DSP_OK; }
    const vv_dsp_real a0 = (vv_dsp_real)0.35875;
    const vv_dsp_real a1 = (vv_dsp_real)0.48829;
    const vv_dsp_real a2 = (vv_dsp_real)0.14128;
    const vv_dsp_real a3 = (vv_dsp_real)0.01168;
    const vv_dsp_real denom = (vv_dsp_real)(N - 1);
    const vv_dsp_real two_pi_over = (vv_dsp_real)(VV_DSP_TWO_PI) / denom;
    for (size_t n = 0; n < N; ++n) {
        vv_dsp_real x = two_pi_over * (vv_dsp_real)n;
    vv_dsp_real c1 = (vv_dsp_real)VV_DSP_COS(x);
    vv_dsp_real c2 = (vv_dsp_real)VV_DSP_COS((vv_dsp_real)2.0 * x);
    vv_dsp_real c3 = (vv_dsp_real)VV_DSP_COS((vv_dsp_real)3.0 * x);
        out[n] = a0 - a1 * c1 + a2 * c2 - a3 * c3;
    }
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_window_nuttall(size_t N, vv_dsp_real* out) {
    vv_dsp_status s = vv_dsp__validate_window_args(N, out);
    if (s != VV_DSP_OK) return s;
    if (N == 1) { out[0] = (vv_dsp_real)1.0; return VV_DSP_OK; }
    const vv_dsp_real a0 = (vv_dsp_real)0.3635819;
    const vv_dsp_real a1 = (vv_dsp_real)0.4891775;
    const vv_dsp_real a2 = (vv_dsp_real)0.1365995;
    const vv_dsp_real a3 = (vv_dsp_real)0.0106411;
    const vv_dsp_real denom = (vv_dsp_real)(N - 1);
    const vv_dsp_real two_pi_over = (vv_dsp_real)(VV_DSP_TWO_PI) / denom;
    for (size_t n = 0; n < N; ++n) {
        vv_dsp_real x = two_pi_over * (vv_dsp_real)n;
    vv_dsp_real c1 = (vv_dsp_real)VV_DSP_COS(x);
    vv_dsp_real c2 = (vv_dsp_real)VV_DSP_COS((vv_dsp_real)2.0 * x);
    vv_dsp_real c3 = (vv_dsp_real)VV_DSP_COS((vv_dsp_real)3.0 * x);
        out[n] = a0 - a1 * c1 + a2 * c2 - a3 * c3;
    }
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_window_bartlett(size_t N, vv_dsp_real* out) {
    vv_dsp_status s = vv_dsp__validate_window_args(N, out);
    if (s != VV_DSP_OK) return s;
    if (N == 1) { out[0] = (vv_dsp_real)1.0; return VV_DSP_OK; }
    const vv_dsp_real half_N_minus_1 = (vv_dsp_real)(N - 1) / (vv_dsp_real)2.0;
    for (size_t n = 0; n < N; ++n) {
        vv_dsp_real diff = (vv_dsp_real)n - half_N_minus_1;
        if (diff < 0) diff = -diff; // abs(diff)
        out[n] = (vv_dsp_real)1.0 - diff / half_N_minus_1;
    }
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_window_bohman(size_t N, vv_dsp_real* out) {
    vv_dsp_status s = vv_dsp__validate_window_args(N, out);
    if (s != VV_DSP_OK) return s;
    if (N == 1) { out[0] = (vv_dsp_real)1.0; return VV_DSP_OK; }
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
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_window_cosine(size_t N, vv_dsp_real* out) {
    vv_dsp_status s = vv_dsp__validate_window_args(N, out);
    if (s != VV_DSP_OK) return s;
    if (N == 1) { out[0] = (vv_dsp_real)1.0; return VV_DSP_OK; }
    const vv_dsp_real denom = (vv_dsp_real)(N - 1);
    const vv_dsp_real pi_over = VV_DSP_PI / denom;
    for (size_t n = 0; n < N; ++n) {
        out[n] = (vv_dsp_real)VV_DSP_SIN(pi_over * (vv_dsp_real)n);
    }
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_window_planck_taper(size_t N, vv_dsp_real* out) {
    vv_dsp_status s = vv_dsp__validate_window_args(N, out);
    if (s != VV_DSP_OK) return s;
    if (N == 1) { out[0] = (vv_dsp_real)1.0; return VV_DSP_OK; }
    
    // Default epsilon = 0.1 (10% taper)
    const vv_dsp_real epsilon = (vv_dsp_real)0.1;
    const vv_dsp_real N_real = (vv_dsp_real)N;
    const vv_dsp_real taper_width = epsilon * N_real / (vv_dsp_real)2.0;
    
    for (size_t n = 0; n < N; ++n) {
        vv_dsp_real n_real = (vv_dsp_real)n;
        if (n_real < taper_width) {
            // Left taper
            vv_dsp_real x = (vv_dsp_real)2.0 * epsilon * (n_real / N_real - epsilon / (vv_dsp_real)2.0);
            if (x != (vv_dsp_real)0.0) {
                out[n] = (vv_dsp_real)1.0 / ((vv_dsp_real)1.0 + (vv_dsp_real)VV_DSP_EXP((vv_dsp_real)2.0 * epsilon / x - (vv_dsp_real)2.0 * epsilon / (epsilon - x)));
            } else {
                out[n] = (vv_dsp_real)0.0;
            }
        } else if (n_real >= N_real - taper_width) {
            // Right taper
            vv_dsp_real x = (vv_dsp_real)2.0 * epsilon * ((N_real - (vv_dsp_real)1.0 - n_real) / N_real - epsilon / (vv_dsp_real)2.0);
            if (x != (vv_dsp_real)0.0) {
                out[n] = (vv_dsp_real)1.0 / ((vv_dsp_real)1.0 + (vv_dsp_real)VV_DSP_EXP((vv_dsp_real)2.0 * epsilon / x - (vv_dsp_real)2.0 * epsilon / (epsilon - x)));
            } else {
                out[n] = (vv_dsp_real)0.0;
            }
        } else {
            // Middle section
            out[n] = (vv_dsp_real)1.0;
        }
    }
    return VV_DSP_OK;
}

// Helper function for Kaiser window: Modified Bessel function of the first kind, order 0
static vv_dsp_real vv_dsp__bessel_i0(vv_dsp_real x) {
    // Approximation for I_0(x) using series expansion
    vv_dsp_real result = (vv_dsp_real)1.0;
    vv_dsp_real term = (vv_dsp_real)1.0;
    vv_dsp_real x_squared = x * x / (vv_dsp_real)4.0;
    
    for (int n = 1; n <= 20; ++n) { // 20 terms should be sufficient for most cases
        term *= x_squared / ((vv_dsp_real)n * (vv_dsp_real)n);
        result += term;
        if (term < (vv_dsp_real)1e-12) break; // Early termination for convergence
    }
    return result;
}

vv_dsp_status vv_dsp_window_flattop(size_t N, vv_dsp_real* out) {
    vv_dsp_status s = vv_dsp__validate_window_args(N, out);
    if (s != VV_DSP_OK) return s;
    if (N == 1) { out[0] = (vv_dsp_real)1.0; return VV_DSP_OK; }
    
    // Flattop window coefficients
    const vv_dsp_real a0 = (vv_dsp_real)0.21557895;
    const vv_dsp_real a1 = (vv_dsp_real)0.41663158;
    const vv_dsp_real a2 = (vv_dsp_real)0.277263158;
    const vv_dsp_real a3 = (vv_dsp_real)0.083578947;
    const vv_dsp_real a4 = (vv_dsp_real)0.006947368;
    
    const vv_dsp_real denom = (vv_dsp_real)(N - 1);
    const vv_dsp_real two_pi_over = VV_DSP_TWO_PI / denom;
    
    for (size_t n = 0; n < N; ++n) {
        vv_dsp_real x = two_pi_over * (vv_dsp_real)n;
        vv_dsp_real c1 = (vv_dsp_real)VV_DSP_COS(x);
        vv_dsp_real c2 = (vv_dsp_real)VV_DSP_COS((vv_dsp_real)2.0 * x);
        vv_dsp_real c3 = (vv_dsp_real)VV_DSP_COS((vv_dsp_real)3.0 * x);
        vv_dsp_real c4 = (vv_dsp_real)VV_DSP_COS((vv_dsp_real)4.0 * x);
        out[n] = a0 - a1 * c1 + a2 * c2 - a3 * c3 + a4 * c4;
    }
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_window_kaiser(size_t N, vv_dsp_real beta, vv_dsp_real* out) {
    vv_dsp_status s = vv_dsp__validate_window_args(N, out);
    if (s != VV_DSP_OK) return s;
    if (N == 1) { out[0] = (vv_dsp_real)1.0; return VV_DSP_OK; }
    
    const vv_dsp_real bessel_beta = vv_dsp__bessel_i0(beta);
    const vv_dsp_real half_N_minus_1 = (vv_dsp_real)(N - 1) / (vv_dsp_real)2.0;
    
    for (size_t n = 0; n < N; ++n) {
        vv_dsp_real alpha = ((vv_dsp_real)n - half_N_minus_1) / half_N_minus_1;
        vv_dsp_real sqrt_term = (vv_dsp_real)1.0 - alpha * alpha;
        if (sqrt_term >= (vv_dsp_real)0.0) {
            vv_dsp_real sqrt_val = (vv_dsp_real)VV_DSP_SQRT(sqrt_term);
            vv_dsp_real bessel_arg = beta * sqrt_val;
            out[n] = vv_dsp__bessel_i0(bessel_arg) / bessel_beta;
        } else {
            out[n] = (vv_dsp_real)0.0;
        }
    }
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_window_tukey(size_t N, vv_dsp_real alpha, vv_dsp_real* out) {
    vv_dsp_status s = vv_dsp__validate_window_args(N, out);
    if (s != VV_DSP_OK) return s;
    if (N == 1) { out[0] = (vv_dsp_real)1.0; return VV_DSP_OK; }
    
    // Clamp alpha to [0, 1]
    if (alpha < (vv_dsp_real)0.0) alpha = (vv_dsp_real)0.0;
    if (alpha > (vv_dsp_real)1.0) alpha = (vv_dsp_real)1.0;
    
    const vv_dsp_real N_real = (vv_dsp_real)N;
    const vv_dsp_real taper_width = alpha * (N_real - (vv_dsp_real)1.0) / (vv_dsp_real)2.0;
    
    for (size_t n = 0; n < N; ++n) {
        vv_dsp_real n_real = (vv_dsp_real)n;
        
        if (n_real < taper_width) {
            // Left taper (Hann-like)
            vv_dsp_real cos_arg = VV_DSP_PI * n_real / taper_width;
            out[n] = (vv_dsp_real)0.5 * ((vv_dsp_real)1.0 - (vv_dsp_real)VV_DSP_COS(cos_arg));
        } else if (n_real >= N_real - taper_width) {
            // Right taper (Hann-like)
            vv_dsp_real cos_arg = VV_DSP_PI * (N_real - (vv_dsp_real)1.0 - n_real) / taper_width;
            out[n] = (vv_dsp_real)0.5 * ((vv_dsp_real)1.0 - (vv_dsp_real)VV_DSP_COS(cos_arg));
        } else {
            // Middle section (constant)
            out[n] = (vv_dsp_real)1.0;
        }
    }
    return VV_DSP_OK;
}
