/*
 * vv-dsp WAV Audio File I/O utilities
 */
#ifndef VV_DSP_AUDIO_WAV_H
#define VV_DSP_AUDIO_WAV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vv_dsp/vv_dsp_types.h"
#include <stddef.h>

/**
 * WAV audio file metadata structure
 */
typedef struct vv_dsp_wav_info {
    size_t num_samples;     ///< Total number of samples per channel
    int num_channels;       ///< Number of channels (1=mono, 2=stereo, etc.)
    double sample_rate;     ///< Sample rate in Hz
    int bit_depth;          ///< Bit depth (16, 24, or 32)
    int is_float;           ///< 1 if 32-bit float format, 0 if integer PCM
} vv_dsp_wav_info;

/**
 * Read a WAV file into planar vv_dsp_real buffers.
 * 
 * @param filepath Path to the WAV file to read
 * @param out_buffer Pointer to array of channel buffers (allocated by this function)
 * @param out_info Pointer to structure that will be filled with file metadata
 * @return VV_DSP_OK on success, error code on failure
 * 
 * Note: The caller is responsible for freeing the allocated buffer using vv_dsp_wav_free_buffer()
 */
VV_DSP_NODISCARD
vv_dsp_status vv_dsp_wav_read(const char* filepath, 
                              vv_dsp_real*** out_buffer, 
                              vv_dsp_wav_info* out_info);

/**
 * Write planar vv_dsp_real buffers to a WAV file.
 * 
 * @param filepath Path to the WAV file to write
 * @param in_buffer Array of channel buffers (planar format)
 * @param info Metadata for the output file
 * @return VV_DSP_OK on success, error code on failure
 */
VV_DSP_NODISCARD
vv_dsp_status vv_dsp_wav_write(const char* filepath,
                               const vv_dsp_real* const* in_buffer,
                               const vv_dsp_wav_info* info);

/**
 * Get information about a WAV file without reading the audio data.
 * 
 * @param filepath Path to the WAV file to inspect
 * @param out_info Pointer to structure that will be filled with file metadata
 * @return VV_DSP_OK on success, error code on failure
 */
VV_DSP_NODISCARD
vv_dsp_status vv_dsp_wav_info_get(const char* filepath, 
                                  vv_dsp_wav_info* out_info);

/**
 * Free buffer allocated by vv_dsp_wav_read().
 * 
 * @param buffer Buffer to free (can be NULL)
 * @param num_channels Number of channels in the buffer
 */
void vv_dsp_wav_free_buffer(vv_dsp_real*** buffer, int num_channels);

/**
 * Get human-readable error message for the last WAV I/O operation.
 * This is a thread-local error message that gets updated on each operation.
 * 
 * @return Pointer to error message string (valid until next WAV operation)
 */
const char* vv_dsp_wav_get_error_string(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // VV_DSP_AUDIO_WAV_H
