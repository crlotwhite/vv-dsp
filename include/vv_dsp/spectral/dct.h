#ifndef VV_DSP_SPECTRAL_DCT_H
#define VV_DSP_SPECTRAL_DCT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "vv_dsp/vv_dsp_types.h"
#include "vv_dsp/core/nan_policy.h"

// DCT type
typedef enum vv_dsp_dct_type {
    VV_DSP_DCT_II = 2,
    VV_DSP_DCT_III = 3,
    VV_DSP_DCT_IV = 4
} vv_dsp_dct_type;

// Reuse FFT dir semantics: FORWARD applies the specified DCT; BACKWARD applies the inverse transform
typedef enum vv_dsp_dct_dir {
    VV_DSP_DCT_FORWARD = +1,
    VV_DSP_DCT_BACKWARD = -1
} vv_dsp_dct_dir;

// Opaque plan
typedef struct vv_dsp_dct_plan vv_dsp_dct_plan;

// Create a DCT plan for length n and type.
// For type II: FORWARD computes DCT-II, BACKWARD computes the inverse (DCT-III scaling) that reconstructs x from DCT-II.
// For type III: FORWARD computes DCT-III, BACKWARD computes its inverse (DCT-II scaling) that reconstructs x from DCT-III.
// For type IV: FORWARD computes DCT-IV, BACKWARD computes the inverse (DCT-IV with 2/N scaling) that reconstructs x from DCT-IV.
VV_DSP_NODISCARD vv_dsp_status vv_dsp_dct_make_plan(size_t n,
                                                    vv_dsp_dct_type type,
                                                    vv_dsp_dct_dir dir,
                                                    vv_dsp_dct_plan** out_plan);

// Execute DCT according to plan. in/out are real arrays of length n.
VV_DSP_NODISCARD vv_dsp_status vv_dsp_dct_execute(const vv_dsp_dct_plan* plan,
                                                  const vv_dsp_real* in,
                                                  vv_dsp_real* out);

// Destroy plan
vv_dsp_status vv_dsp_dct_destroy(vv_dsp_dct_plan* plan);

// Convenience one-shot helpers without managing a plan explicitly
VV_DSP_NODISCARD vv_dsp_status vv_dsp_dct_forward(size_t n,
                                                  vv_dsp_dct_type type,
                                                  const vv_dsp_real* in,
                                                  vv_dsp_real* out);
VV_DSP_NODISCARD vv_dsp_status vv_dsp_dct_inverse(size_t n,
                                                  vv_dsp_dct_type type,
                                                  const vv_dsp_real* in,
                                                  vv_dsp_real* out);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // VV_DSP_SPECTRAL_DCT_H
