#include <stddef.h>
#include <string.h>
#include "vv_dsp/spectral/utils.h"

static vv_dsp_status shift_core_real(const vv_dsp_real* in, vv_dsp_real* out, size_t n, int inverse) {
    if (!in || !out) return VV_DSP_ERROR_NULL_POINTER;
    if (n == 0) return VV_DSP_ERROR_INVALID_SIZE;
    size_t k = n/2;
    if (!inverse) {
        // fftshift: swap halves
        memcpy(out, in + k, sizeof(*out) * (n - k));
        memcpy(out + (n - k), in, sizeof(*out) * k);
    } else {
        // ifftshift: inverse operation
        memcpy(out, in + (n - k), sizeof(*out) * k);
        memcpy(out + k, in, sizeof(*out) * (n - k));
    }
    return VV_DSP_OK;
}

static vv_dsp_status shift_core_cpx(const vv_dsp_cpx* in, vv_dsp_cpx* out, size_t n, int inverse) {
    if (!in || !out) return VV_DSP_ERROR_NULL_POINTER;
    if (n == 0) return VV_DSP_ERROR_INVALID_SIZE;
    size_t k = n/2;
    if (!inverse) {
        memcpy(out, in + k, sizeof(*out) * (n - k));
        memcpy(out + (n - k), in, sizeof(*out) * k);
    } else {
        memcpy(out, in + (n - k), sizeof(*out) * k);
        memcpy(out + k, in, sizeof(*out) * (n - k));
    }
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_fftshift_real(const vv_dsp_real* in, vv_dsp_real* out, size_t n) {
    return shift_core_real(in, out, n, 0);
}
vv_dsp_status vv_dsp_ifftshift_real(const vv_dsp_real* in, vv_dsp_real* out, size_t n) {
    return shift_core_real(in, out, n, 1);
}
vv_dsp_status vv_dsp_fftshift_cpx(const vv_dsp_cpx* in, vv_dsp_cpx* out, size_t n) {
    return shift_core_cpx(in, out, n, 0);
}
vv_dsp_status vv_dsp_ifftshift_cpx(const vv_dsp_cpx* in, vv_dsp_cpx* out, size_t n) {
    return shift_core_cpx(in, out, n, 1);
}

vv_dsp_status vv_dsp_phase_wrap(const vv_dsp_real* in, vv_dsp_real* out, size_t n) {
    if (!in || !out) return VV_DSP_ERROR_NULL_POINTER;
    for (size_t i=0;i<n;++i) {
        vv_dsp_real x = in[i];
        // map to (-pi, pi]
        while (x <= -VV_DSP_PI) x += VV_DSP_TWO_PI;
        while (x > VV_DSP_PI) x -= VV_DSP_TWO_PI;
        out[i] = x;
    }
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_phase_unwrap(const vv_dsp_real* in, vv_dsp_real* out, size_t n) {
    if (!in || !out) return VV_DSP_ERROR_NULL_POINTER;
    if (n==0) return VV_DSP_ERROR_INVALID_SIZE;
    out[0] = in[0];
    for (size_t i=1;i<n;++i) {
        vv_dsp_real delta = in[i] - in[i-1];
        if (delta > VV_DSP_PI) delta -= VV_DSP_TWO_PI;
        else if (delta < -VV_DSP_PI) delta += VV_DSP_TWO_PI;
        out[i] = out[i-1] + delta;
    }
    return VV_DSP_OK;
}
