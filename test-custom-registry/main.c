#include <stdio.h>
#include <vv_dsp/vv_dsp.h>

int main(void) {
    printf("Testing vv-dsp from custom registry...\n");

    const size_t N = 64;
    vv_dsp_real window[64];
    vv_dsp_status status = vv_dsp_window_hann(N, window);

    if (status != VV_DSP_OK) {
        printf("FAILED: Could not create window\n");
        return 1;
    }

    printf("SUCCESS: Custom registry integration test passed\n");
    printf("Window function created with %zu samples\n", N);
    printf("Sample values: window[0]=%.3f, window[16]=%.3f, window[32]=%.3f\n",
           window[0], window[16], window[32]);

    return 0;
}
