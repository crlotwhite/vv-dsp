#include "vv_dsp/filter/common.h"
#include "vv_dsp/filter/fir.h"
#include <stdlib.h>
#include <string.h>

// Simple reflection padding helper
static void reflect_pad(const vv_dsp_real* in, size_t n, size_t pad, vv_dsp_real* out) {
    // out length = n + 2*pad
    // copy center
    memcpy(out + pad, in, n * sizeof(vv_dsp_real));
    // left pad: reflect starting from index 1 back
    for (size_t i = 0; i < pad; ++i) {
        size_t src = (i + 1 <= n) ? (i + 1) : n; // guard
        out[pad - 1 - i] = in[src - 1];
    }
    // right pad: reflect from end-2 backward
    for (size_t i = 0; i < pad; ++i) {
        size_t src = (i + 1 <= n) ? (n - 1 - i) : 0;
        out[pad + n + i] = in[src];
    }
}

vv_dsp_status vv_dsp_filtfilt_fir(const vv_dsp_real* coeffs,
                                  size_t num_taps,
                                  const vv_dsp_real* input,
                                  vv_dsp_real* output,
                                  size_t num_samples) {
    if (!coeffs || !input || !output) return VV_DSP_ERROR_NULL_POINTER;
    if (num_taps == 0) return VV_DSP_ERROR_INVALID_SIZE;

    // Minimal padding = num_taps-1
    const size_t pad = (num_taps > 1) ? (num_taps - 1) : 0;
    const size_t ext_n = num_samples + 2 * pad;

    vv_dsp_real* ext = (vv_dsp_real*)malloc(ext_n * sizeof(vv_dsp_real));
    if (!ext) return VV_DSP_ERROR_INTERNAL;

    reflect_pad(input, num_samples, pad, ext);

    // Forward filter (stateless direct convolution)
    vv_dsp_real* tmp = (vv_dsp_real*)malloc(ext_n * sizeof(vv_dsp_real));
    if (!tmp) { free(ext); return VV_DSP_ERROR_INTERNAL; }
    // y = h (*) x with zero-padding
    for (size_t i = 0; i < ext_n; ++i) {
        vv_dsp_real acc = 0;
        size_t maxk = (i + 1 < num_taps) ? (i + 1) : num_taps;
        for (size_t k = 0; k < maxk; ++k) acc += coeffs[k] * ext[i - k];
        tmp[i] = acc;
    }

    // Reverse
    for (size_t i = 0; i < ext_n / 2; ++i) {
        vv_dsp_real t = tmp[i];
        tmp[i] = tmp[ext_n - 1 - i];
        tmp[ext_n - 1 - i] = t;
    }

    // Backward filter (stateless direct convolution)
    vv_dsp_real* tmp2 = (vv_dsp_real*)malloc(ext_n * sizeof(vv_dsp_real));
    if (!tmp2) { free(tmp); free(ext); return VV_DSP_ERROR_INTERNAL; }
    for (size_t i = 0; i < ext_n; ++i) {
        vv_dsp_real acc = 0;
        size_t maxk = (i + 1 < num_taps) ? (i + 1) : num_taps;
        for (size_t k = 0; k < maxk; ++k) acc += coeffs[k] * tmp[i - k];
        tmp2[i] = acc;
    }

    // Reverse back and extract center
    for (size_t i = 0; i < ext_n / 2; ++i) {
        vv_dsp_real t = tmp2[i];
        tmp2[i] = tmp2[ext_n - 1 - i];
        tmp2[ext_n - 1 - i] = t;
    }
    memcpy(output, tmp2 + pad, num_samples * sizeof(vv_dsp_real));

    free(tmp2);
    free(tmp);
    free(ext);
    return VV_DSP_OK;
}
