/*
 * IIR (biquad) filter API
 */
#ifndef VV_DSP_FILTER_IIR_H
#define VV_DSP_FILTER_IIR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vv_dsp/vv_dsp_types.h"

/** Direct Form II Transposed biquad structure */
typedef struct {
    vv_dsp_real a1, a2, b0, b1, b2; // coefficients (a0 assumed 1)
    vv_dsp_real z1, z2;             // state
} vv_dsp_biquad;

vv_dsp_status vv_dsp_biquad_init(vv_dsp_biquad* biquad,
                                 vv_dsp_real b0,
                                 vv_dsp_real b1,
                                 vv_dsp_real b2,
                                 vv_dsp_real a1,
                                 vv_dsp_real a2);

void vv_dsp_biquad_reset(vv_dsp_biquad* biquad);

/** Process one sample through the biquad */
vv_dsp_real vv_dsp_biquad_process(vv_dsp_biquad* biquad, vv_dsp_real input_sample);

vv_dsp_status vv_dsp_iir_apply(vv_dsp_biquad* biquads,
                               size_t num_stages,
                               const vv_dsp_real* input,
                               vv_dsp_real* output,
                               size_t num_samples);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // VV_DSP_FILTER_IIR_H
