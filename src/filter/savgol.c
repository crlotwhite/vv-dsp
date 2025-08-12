// Savitzky–Golay filter implementation

#include <stddef.h>
#include <math.h>
#include <stdlib.h>

#include "vv_dsp/vv_dsp_types.h"
#include "vv_dsp/filter/savgol.h"
#include "vv_dsp/core/nan_policy.h"

static VV_DSP_INLINE int is_odd(int x) { return (x & 1) != 0; }

// Compute factorial-like product for binomial coefficients safely in double
static double fact_ratio(int n, int k) {
    // n! / k! where n>=k>=0
    if (k > n) return 0.0;
    double r = 1.0;
    for (int i = k + 1; i <= n; ++i) r *= (double)i;
    return r;
}

// Compute Legendre polynomial via recurrence (for smoothing filter weights only)
// Not used directly; using direct Savitzky-Golay closed form.

// Closed-form Savitzky–Golay smoothing coefficients (deriv==0) using Gram polynomials
// From: G. Savitzky and M. J. E. Golay (1964) and Orfanidis notes.
// This function fills coeffs of length m = window_length with symmetric weights summing to 1.
static vv_dsp_status sg_smoothing_kernel(int window_length, int polyorder, vv_dsp_real* coeffs_out) {
    int m = window_length;
    int half = m / 2;
    // We'll compute using normal equations A^T A inverse method (general approach valid for any deriv)
    // Build Vandermonde matrix A of size m x (p+1) with centered indices t in [-half..half]
    int p = polyorder;
    int cols = p + 1;

    // Allocate small matrices on stack for modest sizes (window lengths are typically small, e.g., <= 51)
    double ATA[16*16]; // supports up to p<=15
    double ATY[16];
    if (cols > 16) return VV_DSP_ERROR_OUT_OF_RANGE;

    // Zero initialize
    for (int i = 0; i < cols*cols; ++i) ATA[i] = 0.0;
    for (int i = 0; i < cols; ++i) ATY[i] = 0.0;

    // y is smoothing: we want convolution coefficients h such that output[0] = sum h[k]*y[k]
    // In LS sense, h corresponds to first row of projection for evaluating polynomial at t=0
    // Set up normal equations for polynomial fit; then take c solving (A^T A) c = A^T y, with y being unit vector at center.

    // Build A and accumulate ATA, and ATy for y being delta at center position (k==half)
    for (int row = 0; row < m; ++row) {
        int t = row - half; // centered index
        // Compute powers t^j
        double pow_t[16];
        pow_t[0] = 1.0;
        for (int j = 1; j < cols; ++j) pow_t[j] = pow_t[j-1] * (double)t;
        // ATA += pow_t^T * pow_t
        for (int i = 0; i < cols; ++i) {
            for (int j = 0; j < cols; ++j) {
                ATA[i*cols + j] += pow_t[i] * pow_t[j];
            }
        }
        // y is 1 at center, 0 elsewhere: ATY += pow_t * y[row]
        double yrow = (row == half) ? 1.0 : 0.0;
        for (int i = 0; i < cols; ++i) ATY[i] += pow_t[i] * yrow;
    }

    // Solve (ATA) c = ATY using Gaussian elimination (small system)
    // Augmented matrix [ATA | ATY]
    double M[16*17];
    for (int i = 0; i < cols; ++i) {
        for (int j = 0; j < cols; ++j) M[i* (cols+1) + j] = ATA[i*cols + j];
        M[i* (cols+1) + cols] = ATY[i];
    }

    // Gaussian elimination with partial pivoting
    for (int k = 0; k < cols; ++k) {
        // pivot
        int piv = k; double pivval = fabs(M[k*(cols+1)+k]);
        for (int r = k+1; r < cols; ++r) {
            double v = fabs(M[r*(cols+1)+k]); if (v > pivval) { piv = r; pivval = v; }
        }
        if (pivval == 0.0) return VV_DSP_ERROR_INTERNAL;
        if (piv != k) {
            for (int j = k; j <= cols; ++j) {
                double tmp = M[k*(cols+1)+j]; M[k*(cols+1)+j] = M[piv*(cols+1)+j]; M[piv*(cols+1)+j] = tmp;
            }
        }
        // normalize row k
        double diag = M[k*(cols+1)+k];
        for (int j = k; j <= cols; ++j) M[k*(cols+1)+j] /= diag;
        // eliminate other rows
        for (int r = 0; r < cols; ++r) if (r != k) {
            double f = M[r*(cols+1)+k];
            for (int j = k; j <= cols; ++j) M[r*(cols+1)+j] -= f * M[k*(cols+1)+j];
        }
    }
    double c[16];
    for (int i = 0; i < cols; ++i) c[i] = M[i*(cols+1)+cols];

    // Convolution kernel h[k] = row vector e0^T * (A^T A)^{-1} A^T evaluated at sample k
    // We'll compute weights via solving (A^T A) X = A^T per sample and taking the first row of X.

    // Recompute properly: Solve (ATA) X = AT, where AT is cols x m matrix of A^T
    // Build A^T (cols x m)
    // Note: For m up to ~101 and p<=10 typical, stack arrays could be large. We'll bound by p<=15 and m<=129 to be safe but keep simple implementation for now.
    // For simplicity and robustness, implement minimal solver that for each sample r solves (ATA) w_col = a_col, where a_col is column vector of A^T column r.
    for (int r = 0; r < m; ++r) {
        int t = r - half;
        double a_col[16];
        a_col[0] = 1.0; for (int j = 1; j < cols; ++j) a_col[j] = a_col[j-1] * (double)t;
        // Copy augmented matrix
        double MM[16*17];
        for (int i = 0; i < cols; ++i) {
            for (int j = 0; j < cols; ++j) MM[i*(cols+1)+j] = ATA[i*cols + j];
            MM[i*(cols+1)+cols] = a_col[i];
        }
        // Solve (ATA) x = a_col
        for (int k = 0; k < cols; ++k) {
            int piv = k; double pivval = fabs(MM[k*(cols+1)+k]);
            for (int rr = k+1; rr < cols; ++rr) { double v = fabs(MM[rr*(cols+1)+k]); if (v > pivval) { piv = rr; pivval = v; } }
            if (pivval == 0.0) return VV_DSP_ERROR_INTERNAL;
            if (piv != k) {
                for (int j = k; j <= cols; ++j) { double tmp = MM[k*(cols+1)+j]; MM[k*(cols+1)+j] = MM[piv*(cols+1)+j]; MM[piv*(cols+1)+j] = tmp; }
            }
            double diag = MM[k*(cols+1)+k];
            for (int j = k; j <= cols; ++j) MM[k*(cols+1)+j] /= diag;
            for (int rr = 0; rr < cols; ++rr) if (rr != k) {
                double f = MM[rr*(cols+1)+k];
                for (int j = k; j <= cols; ++j) MM[rr*(cols+1)+j] -= f * MM[k*(cols+1)+j];
            }
        }
        double xcol[16];
        for (int i = 0; i < cols; ++i) xcol[i] = MM[i*(cols+1)+cols];
        // weight for sample r when evaluating at t=0 is e0^T xcol = xcol[0]
        coeffs_out[r] = (vv_dsp_real)xcol[0];
    }

    // Normalize to sum to 1 (numerical safeguard)
    double s = 0.0; for (int r = 0; r < m; ++r) s += (double)coeffs_out[r];
    if (s != 0.0) { double invs = 1.0 / s; for (int r = 0; r < m; ++r) coeffs_out[r] = (vv_dsp_real)((double)coeffs_out[r] * invs); }
    return VV_DSP_OK;
}

// General derivative kernel: compute coefficients that estimate the deriv-th derivative at center
static vv_dsp_status sg_derivative_kernel(int window_length, int polyorder, int deriv, vv_dsp_real delta, vv_dsp_real* coeffs_out) {
    int m = window_length, half = m/2, p = polyorder, cols = p+1;
    if (cols > 16) return VV_DSP_ERROR_OUT_OF_RANGE;
    double ATA[16*16]; for (int i=0;i<cols*cols;++i) ATA[i]=0.0;
    // Build ATA
    for (int r=0;r<m;++r){ int t=r-half; double pow_t[16]; pow_t[0]=1.0; for(int j=1;j<cols;++j) pow_t[j]=pow_t[j-1]*(double)t; for(int i=0;i<cols;++i) for(int j=0;j<cols;++j) ATA[i*cols+j]+=pow_t[i]*pow_t[j]; }
    // Target vector for derivative: e_deriv scaled by deriv! and delta^deriv
    double b[16]; for (int i=0;i<cols;++i) b[i]=0.0; b[deriv] = fact_ratio(deriv, 0); // deriv!
    // Solve (ATA) c = b for c (coeffs of evaluation functional)
    double M[16*17]; for (int i=0;i<cols;++i){ for(int j=0;j<cols;++j) M[i*(cols+1)+j]=ATA[i*cols+j]; M[i*(cols+1)+cols]=b[i]; }
    for (int k=0;k<cols;++k){ int piv=k; double pivv=fabs(M[k*(cols+1)+k]); for(int r=k+1;r<cols;++r){ double v=fabs(M[r*(cols+1)+k]); if(v>pivv){piv=r;pivv=v;} } if(pivv==0.0) return VV_DSP_ERROR_INTERNAL; if(piv!=k){ for(int j=k;j<=cols;++j){ double tmp=M[k*(cols+1)+j]; M[k*(cols+1)+j]=M[piv*(cols+1)+j]; M[piv*(cols+1)+j]=tmp; }} double diag=M[k*(cols+1)+k]; for(int j=k;j<=cols;++j) M[k*(cols+1)+j]/=diag; for(int r=0;r<cols;++r) if(r!=k){ double f=M[r*(cols+1)+k]; for(int j=k;j<=cols;++j) M[r*(cols+1)+j]-=f*M[k*(cols+1)+j]; } }
    double c[16]; for(int i=0;i<cols;++i) c[i]=M[i*(cols+1)+cols];
    // Now weights w_r = a_r^T c where a_r = [1, t, t^2, ...]^T
    for (int r=0;r<m;++r){ int t=r-half; double tp=1.0; double wr=0.0; for(int j=0;j<cols;++j){ if(j==0) tp=1.0; else tp*= (double)t; wr += c[j]*tp; } coeffs_out[r]=(vv_dsp_real)wr; }
    // Apply delta scaling: derivative estimate should divide by delta^deriv
    if (deriv>0) { double s = pow((double)delta, (double)deriv); if (s==0.0) return VV_DSP_ERROR_OUT_OF_RANGE; double inv = 1.0/s; for(int r=0;r<m;++r) coeffs_out[r]=(vv_dsp_real)((double)coeffs_out[r]*inv); }
    return VV_DSP_OK;
}

static void pad_signal(const vv_dsp_real* x, size_t N, int pad, vv_dsp_savgol_mode mode, vv_dsp_real* xp) {
    // xp length = N + 2*pad, indices [0..pad-1] left pad, [pad .. pad+N-1] data, [pad+N .. pad+N+pad-1] right pad
    size_t L = (size_t)pad;
    // center copy
    for (size_t i=0;i<N;++i) xp[L+i]=x[i];
    // left
    for (int i=0;i<pad;++i) {
        size_t src_idx = 0;
        switch (mode) {
            case VV_DSP_SAVGOL_MODE_REFLECT:
                src_idx = (size_t)(i+1); // reflect about first sample: x[1], x[2], ...
                if (src_idx >= N) src_idx = N-1; // clamp if tiny N
                xp[(size_t)(L-1 - (size_t)i)] = x[src_idx];
                break;
            case VV_DSP_SAVGOL_MODE_CONSTANT:
            case VV_DSP_SAVGOL_MODE_NEAREST:
                xp[(size_t)(L-1 - (size_t)i)] = x[0];
                break;
            case VV_DSP_SAVGOL_MODE_WRAP:
                xp[(size_t)(L-1 - (size_t)i)] = x[(N - ((size_t)i % N) - 1) % N];
                break;
        }
    }
    // right
    for (int i=0;i<pad;++i) {
        size_t src_idx = N-2 - (size_t)i;
        switch (mode) {
            case VV_DSP_SAVGOL_MODE_REFLECT:
                if (N>=2) xp[L+N + (size_t)i] = x[src_idx]; else xp[L+N+(size_t)i]=x[N-1];
                break;
            case VV_DSP_SAVGOL_MODE_CONSTANT:
            case VV_DSP_SAVGOL_MODE_NEAREST:
                xp[L+N + (size_t)i] = x[N-1];
                break;
            case VV_DSP_SAVGOL_MODE_WRAP:
                xp[L+N + (size_t)i] = x[((size_t)i) % N];
                break;
        }
    }
}

static void convolve_valid(const vv_dsp_real* x, size_t N, const vv_dsp_real* h, int m, vv_dsp_real* y) {
    int half = m/2;
    size_t padN = N + 2*(size_t)half;
    // x is already padded in our use; here assume x points to padded buffer, and N is original N
    const vv_dsp_real* xp = x; (void)padN;
    for (size_t n=0;n<N;++n) {
        double acc = 0.0;
        for (int k=0;k<m;++k) {
            acc += (double)h[k] * (double)xp[n + (size_t)k];
        }
        y[n] = (vv_dsp_real)acc;
    }
}

vv_dsp_status vv_dsp_savgol(const vv_dsp_real* y,
                            size_t N,
                            int window_length,
                            int polyorder,
                            int deriv,
                            vv_dsp_real delta,
                            vv_dsp_savgol_mode mode,
                            vv_dsp_real* output)
{
    if (!y || !output) return VV_DSP_ERROR_NULL_POINTER;
    if (N == 0) return VV_DSP_ERROR_INVALID_SIZE;
    if (window_length <= 0 || !is_odd(window_length)) return VV_DSP_ERROR_OUT_OF_RANGE;
    if (polyorder < 0) return VV_DSP_ERROR_OUT_OF_RANGE;
    if (deriv < 0) return VV_DSP_ERROR_OUT_OF_RANGE;
    if (deriv > polyorder) return VV_DSP_ERROR_OUT_OF_RANGE;
    if ((size_t)window_length > N) return VV_DSP_ERROR_INVALID_SIZE;
    if (deriv > 0 && !(delta > (vv_dsp_real)0)) return VV_DSP_ERROR_OUT_OF_RANGE;

    // Apply NaN/Inf policy to input data
    // We need to create a copy of the input for policy processing since the input is const
    vv_dsp_real* y_processed = (vv_dsp_real*)malloc(N * sizeof(vv_dsp_real));
    if (!y_processed) return VV_DSP_ERROR_INTERNAL;

    // Apply NaN/Inf policy using the copy variant
    vv_dsp_status policy_status = vv_dsp_apply_nan_policy_copy(y, N, y_processed);
    if (policy_status != VV_DSP_OK) {
        free(y_processed);
        return policy_status;
    }

    int m = window_length;
    int half = m/2;
    vv_dsp_real kernel_stack[257]; // supports up to m<=257; can extend if needed
    if (m > (int)(sizeof(kernel_stack)/sizeof(kernel_stack[0]))) {
        free(y_processed);
        return VV_DSP_ERROR_OUT_OF_RANGE;
    }
    vv_dsp_status st;
    if (deriv == 0) st = sg_smoothing_kernel(m, polyorder, kernel_stack);
    else st = sg_derivative_kernel(m, polyorder, deriv, delta, kernel_stack);
    if (st != VV_DSP_OK) {
        free(y_processed);
        return st;
    }

    // Prepare padded input (using processed data)
    vv_dsp_real* xp = (vv_dsp_real*)malloc((size_t)(N + 2*(size_t)half) * sizeof(vv_dsp_real));
    if (!xp) {
        free(y_processed);
        return VV_DSP_ERROR_INTERNAL;
    }
    pad_signal(y_processed, N, half, mode, xp);

    // Convolution (valid)
    convolve_valid(xp, N, kernel_stack, m, output);

    // Apply NaN/Inf policy to output as well (in case computations generated non-finite values)
    vv_dsp_status output_policy_status = vv_dsp_apply_nan_policy_inplace(output, N);

    free(xp);
    free(y_processed);

    // Return the more critical error if there was one
    if (output_policy_status != VV_DSP_OK) {
        return output_policy_status;
    }

    return VV_DSP_OK;
}
