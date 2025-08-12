#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "vv_dsp/vv_dsp_math.h"
#include "vv_dsp/vv_dsp_types.h"
#include "vv_dsp/spectral/dct.h"
#include "vv_dsp/core/nan_policy.h"

struct vv_dsp_dct_plan {
    size_t n;
    vv_dsp_dct_type type;
    vv_dsp_dct_dir dir;
};

static VV_DSP_INLINE vv_dsp_real vv_pi(void) { return VV_DSP_PI; }

// Naive O(N^2) DCT implementations (reference-correct, SIMD-friendly loops)
// Canonical unnormalized pair:
// DCT-II (forward): X[k] = sum_{n=0}^{N-1} x[n] * cos(pi/N * (n+0.5) * k)
// Inverse (DCT-III): x[n] = (2/N) * [ 0.5*X[0] + sum_{k=1}^{N-1} X[k] * cos(pi/N * k * (n+0.5)) ]
static void dct2_forward(const vv_dsp_real* x, vv_dsp_real* X, size_t N) {
    for (size_t k = 0; k < N; ++k) {
        vv_dsp_real sum = 0;
        for (size_t n = 0; n < N; ++n) {
            vv_dsp_real ang = vv_pi() * ((vv_dsp_real)n + (vv_dsp_real)0.5) * (vv_dsp_real)k / (vv_dsp_real)N;
            sum += x[n] * VV_DSP_COS(ang);
        }
        X[k] = sum;
    }
}

static void dct3_inverse_from_ii(const vv_dsp_real* X, vv_dsp_real* x, size_t N) {
    const vv_dsp_real scale = (vv_dsp_real)2.0 / (vv_dsp_real)N;
    for (size_t n = 0; n < N; ++n) {
        vv_dsp_real sum = (vv_dsp_real)0.5 * X[0];
        for (size_t k = 1; k < N; ++k) {
            vv_dsp_real ang = vv_pi() * (vv_dsp_real)k * ((vv_dsp_real)n + (vv_dsp_real)0.5) / (vv_dsp_real)N;
            sum += X[k] * VV_DSP_COS(ang);
        }
        x[n] = scale * sum;
    }
}

// DCT-III forward (unnormalized):
// Y[k] = x[0] + 2 * sum_{n=1}^{N-1} x[n] * cos(pi/N * k * (n+0.5))
static void dct3_forward(const vv_dsp_real* x, vv_dsp_real* Y, size_t N) {
    for (size_t k = 0; k < N; ++k) {
        vv_dsp_real sum = x[0];
        for (size_t n = 1; n < N; ++n) {
            vv_dsp_real ang = vv_pi() * (vv_dsp_real)k * ((vv_dsp_real)n + (vv_dsp_real)0.5) / (vv_dsp_real)N;
            sum += (vv_dsp_real)2.0 * x[n] * VV_DSP_COS(ang);
        }
        Y[k] = sum;
    }
}

static void dct4_transform(const vv_dsp_real* x, vv_dsp_real* X, size_t N, int inverse) {
    // DCT-IV is its own inverse up to a scale of 2/N
    for (size_t k = 0; k < N; ++k) {
        vv_dsp_real sum = 0;
        for (size_t n = 0; n < N; ++n) {
            vv_dsp_real ang = vv_pi() * ((vv_dsp_real)n + (vv_dsp_real)0.5) * ((vv_dsp_real)k + (vv_dsp_real)0.5) / (vv_dsp_real)N;
            sum += x[n] * VV_DSP_COS(ang);
        }
        if (inverse) sum *= (vv_dsp_real)2.0 / (vv_dsp_real)N;
        X[k] = sum;
    }
}

VV_DSP_NODISCARD vv_dsp_status vv_dsp_dct_make_plan(size_t n,
                                                    vv_dsp_dct_type type,
                                                    vv_dsp_dct_dir dir,
                                                    vv_dsp_dct_plan** out_plan) {
    if (!out_plan) return VV_DSP_ERROR_NULL_POINTER;
    *out_plan = NULL;
    if (n == 0) return VV_DSP_ERROR_INVALID_SIZE;
    if (!(type == VV_DSP_DCT_II || type == VV_DSP_DCT_III || type == VV_DSP_DCT_IV)) return VV_DSP_ERROR_OUT_OF_RANGE;
    if (!(dir == VV_DSP_DCT_FORWARD || dir == VV_DSP_DCT_BACKWARD)) return VV_DSP_ERROR_OUT_OF_RANGE;
    vv_dsp_dct_plan* p = (vv_dsp_dct_plan*)malloc(sizeof(vv_dsp_dct_plan));
    if (!p) return VV_DSP_ERROR_INTERNAL;
    p->n = n; p->type = type; p->dir = dir;
    *out_plan = p;
    return VV_DSP_OK;
}

VV_DSP_NODISCARD vv_dsp_status vv_dsp_dct_execute(const vv_dsp_dct_plan* plan,
                                                  const vv_dsp_real* in,
                                                  vv_dsp_real* out) {
    if (!plan || !in || !out) return VV_DSP_ERROR_NULL_POINTER;
    
    const size_t N = plan->n;
    
    // Allocate temporary buffer for NaN/Inf processed input
    vv_dsp_real* temp_input = (vv_dsp_real*)malloc(N * sizeof(vv_dsp_real));
    if (!temp_input) return VV_DSP_ERROR_INTERNAL;
    
    // Check and handle NaN/Inf in input according to policy
    vv_dsp_status status = vv_dsp_apply_nan_policy_copy(in, N, temp_input);
    if (status != VV_DSP_OK) {
        free(temp_input);
        return status;  // Return early on error (e.g., if policy is ERROR and NaN/Inf found)
    }
    
    // Apply the appropriate DCT transform
    if (plan->type == VV_DSP_DCT_II) {
        if (plan->dir == VV_DSP_DCT_FORWARD) {
            dct2_forward(temp_input, out, N);
        } else {
            // inverse of DCT-II is DCT-III with 2/N scaling and half-weight for k=0 term
            dct3_inverse_from_ii(temp_input, out, N);
        }
    } else if (plan->type == VV_DSP_DCT_III) {
        if (plan->dir == VV_DSP_DCT_FORWARD) {
            // Compute DCT-III directly
            dct3_forward(temp_input, out, N);
        } else {
            // inverse of DCT-III recovers time via same formula as dct3_inverse_from_ii
            dct3_inverse_from_ii(temp_input, out, N);
        }
    } else if (plan->type == VV_DSP_DCT_IV) {
        dct4_transform(temp_input, out, N, plan->dir == VV_DSP_DCT_BACKWARD);
    } else {
        free(temp_input);
        return VV_DSP_ERROR_OUT_OF_RANGE;
    }
    
    free(temp_input);
    
    // Apply NaN/Inf policy to output
    status = vv_dsp_apply_nan_policy_inplace(out, N);
    if (status != VV_DSP_OK) {
        return status;
    }
    
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_dct_destroy(vv_dsp_dct_plan* plan) {
    if (!plan) return VV_DSP_ERROR_NULL_POINTER;
    free(plan);
    return VV_DSP_OK;
}

VV_DSP_NODISCARD vv_dsp_status vv_dsp_dct_forward(size_t n,
                                                  vv_dsp_dct_type type,
                                                  const vv_dsp_real* in,
                                                  vv_dsp_real* out) {
    vv_dsp_dct_plan* p = NULL; vv_dsp_status s = vv_dsp_dct_make_plan(n, type, VV_DSP_DCT_FORWARD, &p);
    if (s != VV_DSP_OK) return s;
    s = vv_dsp_dct_execute(p, in, out);
    vv_dsp_dct_destroy(p);
    return s;
}

VV_DSP_NODISCARD vv_dsp_status vv_dsp_dct_inverse(size_t n,
                                                  vv_dsp_dct_type type,
                                                  const vv_dsp_real* in,
                                                  vv_dsp_real* out) {
    vv_dsp_dct_plan* p = NULL; vv_dsp_status s = vv_dsp_dct_make_plan(n, type, VV_DSP_DCT_BACKWARD, &p);
    if (s != VV_DSP_OK) return s;
    s = vv_dsp_dct_execute(p, in, out);
    vv_dsp_dct_destroy(p);
    return s;
}
