#ifndef VV_DSP_H
#define VV_DSP_H

#ifdef __cplusplus
extern "C" {
#endif

// vv-dsp umbrella header
// Include public headers for submodules as they become available
#include "vv_dsp/core.h"
#include "vv_dsp/spectral.h"
#include "vv_dsp/filter.h"
#include "vv_dsp/resample.h"
#include "vv_dsp/envelope.h"
#include "vv_dsp/window.h"
#include "vv_dsp/adapters.h"

// Version macros
#define VV_DSP_VERSION_MAJOR 0
#define VV_DSP_VERSION_MINOR 1
#define VV_DSP_VERSION_PATCH 0

#ifdef __cplusplus
} // extern "C"
#endif

#endif // VV_DSP_H
