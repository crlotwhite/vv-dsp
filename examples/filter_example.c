#include <stdio.h>
#include <stdlib.h>
#include "vv_dsp/vv_dsp.h"

int main(void) {
    const size_t N = 33; vv_dsp_real h[N];
    if (vv_dsp_fir_design_lowpass(h, N, (vv_dsp_real)0.2, VV_DSP_WINDOW_HAMMING) != VV_DSP_OK) return 1;
    const size_t L = 128; vv_dsp_real x[L]; vv_dsp_real y[L];
    for (size_t i=0;i<L;++i) x[i] = (vv_dsp_real)((i%10)<5 ? 1.0f : -1.0f);
    vv_dsp_fir_state st; vv_dsp_fir_state_init(&st, N);
    vv_dsp_fir_apply(&st, h, x, y, L);
    vv_dsp_fir_state_free(&st);
    printf("y[0..4]: %f %f %f %f %f\n", (double)y[0], (double)y[1], (double)y[2], (double)y[3], (double)y[4]);
    return 0;
}
