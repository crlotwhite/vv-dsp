#include <stdio.h>
#include <vv_dsp/vv_dsp.h>

int main(void) {
    printf("vv-dsp Consumer Test\n");

    printf("vv-dsp integration: SUCCESS\n");

    // Test window function
    const size_t N = 64;
    vv_dsp_real window[64];
    vv_dsp_status status = vv_dsp_window_hann(N, window);
    if (status != VV_DSP_OK) {
        printf("Failed to create Hann window: %d\n", status);
        return 1;
    }

    printf("Hann window creation: SUCCESS\n");
    printf("Window[0] = %f, Window[N/2] = %f, Window[N-1] = %f\n",
           window[0], window[N/2], window[N-1]);

    // Test FFT plan creation
    vv_dsp_fft_plan* plan = NULL;
    status = vv_dsp_fft_make_plan(N, VV_DSP_FFT_C2C, VV_DSP_FFT_FORWARD, &plan);
    if (status != VV_DSP_OK) {
        printf("Failed to create FFT plan: %d\n", status);
        return 1;
    }

    printf("FFT plan creation: SUCCESS\n");

    // Clean up
    vv_dsp_fft_destroy(plan);

    printf("All tests passed!\n");
    return 0;
}
