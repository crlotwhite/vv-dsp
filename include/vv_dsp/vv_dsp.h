#ifndef VV_DSP_H
#define VV_DSP_H

#ifdef __cplusplus
extern "C" {
#endif

// vv-dsp umbrella header
// Include public headers for submodules as they become available
#include "vv_dsp/vv_dsp_types.h"
#include "vv_dsp/vv_dsp_math.h"
#include "vv_dsp/core.h"
#include "vv_dsp/spectral.h"
#include "vv_dsp/spectral/dct.h"
#include "vv_dsp/spectral/fft.h"
#include "vv_dsp/filter.h"
#include "vv_dsp/resample.h"
#include "vv_dsp/envelope.h"
#include "vv_dsp/window.h"
#include "vv_dsp/adapters.h"

#ifdef VV_DSP_AUDIO_ENABLED
#include "vv_dsp/audio.h"
#endif

// Version macros
#define VV_DSP_VERSION_MAJOR 0
#define VV_DSP_VERSION_MINOR 1
#define VV_DSP_VERSION_PATCH 0

#ifdef __cplusplus
} // extern "C"
#endif

#endif // VV_DSP_H
