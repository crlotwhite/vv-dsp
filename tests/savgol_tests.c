#include <stdio.h>
#include <string.h>
#include "vv_dsp/vv_dsp.h"

static int expect_status(vv_dsp_status got, vv_dsp_status expect, const char* msg) {
    if (got != expect) {
        fprintf(stderr, "FAIL: %s (got=%d expect=%d)\n", msg, (int)got, (int)expect);
        return 1;
    }
    return 0;
}

int main(void) {
    int fails = 0;
    const vv_dsp_real x[5] = {1,2,3,4,5};
    vv_dsp_real y[5];

    // even window_length -> OUT_OF_RANGE
    fails += expect_status(vv_dsp_savgol(x, 5, 4, 2, 0, (vv_dsp_real)1, VV_DSP_SAVGOL_MODE_REFLECT, y),
                           VV_DSP_ERROR_OUT_OF_RANGE,
                           "even window_length should be OUT_OF_RANGE");

    // polyorder < 0 -> OUT_OF_RANGE
    fails += expect_status(vv_dsp_savgol(x, 5, 5, -1, 0, (vv_dsp_real)1, VV_DSP_SAVGOL_MODE_REFLECT, y),
                           VV_DSP_ERROR_OUT_OF_RANGE,
                           "negative polyorder should be OUT_OF_RANGE");

    // deriv > polyorder -> OUT_OF_RANGE
    fails += expect_status(vv_dsp_savgol(x, 5, 5, 2, 3, (vv_dsp_real)1, VV_DSP_SAVGOL_MODE_REFLECT, y),
                           VV_DSP_ERROR_OUT_OF_RANGE,
                           "deriv > polyorder should be OUT_OF_RANGE");

    // window_length > N -> INVALID_SIZE
    fails += expect_status(vv_dsp_savgol(x, 5, 7, 2, 0, (vv_dsp_real)1, VV_DSP_SAVGOL_MODE_REFLECT, y),
                           VV_DSP_ERROR_INVALID_SIZE,
                           "window_length > N should be INVALID_SIZE");

    // deriv > 0 and delta <= 0 -> OUT_OF_RANGE
    fails += expect_status(vv_dsp_savgol(x, 5, 5, 2, 1, (vv_dsp_real)0, VV_DSP_SAVGOL_MODE_REFLECT, y),
                           VV_DSP_ERROR_OUT_OF_RANGE,
                           "nonpositive delta with deriv>0 should be OUT_OF_RANGE");

    // Valid args now implemented -> OK
    fails += expect_status(vv_dsp_savgol(x, 5, 5, 2, 0, (vv_dsp_real)1, VV_DSP_SAVGOL_MODE_REFLECT, y),
                           VV_DSP_OK,
                           "valid args should return OK");

    if (fails) {
        fprintf(stderr, "savgol_tests: %d failures\n", fails);
        return 1;
    }
    printf("savgol_tests: OK\n");
    return 0;
}
