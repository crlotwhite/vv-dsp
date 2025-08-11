#include <stdio.h>
#include <math.h>
#include <string.h>
#include "vv_dsp/vv_dsp.h"

static int approx_equal(vv_dsp_real a, vv_dsp_real b) {
#ifdef VV_DSP_USE_DOUBLE
    const vv_dsp_real eps = (vv_dsp_real)1e-9;
#else
    const vv_dsp_real eps = (vv_dsp_real)1e-5f;
#endif
    vv_dsp_real d = a - b;
    if (d < 0) d = -d;
    return d <= eps;
}

static int test_get_num_frames(void) {
    int ok = 1;
    
    // Test non-centered framing
    // signal_len=1024, frame_len=256, hop_len=128, center=0
    // Expected: 1 + (1024 - 256) / 128 = 1 + 768/128 = 1 + 6 = 7
    size_t num_frames = vv_dsp_get_num_frames(1024, 256, 128, 0);
    ok &= (num_frames == 7);
    
    // Test centered framing
    // signal_len=1024, frame_len=256, hop_len=128, center=1
    // Expected: ceil(1024 / 128) = ceil(8) = 8
    num_frames = vv_dsp_get_num_frames(1024, 256, 128, 1);
    ok &= (num_frames == 8);
    
    // Edge case: signal shorter than frame (non-centered)
    num_frames = vv_dsp_get_num_frames(100, 256, 128, 0);
    ok &= (num_frames == 0);
    
    // Edge case: signal shorter than frame (centered)
    num_frames = vv_dsp_get_num_frames(100, 256, 128, 1);
    ok &= (num_frames == 1);  // ceil(100/128) = 1
    
    // Zero hop_len should return 0
    num_frames = vv_dsp_get_num_frames(1024, 256, 0, 0);
    ok &= (num_frames == 0);
    
    return ok;
}

static int test_fetch_frame_non_centered(void) {
    int ok = 1;
    
    // Create a simple test signal: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
    vv_dsp_real signal[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    vv_dsp_real frame[4] = {0};
    
    // Extract first frame (frame_len=4, hop_len=2)
    // Frame 0: should start at index 0*2=0, get [0, 1, 2, 3]
    vv_dsp_status status = vv_dsp_fetch_frame(signal, 10, frame, 4, 2, 0, 0, NULL);
    ok &= (status == VV_DSP_OK);
    ok &= approx_equal(frame[0], 0) && approx_equal(frame[1], 1) && 
          approx_equal(frame[2], 2) && approx_equal(frame[3], 3);
    
    // Extract second frame
    // Frame 1: should start at index 1*2=2, get [2, 3, 4, 5]
    status = vv_dsp_fetch_frame(signal, 10, frame, 4, 2, 1, 0, NULL);
    ok &= (status == VV_DSP_OK);
    ok &= approx_equal(frame[0], 2) && approx_equal(frame[1], 3) && 
          approx_equal(frame[2], 4) && approx_equal(frame[3], 5);
    
    // Extract frame that goes beyond signal boundary
    // Frame 4: should start at index 4*2=8, get [8, 9, 0, 0] (zero-padded)
    status = vv_dsp_fetch_frame(signal, 10, frame, 4, 2, 4, 0, NULL);
    ok &= (status == VV_DSP_OK);
    ok &= approx_equal(frame[0], 8) && approx_equal(frame[1], 9) && 
          approx_equal(frame[2], 0) && approx_equal(frame[3], 0);
    
    return ok;
}

static int test_fetch_frame_centered(void) {
    int ok = 1;
    
    // Create a simple test signal: [1, 2, 3, 4, 5, 6]
    vv_dsp_real signal[6] = {1, 2, 3, 4, 5, 6};
    vv_dsp_real frame[4] = {0};
    
    // Extract first frame with centering (frame_len=4, hop_len=2)
    // Frame 0: center at 0*2=0, start at 0-4/2=-2, with reflection: [3, 2, 1, 2]
    vv_dsp_status status = vv_dsp_fetch_frame(signal, 6, frame, 4, 2, 0, 1, NULL);
    ok &= (status == VV_DSP_OK);
    // With reflection padding for indices [-2, -1, 0, 1]:
    // -2 -> reflect to index 1 (2)
    // -1 -> reflect to index 0 (1)  
    //  0 -> index 0 (1)
    //  1 -> index 1 (2)
    ok &= approx_equal(frame[0], 2) && approx_equal(frame[1], 1) && 
          approx_equal(frame[2], 1) && approx_equal(frame[3], 2);
    
    // Extract middle frame
    // Frame 1: center at 1*2=2, start at 2-4/2=0, get [1, 2, 3, 4]
    status = vv_dsp_fetch_frame(signal, 6, frame, 4, 2, 1, 1, NULL);
    ok &= (status == VV_DSP_OK);
    ok &= approx_equal(frame[0], 1) && approx_equal(frame[1], 2) && 
          approx_equal(frame[2], 3) && approx_equal(frame[3], 4);
    
    return ok;
}

static int test_fetch_frame_windowing(void) {
    int ok = 1;
    
    // Test windowing functionality
    vv_dsp_real signal[4] = {1, 2, 3, 4};
    vv_dsp_real window[4] = {0.5, 1.0, 1.0, 0.5};
    vv_dsp_real frame[4] = {0};
    
    // Extract frame with windowing
    vv_dsp_status status = vv_dsp_fetch_frame(signal, 4, frame, 4, 4, 0, 0, window);
    ok &= (status == VV_DSP_OK);
    
    // Expected: [1*0.5, 2*1.0, 3*1.0, 4*0.5] = [0.5, 2.0, 3.0, 2.0]
    ok &= approx_equal(frame[0], 0.5) && approx_equal(frame[1], 2.0) && 
          approx_equal(frame[2], 3.0) && approx_equal(frame[3], 2.0);
    
    return ok;
}

static int test_overlap_add(void) {
    int ok = 1;
    
    // Test overlap-add functionality
    vv_dsp_real output[8] = {0}; // Initialize to zero
    vv_dsp_real frame1[4] = {1, 2, 3, 4};
    vv_dsp_real frame2[4] = {0.5, 1.0, 1.5, 2.0};
    
    // Add first frame at position 0 (frame_index=0, hop_len=2)
    vv_dsp_status status = vv_dsp_overlap_add(frame1, output, 8, 4, 2, 0);
    ok &= (status == VV_DSP_OK);
    
    // Output should be: [1, 2, 3, 4, 0, 0, 0, 0]
    ok &= approx_equal(output[0], 1) && approx_equal(output[1], 2) && 
          approx_equal(output[2], 3) && approx_equal(output[3], 4);
    
    // Add second frame at position 2 (frame_index=1, hop_len=2)  
    status = vv_dsp_overlap_add(frame2, output, 8, 4, 2, 1);
    ok &= (status == VV_DSP_OK);
    
    // Output should be: [1, 2, 3+0.5, 4+1.0, 1.5, 2.0, 0, 0] = [1, 2, 3.5, 5.0, 1.5, 2.0, 0, 0]
    ok &= approx_equal(output[0], 1) && approx_equal(output[1], 2) && 
          approx_equal(output[2], 3.5) && approx_equal(output[3], 5.0) &&
          approx_equal(output[4], 1.5) && approx_equal(output[5], 2.0) &&
          approx_equal(output[6], 0) && approx_equal(output[7], 0);
    
    return ok;
}

static int test_analysis_synthesis_loop(void) {
    int ok = 1;
    
    // Test a complete analysis-synthesis loop without processing
    vv_dsp_real signal[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    vv_dsp_real output[8] = {0};
    vv_dsp_real frame[4] = {0};
    
    size_t frame_len = 4;
    size_t hop_len = 2;
    size_t signal_len = 8;
    
    // Get number of frames
    size_t num_frames = vv_dsp_get_num_frames(signal_len, frame_len, hop_len, 0);
    // Expected: 1 + (8-4)/2 = 1 + 2 = 3
    ok &= (num_frames == 3);
    
    // Perform analysis-synthesis loop
    for (size_t i = 0; i < num_frames; i++) {
        // Extract frame
        vv_dsp_status status = vv_dsp_fetch_frame(signal, signal_len, frame, frame_len, hop_len, i, 0, NULL);
        ok &= (status == VV_DSP_OK);
        
        // No processing - just add back to output
        status = vv_dsp_overlap_add(frame, output, signal_len, frame_len, hop_len, i);
        ok &= (status == VV_DSP_OK);
    }
    
    // Check reconstruction
    // Frame 0 (0-3): [1,2,3,4] at position 0
    // Frame 1 (2-5): [3,4,5,6] at position 2  
    // Frame 2 (4-7): [5,6,7,8] at position 4
    // Overlap regions: positions 2-3 and 4-5 get summed
    // Expected: [1, 2, 3+3, 4+4, 5+5, 6+6, 7, 8] = [1, 2, 6, 8, 10, 12, 7, 8]
    ok &= approx_equal(output[0], 1) && approx_equal(output[1], 2) && 
          approx_equal(output[2], 6) && approx_equal(output[3], 8) &&
          approx_equal(output[4], 10) && approx_equal(output[5], 12) &&
          approx_equal(output[6], 7) && approx_equal(output[7], 8);
    
    return ok;
}

static int test_error_conditions(void) {
    int ok = 1;
    
    vv_dsp_real signal[4] = {1, 2, 3, 4};
    vv_dsp_real frame[4] = {0};
    vv_dsp_real output[4] = {0};
    
    // Test NULL pointers
    ok &= (vv_dsp_fetch_frame(NULL, 4, frame, 4, 2, 0, 0, NULL) == VV_DSP_ERROR_NULL_POINTER);
    ok &= (vv_dsp_fetch_frame(signal, 4, NULL, 4, 2, 0, 0, NULL) == VV_DSP_ERROR_NULL_POINTER);
    ok &= (vv_dsp_overlap_add(NULL, output, 4, 4, 2, 0) == VV_DSP_ERROR_NULL_POINTER);
    ok &= (vv_dsp_overlap_add(frame, NULL, 4, 4, 2, 0) == VV_DSP_ERROR_NULL_POINTER);
    
    // Test zero sizes
    ok &= (vv_dsp_fetch_frame(signal, 0, frame, 4, 2, 0, 0, NULL) == VV_DSP_ERROR_INVALID_SIZE);
    ok &= (vv_dsp_fetch_frame(signal, 4, frame, 0, 2, 0, 0, NULL) == VV_DSP_ERROR_INVALID_SIZE);
    ok &= (vv_dsp_fetch_frame(signal, 4, frame, 4, 0, 0, 0, NULL) == VV_DSP_ERROR_INVALID_SIZE);
    ok &= (vv_dsp_overlap_add(frame, output, 0, 4, 2, 0) == VV_DSP_ERROR_INVALID_SIZE);
    ok &= (vv_dsp_overlap_add(frame, output, 4, 0, 2, 0) == VV_DSP_ERROR_INVALID_SIZE);
    ok &= (vv_dsp_overlap_add(frame, output, 4, 4, 0, 0) == VV_DSP_ERROR_INVALID_SIZE);
    
    return ok;
}

int main(void) {
    int ok = 1;
    
    printf("Running framing tests...\n");
    
    ok &= test_get_num_frames();
    printf("  test_get_num_frames: %s\n", ok ? "PASS" : "FAIL");
    
    ok &= test_fetch_frame_non_centered();
    printf("  test_fetch_frame_non_centered: %s\n", ok ? "PASS" : "FAIL");
    
    ok &= test_fetch_frame_centered();
    printf("  test_fetch_frame_centered: %s\n", ok ? "PASS" : "FAIL");
    
    ok &= test_fetch_frame_windowing();
    printf("  test_fetch_frame_windowing: %s\n", ok ? "PASS" : "FAIL");
    
    ok &= test_overlap_add();
    printf("  test_overlap_add: %s\n", ok ? "PASS" : "FAIL");
    
    ok &= test_analysis_synthesis_loop();
    printf("  test_analysis_synthesis_loop: %s\n", ok ? "PASS" : "FAIL");
    
    ok &= test_error_conditions();
    printf("  test_error_conditions: %s\n", ok ? "PASS" : "FAIL");
    
    printf("\nAll framing tests: %s\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
