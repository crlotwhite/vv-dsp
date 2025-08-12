#include <stdio.h>
#include "vv_dsp/vv_dsp.h"

int main() {
    printf("Testing vv_dsp_get_num_frames...\n");
    size_t frames = vv_dsp_get_num_frames(48000, 1024, 256, 0);
    printf("Result: %zu frames\n", frames);
    
    printf("Testing STFT creation...\n");
    vv_dsp_stft* stft = NULL;
    vv_dsp_stft_params params = {
        .fft_size = 1024,
        .hop_size = 256,
        .window = VV_DSP_STFT_WIN_HANN
    };
    
    vv_dsp_status status = vv_dsp_stft_create(&params, &stft);
    printf("STFT creation status: %d\n", status);
    
    if (status == VV_DSP_OK && stft) {
        printf("STFT created successfully\n");
        vv_dsp_stft_destroy(stft);
        printf("STFT destroyed successfully\n");
    }
    
    return 0;
}
