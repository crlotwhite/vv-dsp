#include "vv_dsp/audio.h"
#include "vv_dsp/vv_dsp_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

// Test constants
#define TEST_SAMPLE_RATE 44100.0
#define TEST_DURATION_SAMPLES 1024
#define TEST_FREQUENCY 440.0
#define PI 3.14159265358979323846

// Test helper functions
static void generate_sine_wave(vv_dsp_real** channels, int num_channels, size_t num_samples, double frequency, double sample_rate) {
    for (int ch = 0; ch < num_channels; ch++) {
        for (size_t i = 0; i < num_samples; i++) {
            double phase = 2.0 * PI * frequency * (double)i / sample_rate;
            // Slightly different phase per channel to test multichannel
            phase += (double)ch * PI / 4.0;
            channels[ch][i] = (vv_dsp_real)(0.5 * sin(phase));
        }
    }
}

static int compare_buffers(vv_dsp_real** a, vv_dsp_real** b, int num_channels, size_t num_samples, vv_dsp_real tolerance) {
    for (int ch = 0; ch < num_channels; ch++) {
        for (size_t i = 0; i < num_samples; i++) {
            vv_dsp_real diff = fabsf(a[ch][i] - b[ch][i]);
            if (diff > tolerance) {
                printf("Buffer mismatch at channel %d, sample %zu: %f vs %f (diff: %f)\n", 
                       ch, i, (double)a[ch][i], (double)b[ch][i], (double)diff);
                return 0;
            }
        }
    }
    return 1;
}

// Test WAV I/O info function
static int test_wav_info_invalid_file(void) {
    printf("Testing WAV info with invalid file...\n");
    
    vv_dsp_wav_info info;
    vv_dsp_status status = vv_dsp_wav_info_get("nonexistent_file.wav", &info);
    
    if (status == VV_DSP_OK) {
        printf("ERROR: Expected failure for nonexistent file\n");
        return 0;
    }
    
    printf("Correctly failed with status: %d\n", status);
    return 1;
}

// Test WAV roundtrip for different formats
static int test_wav_roundtrip(int bit_depth, int is_float, int num_channels) {
    printf("Testing WAV roundtrip: %d-bit %s, %d channels...\n", 
           bit_depth, is_float ? "float" : "PCM", num_channels);
    
    // Allocate original buffers
    vv_dsp_real** original_buffers = (vv_dsp_real**)malloc(sizeof(vv_dsp_real*) * (size_t)num_channels);
    if (!original_buffers) {
        printf("ERROR: Failed to allocate original buffer array\n");
        return 0;
    }
    
    for (int ch = 0; ch < num_channels; ch++) {
        original_buffers[ch] = (vv_dsp_real*)malloc(sizeof(vv_dsp_real) * TEST_DURATION_SAMPLES);
        if (!original_buffers[ch]) {
            printf("ERROR: Failed to allocate original buffer for channel %d\n", ch);
            return 0;
        }
    }
    
    // Generate test signal
    generate_sine_wave(original_buffers, num_channels, TEST_DURATION_SAMPLES, TEST_FREQUENCY, TEST_SAMPLE_RATE);
    
    // Setup WAV info
    vv_dsp_wav_info write_info;
    write_info.num_samples = TEST_DURATION_SAMPLES;
    write_info.num_channels = num_channels;
    write_info.sample_rate = TEST_SAMPLE_RATE;
    write_info.bit_depth = bit_depth;
    write_info.is_float = is_float;
    
    // Write WAV file
    const char* temp_filename = "/tmp/test_audio.wav";
    vv_dsp_status status = vv_dsp_wav_write(temp_filename, (const vv_dsp_real* const*)original_buffers, &write_info);
    if (status != VV_DSP_OK) {
        printf("ERROR: Failed to write WAV file: %s\n", vv_dsp_wav_get_error_string());
        return 0;
    }
    
    // Read WAV file back
    vv_dsp_real** read_buffers = NULL;
    vv_dsp_wav_info read_info;
    status = vv_dsp_wav_read(temp_filename, &read_buffers, &read_info);
    if (status != VV_DSP_OK) {
        printf("ERROR: Failed to read WAV file: %s\n", vv_dsp_wav_get_error_string());
        return 0;
    }
    
    // Verify metadata
    if (read_info.num_samples != write_info.num_samples ||
        read_info.num_channels != write_info.num_channels ||
        fabs(read_info.sample_rate - write_info.sample_rate) > 1.0 ||
        read_info.bit_depth != write_info.bit_depth ||
        read_info.is_float != write_info.is_float) {
        printf("ERROR: Metadata mismatch\n");
        printf("  Expected: %zu samples, %d channels, %.1f Hz, %d bits, %s\n",
               write_info.num_samples, write_info.num_channels, write_info.sample_rate,
               write_info.bit_depth, write_info.is_float ? "float" : "int");
        printf("  Got:      %zu samples, %d channels, %.1f Hz, %d bits, %s\n",
               read_info.num_samples, read_info.num_channels, read_info.sample_rate,
               read_info.bit_depth, read_info.is_float ? "float" : "int");
        return 0;
    }
    
    // Compare audio data (with tolerance for quantization)
    vv_dsp_real tolerance;
    if (is_float) {
        tolerance = (vv_dsp_real)1e-6;  // Very tight tolerance for float
    } else if (bit_depth == 16) {
        tolerance = (vv_dsp_real)(10.0 / 65536.0);  // ~10 LSBs for 16-bit (more generous)
    } else if (bit_depth == 24) {
        tolerance = (vv_dsp_real)(10.0 / 16777216.0);  // ~10 LSBs for 24-bit
    } else { // 32-bit
        tolerance = (vv_dsp_real)(10.0 / 4294967296.0);  // ~10 LSBs for 32-bit
    }
    
    int buffers_match = compare_buffers(original_buffers, read_buffers, num_channels, TEST_DURATION_SAMPLES, tolerance);
    
    // Cleanup
    for (int ch = 0; ch < num_channels; ch++) {
        free(original_buffers[ch]);
    }
    free(original_buffers);
    
    vv_dsp_wav_free_buffer(&read_buffers, num_channels);
    
    // Remove temp file
    remove(temp_filename);
    
    if (!buffers_match) {
        printf("ERROR: Audio data mismatch\n");
        return 0;
    }
    
    printf("SUCCESS: Roundtrip test passed\n");
    return 1;
}

int main(void) {
    printf("Starting WAV audio I/O tests...\n\n");
    
    int tests_passed = 0;
    int total_tests = 0;
    
    // Test invalid file
    total_tests++;
    if (test_wav_info_invalid_file()) {
        tests_passed++;
    }
    
    // Test various formats
    int bit_depths[] = {16, 24, 32};
    int is_floats[] = {0, 1};
    int channel_counts[] = {1, 2};
    
    for (size_t bd = 0; bd < sizeof(bit_depths)/sizeof(bit_depths[0]); bd++) {
        for (size_t fl = 0; fl < sizeof(is_floats)/sizeof(is_floats[0]); fl++) {
            for (size_t ch = 0; ch < sizeof(channel_counts)/sizeof(channel_counts[0]); ch++) {
                int bit_depth = bit_depths[bd];
                int is_float = is_floats[fl];
                int num_channels = channel_counts[ch];
                
                // Skip invalid combinations
                if (is_float && bit_depth != 32) continue;
                
                total_tests++;
                if (test_wav_roundtrip(bit_depth, is_float, num_channels)) {
                    tests_passed++;
                }
            }
        }
    }
    
    printf("\nTest Results: %d/%d tests passed\n", tests_passed, total_tests);
    
    if (tests_passed == total_tests) {
        printf("All tests PASSED!\n");
        return 0;
    } else {
        printf("Some tests FAILED!\n");
        return 1;
    }
}
