#include "vv_dsp/audio/wav.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

// Thread-local error storage (fallback to global if not supported)
#if defined(__GNUC__) || defined(__clang__)
  static __thread char s_error_buffer[256] = {0};
#elif defined(_MSC_VER)
  static __declspec(thread) char s_error_buffer[256] = {0};
#else
  static char s_error_buffer[256] = {0};
#endif

// WAV format constants
#define WAV_FOURCC_RIFF  0x46464952  // 'RIFF' in little-endian
#define WAV_FOURCC_WAVE  0x45564157  // 'WAVE' in little-endian
#define WAV_FOURCC_FMT   0x20746D66  // 'fmt ' in little-endian
#define WAV_FOURCC_DATA  0x61746164  // 'data' in little-endian

#define WAV_FORMAT_PCM        1
#define WAV_FORMAT_FLOAT      3
#define WAV_FORMAT_EXTENSIBLE 0xFFFE

// RIFF chunk header
typedef struct {
    uint32_t fourcc;
    uint32_t size;
} wav_chunk_header;

// RIFF file header
typedef struct {
    uint32_t riff_fourcc;    // 'RIFF'
    uint32_t file_size;      // File size - 8
    uint32_t wave_fourcc;    // 'WAVE'
} wav_riff_header;

// WAV format chunk (basic WAVEFORMATEX subset)
typedef struct {
    uint16_t format_tag;        // Format type
    uint16_t channels;          // Number of channels
    uint32_t sample_rate;       // Sample rate
    uint32_t byte_rate;         // Bytes per second
    uint16_t block_align;       // Block alignment
    uint16_t bits_per_sample;   // Bits per sample
} wav_format_chunk;

// Forward declarations for internal functions
static vv_dsp_status wav_parse_header(FILE* fp, vv_dsp_wav_info* info);
static vv_dsp_status wav_write_header(FILE* fp, const vv_dsp_wav_info* info);
static vv_dsp_status wav_read_samples(FILE* fp, const vv_dsp_wav_info* info, vv_dsp_real*** buffer);
static vv_dsp_status wav_write_samples(FILE* fp, const vv_dsp_wav_info* info, const vv_dsp_real* const* buffer);
static void set_error(const char* msg);
static vv_dsp_status skip_chunk(FILE* fp, uint32_t size);
static vv_dsp_status find_chunk(FILE* fp, uint32_t target_fourcc, uint32_t* out_size);
static vv_dsp_status deinterleave_float32(const uint8_t* interleaved, vv_dsp_real** planar, const vv_dsp_wav_info* info);
static vv_dsp_status deinterleave_pcm16(const uint8_t* interleaved, vv_dsp_real** planar, const vv_dsp_wav_info* info);
static vv_dsp_status deinterleave_pcm24(const uint8_t* interleaved, vv_dsp_real** planar, const vv_dsp_wav_info* info);
static vv_dsp_status deinterleave_pcm32(const uint8_t* interleaved, vv_dsp_real** planar, const vv_dsp_wav_info* info);
static vv_dsp_status interleave_float32(const vv_dsp_real* const* planar, uint8_t* interleaved, const vv_dsp_wav_info* info);
static vv_dsp_status interleave_pcm16(const vv_dsp_real* const* planar, uint8_t* interleaved, const vv_dsp_wav_info* info);
static vv_dsp_status interleave_pcm24(const vv_dsp_real* const* planar, uint8_t* interleaved, const vv_dsp_wav_info* info);
static vv_dsp_status interleave_pcm32(const vv_dsp_real* const* planar, uint8_t* interleaved, const vv_dsp_wav_info* info);

vv_dsp_status vv_dsp_wav_read(const char* filepath, 
                              vv_dsp_real*** out_buffer, 
                              vv_dsp_wav_info* out_info) {
    if (!filepath || !out_buffer || !out_info) {
        set_error("NULL pointer passed to vv_dsp_wav_read");
        return VV_DSP_ERROR_NULL_POINTER;
    }

    *out_buffer = NULL;
    memset(out_info, 0, sizeof(*out_info));

    FILE* fp = fopen(filepath, "rb");
    if (!fp) {
        set_error("Failed to open file for reading");
        return VV_DSP_ERROR_INTERNAL;
    }

    vv_dsp_status status = wav_parse_header(fp, out_info);
    if (status != VV_DSP_OK) {
        fclose(fp);
        return status;
    }

    status = wav_read_samples(fp, out_info, out_buffer);
    fclose(fp);

    if (status != VV_DSP_OK && *out_buffer) {
        vv_dsp_wav_free_buffer(out_buffer, out_info->num_channels);
    }

    return status;
}

vv_dsp_status vv_dsp_wav_write(const char* filepath,
                               const vv_dsp_real* const* in_buffer,
                               const vv_dsp_wav_info* info) {
    if (!filepath || !in_buffer || !info) {
        set_error("NULL pointer passed to vv_dsp_wav_write");
        return VV_DSP_ERROR_NULL_POINTER;
    }

    if (info->num_channels <= 0 || info->num_samples == 0 || info->sample_rate <= 0) {
        set_error("Invalid WAV parameters");
        return VV_DSP_ERROR_INVALID_SIZE;
    }

    FILE* fp = fopen(filepath, "wb");
    if (!fp) {
        set_error("Failed to open file for writing");
        return VV_DSP_ERROR_INTERNAL;
    }

    vv_dsp_status status = wav_write_header(fp, info);
    if (status != VV_DSP_OK) {
        fclose(fp);
        return status;
    }

    status = wav_write_samples(fp, info, in_buffer);
    fclose(fp);

    return status;
}

vv_dsp_status vv_dsp_wav_info_get(const char* filepath, 
                                  vv_dsp_wav_info* out_info) {
    if (!filepath || !out_info) {
        set_error("NULL pointer passed to vv_dsp_wav_info_get");
        return VV_DSP_ERROR_NULL_POINTER;
    }

    memset(out_info, 0, sizeof(*out_info));

    FILE* fp = fopen(filepath, "rb");
    if (!fp) {
        set_error("Failed to open file for reading");
        return VV_DSP_ERROR_INTERNAL;
    }

    vv_dsp_status status = wav_parse_header(fp, out_info);
    fclose(fp);

    return status;
}

void vv_dsp_wav_free_buffer(vv_dsp_real*** buffer, int num_channels) {
    if (!buffer || !*buffer) return;
    
    for (int ch = 0; ch < num_channels; ch++) {
        free((*buffer)[ch]);
    }
    free(*buffer);
    *buffer = NULL;
}

const char* vv_dsp_wav_get_error_string(void) {
    return s_error_buffer[0] ? s_error_buffer : "No error";
}

// Internal helper functions (stubs for now)
static vv_dsp_status wav_parse_header(FILE* fp, vv_dsp_wav_info* info) {
    wav_riff_header riff_header;
    wav_format_chunk fmt_chunk;
    uint32_t data_size = 0;
    
    // Read RIFF header
    if (fread(&riff_header, sizeof(riff_header), 1, fp) != 1) {
        set_error("Failed to read RIFF header");
        return VV_DSP_ERROR_INTERNAL;
    }
    
    // Validate RIFF header
    if (riff_header.riff_fourcc != WAV_FOURCC_RIFF) {
        set_error("File is not a RIFF file");
        return VV_DSP_ERROR_INVALID_SIZE;
    }
    
    if (riff_header.wave_fourcc != WAV_FOURCC_WAVE) {
        set_error("File is not a WAVE file");
        return VV_DSP_ERROR_INVALID_SIZE;
    }
    
    // Find fmt chunk
    uint32_t fmt_size;
    vv_dsp_status status = find_chunk(fp, WAV_FOURCC_FMT, &fmt_size);
    if (status != VV_DSP_OK) {
        set_error("fmt chunk not found");
        return status;
    }
    
    if (fmt_size < sizeof(wav_format_chunk)) {
        set_error("fmt chunk too small");
        return VV_DSP_ERROR_INVALID_SIZE;
    }
    
    // Read format chunk
    if (fread(&fmt_chunk, sizeof(fmt_chunk), 1, fp) != 1) {
        set_error("Failed to read format chunk");
        return VV_DSP_ERROR_INTERNAL;
    }
    
    // Skip extra format bytes if present
    if (fmt_size > sizeof(wav_format_chunk)) {
        status = skip_chunk(fp, fmt_size - sizeof(wav_format_chunk));
        if (status != VV_DSP_OK) return status;
    }
    
    // Validate format
    if (fmt_chunk.format_tag != WAV_FORMAT_PCM && fmt_chunk.format_tag != WAV_FORMAT_FLOAT) {
        set_error("Unsupported WAV format (only PCM and float supported)");
        return VV_DSP_ERROR_INVALID_SIZE;
    }
    
    if (fmt_chunk.channels == 0 || fmt_chunk.channels > 8) {
        set_error("Invalid number of channels");
        return VV_DSP_ERROR_INVALID_SIZE;
    }
    
    if (fmt_chunk.bits_per_sample != 16 && fmt_chunk.bits_per_sample != 24 && fmt_chunk.bits_per_sample != 32) {
        set_error("Unsupported bit depth (only 16, 24, and 32 bits supported)");
        return VV_DSP_ERROR_INVALID_SIZE;
    }
    
    // Find data chunk
    status = find_chunk(fp, WAV_FOURCC_DATA, &data_size);
    if (status != VV_DSP_OK) {
        set_error("data chunk not found");
        return status;
    }
    
    // Calculate number of samples
    uint32_t bytes_per_sample = fmt_chunk.bits_per_sample / 8;
    uint32_t total_samples = data_size / (bytes_per_sample * fmt_chunk.channels);
    
    // Fill output info structure
    info->num_samples = total_samples;
    info->num_channels = fmt_chunk.channels;
    info->sample_rate = fmt_chunk.sample_rate;
    info->bit_depth = fmt_chunk.bits_per_sample;
    info->is_float = (fmt_chunk.format_tag == WAV_FORMAT_FLOAT) ? 1 : 0;
    
    return VV_DSP_OK;
}

static vv_dsp_status wav_write_header(FILE* fp, const vv_dsp_wav_info* info) {
    wav_riff_header riff_header;
    wav_chunk_header fmt_chunk_header;
    wav_format_chunk fmt_chunk;
    wav_chunk_header data_chunk_header;
    
    // Calculate sizes
    uint32_t bytes_per_sample = (uint32_t)(info->bit_depth / 8);
    uint32_t data_size = (uint32_t)(info->num_samples * (size_t)info->num_channels * bytes_per_sample);
    uint32_t file_size = sizeof(wav_riff_header) - 8 + sizeof(wav_chunk_header) + sizeof(wav_format_chunk) + sizeof(wav_chunk_header) + data_size;
    
    // Write RIFF header
    riff_header.riff_fourcc = WAV_FOURCC_RIFF;
    riff_header.file_size = file_size;
    riff_header.wave_fourcc = WAV_FOURCC_WAVE;
    
    if (fwrite(&riff_header, sizeof(riff_header), 1, fp) != 1) {
        set_error("Failed to write RIFF header");
        return VV_DSP_ERROR_INTERNAL;
    }
    
    // Write fmt chunk header
    fmt_chunk_header.fourcc = WAV_FOURCC_FMT;
    fmt_chunk_header.size = sizeof(wav_format_chunk);
    
    if (fwrite(&fmt_chunk_header, sizeof(fmt_chunk_header), 1, fp) != 1) {
        set_error("Failed to write fmt chunk header");
        return VV_DSP_ERROR_INTERNAL;
    }
    
    // Write fmt chunk data
    fmt_chunk.format_tag = info->is_float ? WAV_FORMAT_FLOAT : WAV_FORMAT_PCM;
    fmt_chunk.channels = (uint16_t)info->num_channels;
    fmt_chunk.sample_rate = (uint32_t)info->sample_rate;
    fmt_chunk.byte_rate = (uint32_t)(info->sample_rate * (double)info->num_channels * (double)bytes_per_sample);
    fmt_chunk.block_align = (uint16_t)((uint32_t)info->num_channels * bytes_per_sample);
    fmt_chunk.bits_per_sample = (uint16_t)info->bit_depth;
    
    if (fwrite(&fmt_chunk, sizeof(fmt_chunk), 1, fp) != 1) {
        set_error("Failed to write fmt chunk data");
        return VV_DSP_ERROR_INTERNAL;
    }
    
    // Write data chunk header
    data_chunk_header.fourcc = WAV_FOURCC_DATA;
    data_chunk_header.size = data_size;
    
    if (fwrite(&data_chunk_header, sizeof(data_chunk_header), 1, fp) != 1) {
        set_error("Failed to write data chunk header");
        return VV_DSP_ERROR_INTERNAL;
    }
    
    return VV_DSP_OK;
}

static vv_dsp_status wav_read_samples(FILE* fp, const vv_dsp_wav_info* info, vv_dsp_real*** buffer) {
    // Allocate planar buffer array
    vv_dsp_real** planar_buffer = (vv_dsp_real**)malloc(sizeof(vv_dsp_real*) * (size_t)info->num_channels);
    if (!planar_buffer) {
        set_error("Failed to allocate channel buffer array");
        return VV_DSP_ERROR_INTERNAL;
    }
    
    // Initialize channel buffer pointers to NULL for cleanup
    for (int ch = 0; ch < info->num_channels; ch++) {
        planar_buffer[ch] = NULL;
    }
    
    // Allocate individual channel buffers
    for (int ch = 0; ch < info->num_channels; ch++) {
        planar_buffer[ch] = (vv_dsp_real*)malloc(sizeof(vv_dsp_real) * info->num_samples);
        if (!planar_buffer[ch]) {
            set_error("Failed to allocate channel buffer");
            vv_dsp_wav_free_buffer(&planar_buffer, info->num_channels);
            return VV_DSP_ERROR_INTERNAL;
        }
    }
    
    // Calculate bytes per sample
    int bytes_per_sample = info->bit_depth / 8;
    size_t interleaved_buffer_size = info->num_samples * (size_t)info->num_channels * (size_t)bytes_per_sample;
    
    // Allocate temporary interleaved buffer
    uint8_t* interleaved_buffer = (uint8_t*)malloc(interleaved_buffer_size);
    if (!interleaved_buffer) {
        set_error("Failed to allocate interleaved buffer");
        vv_dsp_wav_free_buffer(&planar_buffer, info->num_channels);
        return VV_DSP_ERROR_INTERNAL;
    }
    
    // Read all interleaved data
    if (fread(interleaved_buffer, 1, interleaved_buffer_size, fp) != interleaved_buffer_size) {
        set_error("Failed to read audio data");
        free(interleaved_buffer);
        vv_dsp_wav_free_buffer(&planar_buffer, info->num_channels);
        return VV_DSP_ERROR_INTERNAL;
    }
    
    // Deinterleave and convert samples
    vv_dsp_status status = VV_DSP_OK;
    if (info->is_float && info->bit_depth == 32) {
        status = deinterleave_float32(interleaved_buffer, planar_buffer, info);
    } else if (info->bit_depth == 16) {
        status = deinterleave_pcm16(interleaved_buffer, planar_buffer, info);
    } else if (info->bit_depth == 24) {
        status = deinterleave_pcm24(interleaved_buffer, planar_buffer, info);
    } else if (info->bit_depth == 32) {
        status = deinterleave_pcm32(interleaved_buffer, planar_buffer, info);
    } else {
        set_error("Unsupported bit depth");
        status = VV_DSP_ERROR_INVALID_SIZE;
    }
    
    free(interleaved_buffer);
    
    if (status != VV_DSP_OK) {
        vv_dsp_wav_free_buffer(&planar_buffer, info->num_channels);
        return status;
    }
    
    *buffer = planar_buffer;
    return VV_DSP_OK;
}

static vv_dsp_status wav_write_samples(FILE* fp, const vv_dsp_wav_info* info, const vv_dsp_real* const* buffer) {
    // Calculate buffer size
    int bytes_per_sample = info->bit_depth / 8;
    size_t interleaved_buffer_size = info->num_samples * (size_t)info->num_channels * (size_t)bytes_per_sample;
    
    // Allocate temporary interleaved buffer
    uint8_t* interleaved_buffer = (uint8_t*)malloc(interleaved_buffer_size);
    if (!interleaved_buffer) {
        set_error("Failed to allocate interleaved buffer");
        return VV_DSP_ERROR_INTERNAL;
    }
    
    // Interleave and convert samples
    vv_dsp_status status = VV_DSP_OK;
    if (info->is_float && info->bit_depth == 32) {
        status = interleave_float32(buffer, interleaved_buffer, info);
    } else if (info->bit_depth == 16) {
        status = interleave_pcm16(buffer, interleaved_buffer, info);
    } else if (info->bit_depth == 24) {
        status = interleave_pcm24(buffer, interleaved_buffer, info);
    } else if (info->bit_depth == 32) {
        status = interleave_pcm32(buffer, interleaved_buffer, info);
    } else {
        set_error("Unsupported bit depth");
        status = VV_DSP_ERROR_INVALID_SIZE;
    }
    
    if (status != VV_DSP_OK) {
        free(interleaved_buffer);
        return status;
    }
    
    // Write interleaved data to file
    if (fwrite(interleaved_buffer, 1, interleaved_buffer_size, fp) != interleaved_buffer_size) {
        set_error("Failed to write audio data");
        free(interleaved_buffer);
        return VV_DSP_ERROR_INTERNAL;
    }
    
    free(interleaved_buffer);
    return VV_DSP_OK;
}

static void set_error(const char* msg) {
    if (msg) {
        strncpy(s_error_buffer, msg, sizeof(s_error_buffer) - 1);
        s_error_buffer[sizeof(s_error_buffer) - 1] = '\0';
    }
}

static vv_dsp_status skip_chunk(FILE* fp, uint32_t size) {
    if (fseek(fp, (long)size, SEEK_CUR) != 0) {
        set_error("Failed to skip chunk data");
        return VV_DSP_ERROR_INTERNAL;
    }
    return VV_DSP_OK;
}

static vv_dsp_status find_chunk(FILE* fp, uint32_t target_fourcc, uint32_t* out_size) {
    wav_chunk_header header;
    
    while (fread(&header, sizeof(header), 1, fp) == 1) {
        if (header.fourcc == target_fourcc) {
            *out_size = header.size;
            return VV_DSP_OK;
        }
        
        // Skip this chunk
        vv_dsp_status status = skip_chunk(fp, header.size);
        if (status != VV_DSP_OK) return status;
        
        // Pad to even boundary if needed
        if (header.size & 1) {
            if (fseek(fp, 1, SEEK_CUR) != 0) {
                set_error("Failed to skip padding byte");
                return VV_DSP_ERROR_INTERNAL;
            }
        }
    }
    
    set_error("Target chunk not found");
    return VV_DSP_ERROR_INTERNAL;
}

static vv_dsp_status deinterleave_float32(const uint8_t* interleaved, vv_dsp_real** planar, const vv_dsp_wav_info* info) {
    const float* float_data = (const float*)interleaved;
    
    for (size_t sample = 0; sample < info->num_samples; sample++) {
        for (int ch = 0; ch < info->num_channels; ch++) {
            size_t interleaved_idx = sample * (size_t)info->num_channels + (size_t)ch;
            planar[ch][sample] = (vv_dsp_real)float_data[interleaved_idx];
        }
    }
    
    return VV_DSP_OK;
}

static vv_dsp_status deinterleave_pcm16(const uint8_t* interleaved, vv_dsp_real** planar, const vv_dsp_wav_info* info) {
    const int16_t* pcm_data = (const int16_t*)interleaved;
    const vv_dsp_real scale = (vv_dsp_real)(1.0 / 32768.0); // Scale from [-32768, 32767] to [-1.0, 1.0)
    
    for (size_t sample = 0; sample < info->num_samples; sample++) {
        for (int ch = 0; ch < info->num_channels; ch++) {
            size_t interleaved_idx = sample * (size_t)info->num_channels + (size_t)ch;
            planar[ch][sample] = (vv_dsp_real)pcm_data[interleaved_idx] * scale;
        }
    }
    
    return VV_DSP_OK;
}

static vv_dsp_status deinterleave_pcm24(const uint8_t* interleaved, vv_dsp_real** planar, const vv_dsp_wav_info* info) {
    const vv_dsp_real scale = (vv_dsp_real)(1.0 / 8388608.0); // Scale from [-8388608, 8388607] to [-1.0, 1.0)
    
    for (size_t sample = 0; sample < info->num_samples; sample++) {
        for (int ch = 0; ch < info->num_channels; ch++) {
            size_t byte_idx = (sample * (size_t)info->num_channels + (size_t)ch) * 3;
            
            // Read 24-bit little-endian signed value
            int32_t pcm_value = (int32_t)interleaved[byte_idx] |
                               ((int32_t)interleaved[byte_idx + 1] << 8) |
                               ((int32_t)interleaved[byte_idx + 2] << 16);
            
            // Sign extend from 24-bit to 32-bit
            if (pcm_value & 0x800000) {
                pcm_value |= 0xFF000000;
            }
            
            planar[ch][sample] = (vv_dsp_real)pcm_value * scale;
        }
    }
    
    return VV_DSP_OK;
}

static vv_dsp_status deinterleave_pcm32(const uint8_t* interleaved, vv_dsp_real** planar, const vv_dsp_wav_info* info) {
    const int32_t* pcm_data = (const int32_t*)interleaved;
    const vv_dsp_real scale = (vv_dsp_real)(1.0 / 2147483648.0); // Scale from [-2^31, 2^31-1] to [-1.0, 1.0)
    
    for (size_t sample = 0; sample < info->num_samples; sample++) {
        for (int ch = 0; ch < info->num_channels; ch++) {
            size_t interleaved_idx = sample * (size_t)info->num_channels + (size_t)ch;
            planar[ch][sample] = (vv_dsp_real)pcm_data[interleaved_idx] * scale;
        }
    }
    
    return VV_DSP_OK;
}

static vv_dsp_status interleave_float32(const vv_dsp_real* const* planar, uint8_t* interleaved, const vv_dsp_wav_info* info) {
    float* float_data = (float*)interleaved;
    
    for (size_t sample = 0; sample < info->num_samples; sample++) {
        for (int ch = 0; ch < info->num_channels; ch++) {
            size_t interleaved_idx = sample * (size_t)info->num_channels + (size_t)ch;
            float_data[interleaved_idx] = (float)planar[ch][sample];
        }
    }
    
    return VV_DSP_OK;
}

static vv_dsp_status interleave_pcm16(const vv_dsp_real* const* planar, uint8_t* interleaved, const vv_dsp_wav_info* info) {
    int16_t* pcm_data = (int16_t*)interleaved;
    
    for (size_t sample = 0; sample < info->num_samples; sample++) {
        for (int ch = 0; ch < info->num_channels; ch++) {
            size_t interleaved_idx = sample * (size_t)info->num_channels + (size_t)ch;
            
            // Clamp and scale from [-1.0, 1.0] to [-32768, 32767]
            vv_dsp_real clamped = planar[ch][sample];
            if (clamped > (vv_dsp_real)1.0) clamped = (vv_dsp_real)1.0;
            if (clamped < (vv_dsp_real)-1.0) clamped = (vv_dsp_real)-1.0;
            
            int32_t scaled = (int32_t)(clamped * (vv_dsp_real)32767.0);
            if (scaled > 32767) scaled = 32767;
            if (scaled < -32768) scaled = -32768;
            
            pcm_data[interleaved_idx] = (int16_t)scaled;
        }
    }
    
    return VV_DSP_OK;
}

static vv_dsp_status interleave_pcm24(const vv_dsp_real* const* planar, uint8_t* interleaved, const vv_dsp_wav_info* info) {
    for (size_t sample = 0; sample < info->num_samples; sample++) {
        for (int ch = 0; ch < info->num_channels; ch++) {
            size_t byte_idx = (sample * (size_t)info->num_channels + (size_t)ch) * 3;
            
            // Clamp and scale from [-1.0, 1.0] to [-8388608, 8388607]
            vv_dsp_real clamped = planar[ch][sample];
            if (clamped > (vv_dsp_real)1.0) clamped = (vv_dsp_real)1.0;
            if (clamped < (vv_dsp_real)-1.0) clamped = (vv_dsp_real)-1.0;
            
            int32_t scaled = (int32_t)(clamped * (vv_dsp_real)8388607.0);
            if (scaled > 8388607) scaled = 8388607;
            if (scaled < -8388608) scaled = -8388608;
            
            // Write 24-bit little-endian value
            interleaved[byte_idx] = (uint8_t)(scaled & 0xFF);
            interleaved[byte_idx + 1] = (uint8_t)((scaled >> 8) & 0xFF);
            interleaved[byte_idx + 2] = (uint8_t)((scaled >> 16) & 0xFF);
        }
    }
    
    return VV_DSP_OK;
}

static vv_dsp_status interleave_pcm32(const vv_dsp_real* const* planar, uint8_t* interleaved, const vv_dsp_wav_info* info) {
    int32_t* pcm_data = (int32_t*)interleaved;
    
    for (size_t sample = 0; sample < info->num_samples; sample++) {
        for (int ch = 0; ch < info->num_channels; ch++) {
            size_t interleaved_idx = sample * (size_t)info->num_channels + (size_t)ch;
            
            // Clamp and scale from [-1.0, 1.0] to [-2^31, 2^31-1]
            vv_dsp_real clamped = planar[ch][sample];
            if (clamped > (vv_dsp_real)1.0) clamped = (vv_dsp_real)1.0;
            if (clamped < (vv_dsp_real)-1.0) clamped = (vv_dsp_real)-1.0;
            
            int64_t scaled = (int64_t)(clamped * (vv_dsp_real)2147483647.0);
            if (scaled > 2147483647LL) scaled = 2147483647LL;
            if (scaled < -2147483648LL) scaled = -2147483648LL;
            
            pcm_data[interleaved_idx] = (int32_t)scaled;
        }
    }
    
    return VV_DSP_OK;
}
