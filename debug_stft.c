#include <stdio.h>
#include <stdlib.h>
#include "vv_dsp/vv_dsp.h"

int main() {
    printf("=== Debugging STFT components ===\n");
    
    // Test 1: Basic function availability
    printf("1. Testing vv_dsp_get_num_frames...\n");
    size_t frames = vv_dsp_get_num_frames(48000, 1024, 256, 0);
    printf("   Result: %zu frames\n", frames);
    
    // Test 2: STFT creation
    printf("2. Testing STFT creation...\n");
    vv_dsp_stft* stft = NULL;
    vv_dsp_stft_params params = {
        .fft_size = 1024,
        .hop_size = 256,
        .window = VV_DSP_STFT_WIN_HANN
    };
    
    vv_dsp_status status = vv_dsp_stft_create(&params, &stft);
    printf("   STFT creation status: %d\n", status);
    
    if (status != VV_DSP_OK || !stft) {
        printf("   ERROR: Failed to create STFT\n");
        return 1;
    }
    
    printf("   STFT created successfully\n");
    
    // Test 3: Frame processing
    printf("3. Testing frame processing...\n");
    const size_t signal_len = 48000;
    const size_t frame_size = 1024;
    const size_t hop_size = 256;
    
    // Allocate test data
    vv_dsp_real* test_signal = malloc(signal_len * sizeof(vv_dsp_real));
    vv_dsp_real* frame_buffer = malloc(frame_size * sizeof(vv_dsp_real));
    vv_dsp_cpx* spectrum = malloc(frame_size * sizeof(vv_dsp_cpx));
    
    if (!test_signal || !frame_buffer || !spectrum) {
        printf("   ERROR: Memory allocation failed\n");
        goto cleanup;
    }
    
    // Generate test signal
    printf("   Generating test signal...\n");
    for (size_t i = 0; i < signal_len; i++) {
        test_signal[i] = 0.5f * sinf(2.0f * 3.14159f * 440.0f * i / 48000.0f);
    }
    
    // Test frame fetching
    printf("   Testing frame fetching...\n");
    status = vv_dsp_fetch_frame(test_signal, signal_len,
                               frame_buffer, frame_size,
                               hop_size, 0, 0, NULL);
    printf("   Frame fetch status: %d\n", status);
    
    if (status != VV_DSP_OK) {
        printf("   ERROR: Frame fetch failed\n");
        goto cleanup;
    }
    
    // Test STFT processing  
    printf("   Testing STFT processing...\n");
    status = vv_dsp_stft_process(stft, frame_buffer, spectrum);
    printf("   STFT process status: %d\n", status);
    
    if (status != VV_DSP_OK) {
        printf("   ERROR: STFT processing failed\n");
        goto cleanup;
    }
    
    printf("   STFT processing successful\n");
    
    // Test a few frames to see if there's a pattern
    printf("4. Testing multiple frames (max 5)...\n");
    size_t num_frames = vv_dsp_get_num_frames(signal_len, frame_size, hop_size, 0);
    printf("   Total frames: %zu\n", num_frames);
    
    size_t test_frames = (num_frames > 5) ? 5 : num_frames;
    for (size_t frame_idx = 0; frame_idx < test_frames; frame_idx++) {
        printf("   Processing frame %zu...\n", frame_idx);
        
        status = vv_dsp_fetch_frame(test_signal, signal_len,
                                   frame_buffer, frame_size,
                                   hop_size, frame_idx, 0, NULL);
        if (status != VV_DSP_OK) {
            printf("   ERROR: Frame %zu fetch failed\n", frame_idx);
            break;
        }
        
        status = vv_dsp_stft_process(stft, frame_buffer, spectrum);
        if (status != VV_DSP_OK) {
            printf("   ERROR: Frame %zu STFT processing failed\n", frame_idx);
            break;
        }
        
        printf("   Frame %zu processed successfully\n", frame_idx);
    }
    
    printf("=== All tests completed successfully ===\n");
    
cleanup:
    free(test_signal);
    free(frame_buffer);
    free(spectrum);
    
    if (stft) {
        vv_dsp_stft_destroy(stft);
        printf("STFT destroyed successfully\n");
    }
    
    return 0;
}
