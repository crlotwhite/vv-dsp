#include <stdlib.h>
#include <math.h>
#include "vv_dsp/vv_dsp_math.h"
#include "vv_dsp/resample/resampler.h"
#include "vv_dsp/resample/interpolate.h"

struct vv_dsp_resampler {
    unsigned int ratio_num;
    unsigned int ratio_den;
    int use_sinc;
    unsigned int taps;
    double cutoff; // normalized (0..1], auto-set from ratio
    // TODO: polyphase filter coeffs, state buffers, phase index, etc.
};

vv_dsp_resampler* vv_dsp_resampler_create(unsigned int ratio_num,
                                          unsigned int ratio_den) {
    if (ratio_num == 0 || ratio_den == 0) return NULL;
    vv_dsp_resampler* rs = (vv_dsp_resampler*)malloc(sizeof(vv_dsp_resampler));
    if (!rs) return NULL;
    rs->ratio_num = ratio_num;
    rs->ratio_den = ratio_den;
    rs->use_sinc = 0;
    rs->taps = 32;
    rs->cutoff = fmin(1.0, (double)ratio_num / (double)ratio_den);
    return rs;
}

void vv_dsp_resampler_destroy(vv_dsp_resampler* rs) {
    if (!rs) return;
    free(rs);
}

int vv_dsp_resampler_set_ratio(vv_dsp_resampler* rs,
                               unsigned int ratio_num,
                               unsigned int ratio_den) {
    if (!rs) return VV_DSP_ERROR_NULL_POINTER;
    if (ratio_num == 0 || ratio_den == 0) return VV_DSP_ERROR_OUT_OF_RANGE;
    rs->ratio_num = ratio_num;
    rs->ratio_den = ratio_den;
    rs->cutoff = fmin(1.0, (double)ratio_num / (double)ratio_den);
    return VV_DSP_OK;
}

int vv_dsp_resampler_set_quality(vv_dsp_resampler* rs, int use_sinc, unsigned int taps) {
    if (!rs) return VV_DSP_ERROR_NULL_POINTER;
    rs->use_sinc = use_sinc ? 1 : 0;
    if (taps < 4) taps = 4;
    if (taps > 128) taps = 128;
    rs->taps = taps;
    return VV_DSP_OK;
}

static VV_DSP_INLINE vv_dsp_real sinc_fn(double x) {
    if (x == 0.0) return (vv_dsp_real)1.0;
    double pix = VV_DSP_PI_D * x;
    return (vv_dsp_real)(sin(pix) / pix);
}

static VV_DSP_INLINE double hann_window(unsigned int m, unsigned int N) {
    if (N <= 1) return 1.0;
    return 0.5 - 0.5 * cos((VV_DSP_TWO_PI_D * (double)m) / (double)(N - 1));
}

int vv_dsp_resampler_process_real(vv_dsp_resampler* rs,
                                  const vv_dsp_real* in, size_t in_n,
                                  vv_dsp_real* out, size_t out_cap,
                                  size_t* out_n) {
    if (!rs || !in || !out || !out_n) return VV_DSP_ERROR_NULL_POINTER;
    if (in_n == 0) { *out_n = 0; return VV_DSP_OK; }
    // Compute expected output length for fixed ratio (nearest floor)
    double ratio = (double)rs->ratio_num / (double)rs->ratio_den;
    size_t expect = (size_t)floor((in_n - 1) * ratio) + 1; // map endpoints
    if (expect > out_cap) return VV_DSP_ERROR_INVALID_SIZE;

    *out_n = expect;
    if (!rs->use_sinc) {
        // Linear interpolation path
        for (size_t k = 0; k < expect; ++k) {
            double in_pos = (double)k / ratio; // position in input domain
            vv_dsp_real y = 0;
            vv_dsp_interpolate_linear_real(in, in_n, (vv_dsp_real)in_pos, &y);
            out[k] = y;
        }
        return VV_DSP_OK;
    }

    // Windowed-sinc path with automatic cutoff for anti-aliasing
    unsigned int taps = rs->taps;
    if (taps < 4) taps = 4;
    if ((taps % 2) == 1) taps += 1; // ensure even taps for symmetry
    int half = (int)(taps / 2);
    double cutoff = rs->cutoff; // 0..1

    for (size_t k = 0; k < expect; ++k) {
        double in_pos = (double)k / ratio; // fractional index
        double acc = 0.0;
        double wsum = 0.0;
        int center = (int)floor(in_pos);
        for (int m = -half; m < (int)taps - half; ++m) {
            int idx = center + m;
            double t = (double)idx - in_pos; // distance from fractional center
            // Sinc with cutoff scaling
            double s = (double)sinc_fn(t * cutoff);
            // Hann window (0..taps-1)
            unsigned int mi = (unsigned int)(m + half);
            double w = hann_window(mi, taps);
            double weight = s * w;
            // Sample with clamping at edges
            if (idx < 0) idx = 0;
            if (idx >= (int)in_n) idx = (int)in_n - 1;
            acc += (double)in[idx] * weight;
            wsum += weight;
        }
        // Normalize by windowed kernel sum to reduce amplitude bias
        if (wsum != 0.0) acc /= wsum;
        out[k] = (vv_dsp_real)acc;
    }
    return VV_DSP_OK;
}
