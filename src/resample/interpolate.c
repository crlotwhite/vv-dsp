#include <math.h>
#include "vv_dsp/resample/interpolate.h"

vv_dsp_status vv_dsp_interpolate_linear_real(const vv_dsp_real* x,
                                             size_t n,
                                             vv_dsp_real pos,
                                             vv_dsp_real* out) {
    if (!x || !out) return VV_DSP_ERROR_NULL_POINTER;
    if (n == 0) return VV_DSP_ERROR_INVALID_SIZE;

    if (pos <= 0) { *out = x[0]; return VV_DSP_OK; }
    vv_dsp_real max_index = (vv_dsp_real)(n - 1);
    if (pos >= max_index) { *out = x[n - 1]; return VV_DSP_OK; }

    size_t i = (size_t)floor((double)pos);
    vv_dsp_real t = pos - (vv_dsp_real)i;
    vv_dsp_real a = x[i];
    vv_dsp_real b = x[i + 1];
    *out = (vv_dsp_real)((1.0 - t) * a + t * b);
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_interpolate_cubic_real(const vv_dsp_real* x,
                                            size_t n,
                                            vv_dsp_real pos,
                                            vv_dsp_real* out) {
    if (!x || !out) return VV_DSP_ERROR_NULL_POINTER;
    if (n == 0) return VV_DSP_ERROR_INVALID_SIZE;
    if (n < 2) { *out = x[0]; return VV_DSP_OK; }

    if (pos <= 0) { *out = x[0]; return VV_DSP_OK; }
    vv_dsp_real max_index = (vv_dsp_real)(n - 1);
    if (pos >= max_index) { *out = x[n - 1]; return VV_DSP_OK; }

    size_t i = (size_t)floor((double)pos);
    vv_dsp_real t = pos - (vv_dsp_real)i;

    // Neighbor indices with clamping
    size_t i0 = (i == 0) ? 0 : (i - 1);
    size_t i1 = i;
    size_t i2 = (i + 1 < n) ? (i + 1) : (n - 1);
    size_t i3 = (i + 2 < n) ? (i + 2) : (n - 1);

    vv_dsp_real p0 = x[i0];
    vv_dsp_real p1 = x[i1];
    vv_dsp_real p2 = x[i2];
    vv_dsp_real p3 = x[i3];

    // Catmull-Rom spline basis (uniform):
    // Interpolate between p1 and p2 with tangent m1=(p2-p0)/2, m2=(p3-p1)/2
    vv_dsp_real m1 = (vv_dsp_real)0.5 * (p2 - p0);
    vv_dsp_real m2 = (vv_dsp_real)0.5 * (p3 - p1);

    vv_dsp_real t2 = t * t;
    vv_dsp_real t3 = t2 * t;

    vv_dsp_real h00 = (vv_dsp_real)(2.0 * t3 - 3.0 * t2 + 1.0);
    vv_dsp_real h10 = (vv_dsp_real)(t3 - 2.0 * t2 + t);
    vv_dsp_real h01 = (vv_dsp_real)(-2.0 * t3 + 3.0 * t2);
    vv_dsp_real h11 = (vv_dsp_real)(t3 - t2);

    *out = (vv_dsp_real)(h00 * p1 + h10 * m1 + h01 * p2 + h11 * m2);
    return VV_DSP_OK;
}
