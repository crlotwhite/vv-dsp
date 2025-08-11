/*
 * vv-dsp Filter module public umbrella header
 */
#ifndef VV_DSP_FILTER_H
#define VV_DSP_FILTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vv_dsp/vv_dsp_types.h"

// Public sub-APIs
#include "vv_dsp/filter/common.h"
#include "vv_dsp/filter/fir.h"
#include "vv_dsp/filter/iir.h"
#include "vv_dsp/filter/savgol.h"

// Legacy placeholder kept for sanity tests
int vv_dsp_filter_dummy(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // VV_DSP_FILTER_H
