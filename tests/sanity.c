#include <stdio.h>
#include "vv_dsp/vv_dsp.h"

int main(void) {
    int ok = 1;
    ok &= (vv_dsp_add_int(2, 3) == 5);
    ok &= (vv_dsp_spectral_dummy() == 42);
    ok &= (vv_dsp_filter_dummy() == 7);
    ok &= (vv_dsp_resample_dummy() == 3);
    ok &= (vv_dsp_envelope_dummy() == 5);
    // window module sanity: boxcar(1) should return 1.0
    {
        vv_dsp_real w1[1] = {0};
        ok &= (vv_dsp_window_boxcar(1, w1) == VV_DSP_OK) && (w1[0] == (vv_dsp_real)1.0);
    }
    ok &= (vv_dsp_adapters_dummy() == 1);

    // basic core types and math
    vv_dsp_real x[5] = {1,2,3,4,5};
    vv_dsp_real out = 0;
    size_t idx = 0;
    ok &= (vv_dsp_sum(x, 5, &out) == VV_DSP_OK) && (out == (vv_dsp_real)15);
    ok &= (vv_dsp_min(x, 5, &out) == VV_DSP_OK) && (out == (vv_dsp_real)1);
    ok &= (vv_dsp_max(x, 5, &out) == VV_DSP_OK) && (out == (vv_dsp_real)5);
    ok &= (vv_dsp_argmin(x, 5, &idx) == VV_DSP_OK) && (idx == 0);
    ok &= (vv_dsp_argmax(x, 5, &idx) == VV_DSP_OK) && (idx == 4);

    vv_dsp_real y[5] = {0};
    ok &= (vv_dsp_cumsum(x, 5, y) == VV_DSP_OK) && (y[4] == (vv_dsp_real)15);
    vv_dsp_real d[4] = {0};
    ok &= (vv_dsp_diff(x, 5, d) == VV_DSP_OK) && (d[0] == (vv_dsp_real)1) && (d[3] == (vv_dsp_real)1);

    vv_dsp_cpx a = vv_dsp_cpx_make(1,2);
    vv_dsp_cpx b = vv_dsp_cpx_make(3,4);
    vv_dsp_cpx c = vv_dsp_cpx_add(a,b);
    ok &= (c.re == (vv_dsp_real)4 && c.im == (vv_dsp_real)6);

    if(!ok) {
        fprintf(stderr, "sanity test failed\n");
        return 1;
    }
    printf("sanity test passed\n");
    return 0;
}
