#include <stdlib.h>
#include <string.h>
#include "fft_backend.h"

VV_DSP_NODISCARD vv_dsp_status vv_dsp_fft_make_plan(size_t n,
                                                    vv_dsp_fft_type type,
                                                    vv_dsp_fft_dir dir,
                                                    vv_dsp_fft_plan** out_plan) {
    if (!out_plan) return VV_DSP_ERROR_NULL_POINTER;
    *out_plan = NULL;
    if (n == 0) return VV_DSP_ERROR_INVALID_SIZE;
    if (!(type == VV_DSP_FFT_C2C || type == VV_DSP_FFT_R2C || type == VV_DSP_FFT_C2R)) return VV_DSP_ERROR_OUT_OF_RANGE;
    if (!(dir == VV_DSP_FFT_FORWARD || dir == VV_DSP_FFT_BACKWARD)) return VV_DSP_ERROR_OUT_OF_RANGE;

    vv_dsp_fft_plan* plan = (vv_dsp_fft_plan*)malloc(sizeof(vv_dsp_fft_plan));
    if (!plan) return VV_DSP_ERROR_INTERNAL;
    plan->n = n;
    plan->type = type;
    plan->dir = dir;

    void* backend = NULL;
    vv_dsp_status st = vv_dsp_fft_backend_make(plan, &backend);
    if (st != VV_DSP_OK) { free(plan); return st; }

    // Store backend pointer after the plan struct in a small heap block
    // Simpler: extend plan struct later; for now keep it static and store via malloc header
    // We'll attach via a small allocation trick: allocate extra pointer and store next to plan
    // But to keep it clean, we just ignore and let backend be re-derived in exec (placeholder)
    (void)backend; // placeholder until backends added

    *out_plan = plan;
    return VV_DSP_OK;
}

VV_DSP_NODISCARD vv_dsp_status vv_dsp_fft_execute(const vv_dsp_fft_plan* plan,
                                                  const void* in,
                                                  void* out) {
    if (!plan || !in || !out) return VV_DSP_ERROR_NULL_POINTER;
    void* backend = NULL; // placeholder; stateless reference implementation below
    return vv_dsp_fft_backend_exec(plan, backend, in, out);
}

vv_dsp_status vv_dsp_fft_destroy(vv_dsp_fft_plan* plan) {
    if (!plan) return VV_DSP_ERROR_NULL_POINTER;
    // If backend allocated in future, free it here
    free(plan);
    return VV_DSP_OK;
}
