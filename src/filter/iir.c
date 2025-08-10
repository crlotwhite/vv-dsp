#include "vv_dsp/filter/iir.h"

vv_dsp_status vv_dsp_biquad_init(vv_dsp_biquad* bq,
                                 vv_dsp_real b0,
                                 vv_dsp_real b1,
                                 vv_dsp_real b2,
                                 vv_dsp_real a1,
                                 vv_dsp_real a2) {
    if (!bq) return VV_DSP_ERROR_NULL_POINTER;
    bq->b0 = b0; bq->b1 = b1; bq->b2 = b2;
    bq->a1 = a1; bq->a2 = a2;
    bq->z1 = 0; bq->z2 = 0;
    return VV_DSP_OK;
}

void vv_dsp_biquad_reset(vv_dsp_biquad* bq) {
    if (!bq) return;
    bq->z1 = 0; bq->z2 = 0;
}

vv_dsp_real vv_dsp_biquad_process(vv_dsp_biquad* bq, vv_dsp_real x) {
    // Direct Form II Transposed
    vv_dsp_real y = bq->b0 * x + bq->z1;
    bq->z1 = bq->b1 * x - bq->a1 * y + bq->z2;
    bq->z2 = bq->b2 * x - bq->a2 * y;
    return y;
}

vv_dsp_status vv_dsp_iir_apply(vv_dsp_biquad* biquads,
                               size_t num_stages,
                               const vv_dsp_real* input,
                               vv_dsp_real* output,
                               size_t n) {
    if (!input || !output || (!biquads && num_stages>0)) return VV_DSP_ERROR_NULL_POINTER;
    for (size_t i = 0; i < n; ++i) {
        vv_dsp_real v = input[i];
        for (size_t s = 0; s < num_stages; ++s) {
            v = vv_dsp_biquad_process(&biquads[s], v);
        }
        output[i] = v;
    }
    return VV_DSP_OK;
}
