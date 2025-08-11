/**
 * @file framing.c
 * @brief Signal framing and overlap-add utilities implementation
 * @ingroup core_group
 *
 * Implementation of signal framing functions for STFT and real-time audio processing.
 */

#include "vv_dsp/core.h"
#include "vv_dsp/vv_dsp_types.h"
#include <string.h>

// Helper function to clamp a value between bounds
static VV_DSP_INLINE long clamp_long(long value, long min_val, long max_val) {
    if (value < min_val) return min_val;
    if (value > max_val) return max_val;
    return value;
}

// Helper function for reflection padding index calculation
static VV_DSP_INLINE size_t reflect_index(long idx, size_t signal_len) {
    if (signal_len == 0) return 0;

    if (idx < 0) {
        // Reflect from the beginning
        long abs_idx = -idx - 1;
        if (abs_idx >= (long)signal_len) {
            // Handle multiple reflections by using modulo
            long period = 2 * (long)signal_len;
            abs_idx = abs_idx % period;
            if (abs_idx >= (long)signal_len) {
                abs_idx = period - 1 - abs_idx;
            }
        }
        return (size_t)abs_idx;
    } else if (idx >= (long)signal_len) {
        // Reflect from the end
        long overflow = idx - (long)signal_len;
        long reflected = (long)signal_len - 1 - overflow;

        if (reflected < 0) {
            // Handle multiple reflections
            reflected = -reflected - 1;
            if (reflected >= (long)signal_len) {
                long period = 2 * (long)signal_len;
                reflected = reflected % period;
                if (reflected >= (long)signal_len) {
                    reflected = period - 1 - reflected;
                }
            }
        }
        return (size_t)clamp_long(reflected, 0, (long)signal_len - 1);
    } else {
        return (size_t)idx;
    }
}

size_t vv_dsp_get_num_frames(size_t signal_len, size_t frame_len, size_t hop_len, int center) {
    if (hop_len == 0) return 0;

    if (center != 0) {
        // Centered framing: signal_len / hop_len
        return (signal_len + hop_len - 1) / hop_len;  // Ceiling division
    } else {
        // Non-centered framing: 1 + (signal_len - frame_len) / hop_len
        if (signal_len < frame_len) return 0;
        return 1 + (signal_len - frame_len) / hop_len;
    }
}

vv_dsp_status vv_dsp_fetch_frame(const vv_dsp_real* signal, size_t signal_len,
                                 vv_dsp_real* frame_buffer, size_t frame_len,
                                 size_t hop_len, size_t frame_index, int center,
                                 const vv_dsp_real* window) {
    // Input validation
    if (!signal || !frame_buffer) {
        return VV_DSP_ERROR_NULL_POINTER;
    }

    if (signal_len == 0 || frame_len == 0 || hop_len == 0) {
        return VV_DSP_ERROR_INVALID_SIZE;
    }

    // Calculate frame start position
    long frame_start;
    if (center != 0) {
        // Centered framing: frame is centered at frame_index * hop_len
        frame_start = (long)(frame_index * hop_len) - (long)(frame_len / 2);
    } else {
        // Non-centered framing: frame starts at frame_index * hop_len
        frame_start = (long)(frame_index * hop_len);
    }

    // Fill frame buffer
    for (size_t i = 0; i < frame_len; i++) {
        long sample_idx = frame_start + (long)i;
        vv_dsp_real sample_value;

        if (center != 0) {
            // Use reflection padding for centered framing
            size_t reflected_idx = reflect_index(sample_idx, signal_len);
            sample_value = signal[reflected_idx];
        } else {
            // For non-centered framing, use zero padding outside signal bounds
            if (sample_idx < 0 || sample_idx >= (long)signal_len) {
                sample_value = 0.0;
            } else {
                sample_value = signal[sample_idx];
            }
        }

        // Apply window if provided
        if (window != NULL) {
            frame_buffer[i] = sample_value * window[i];
        } else {
            frame_buffer[i] = sample_value;
        }
    }

    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_overlap_add(const vv_dsp_real* frame, vv_dsp_real* output_signal,
                                 size_t output_len, size_t frame_len, size_t hop_len,
                                 size_t frame_index) {
    // Input validation
    if (!frame || !output_signal) {
        return VV_DSP_ERROR_NULL_POINTER;
    }

    if (output_len == 0 || frame_len == 0 || hop_len == 0) {
        return VV_DSP_ERROR_INVALID_SIZE;
    }

    // Calculate starting position in output signal
    size_t start_pos = frame_index * hop_len;

    // Add frame samples to output signal with bounds checking
    for (size_t i = 0; i < frame_len; i++) {
        size_t output_idx = start_pos + i;
        if (output_idx < output_len) {
            output_signal[output_idx] += frame[i];
        }
        // If output_idx >= output_len, silently ignore (bounds protection)
    }

    return VV_DSP_OK;
}
