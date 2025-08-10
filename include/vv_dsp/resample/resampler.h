#ifndef VV_DSP_RESAMPLE_RESAMPLER_H
#define VV_DSP_RESAMPLE_RESAMPLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vv_dsp/vv_dsp_types.h"

typedef struct vv_dsp_resampler vv_dsp_resampler; // opaque

// Create a resampler with a fixed ratio (out_rate/in_rate = ratio_num/ratio_den).
// ratio_den and ratio_num must be > 0. Returns NULL on invalid params or OOM.
vv_dsp_resampler* vv_dsp_resampler_create(unsigned int ratio_num,
                                          unsigned int ratio_den);

// Destroy and free resources
void vv_dsp_resampler_destroy(vv_dsp_resampler* rs);

// Optionally change the fixed ratio after creation.
// Returns VV_DSP_OK on success.
int vv_dsp_resampler_set_ratio(vv_dsp_resampler* rs,
                               unsigned int ratio_num,
                               unsigned int ratio_den);

// Enable/disable sinc-based filtering and set taps (clamped to a sane range)
// use_sinc: 0 -> linear interpolation, nonzero -> windowed-sinc
// taps: preferred number of taps (even recommended). Returns VV_DSP_OK on success.
int vv_dsp_resampler_set_quality(vv_dsp_resampler* rs, int use_sinc, unsigned int taps);

// Process real-valued input. For now, processes a whole contiguous buffer in one call.
// out_cap is the capacity of out[]; out_n receives the number of samples written.
// Returns VV_DSP_OK on success or error codes on invalid args / insufficient capacity.
int vv_dsp_resampler_process_real(vv_dsp_resampler* rs,
                                  const vv_dsp_real* in, size_t in_n,
                                  vv_dsp_real* out, size_t out_cap,
                                  size_t* out_n);

#ifdef __cplusplus
}
#endif

#endif // VV_DSP_RESAMPLE_RESAMPLER_H
