/*
This file is part of vv-dsp

FFTW backend implementation for the vv-dsp FFT abstraction layer.
Provides high-performance FFT operations using the FFTW3 library with plan caching.

This backend integrates the FFTW3 library into the vv-dsp FFT system with
sophisticated plan caching to optimize performance for repeated transforms.
*/

#include "fft_backend.h"

#ifdef VV_DSP_BACKEND_FFT_fftw

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <fftw3.h>

// Global FFTW configuration
static vv_dsp_fftw_flag g_fftw_planning_flag = VV_DSP_FFTW_MEASURE;
static pthread_mutex_t g_fftw_mutex = PTHREAD_MUTEX_INITIALIZER;

// Plan cache key structure
typedef struct fftw_cache_key {
    size_t n;
    vv_dsp_fft_type type;
    vv_dsp_fft_dir dir;
    vv_dsp_fftw_flag flag;
} fftw_cache_key;

// Plan cache entry structure
typedef struct fftw_cache_entry {
    fftw_cache_key key;

    // FFTW plans for different precision/types
    union {
        fftwf_plan complex_plan;  // For C2C transforms
        fftwf_plan r2c_plan;      // For R2C transforms
        fftwf_plan c2r_plan;      // For C2R transforms
    } plan;

    // Reference counting for safe deletion
    int ref_count;

    // Linked list pointers for cache management
    struct fftw_cache_entry* next;
    struct fftw_cache_entry* prev;
} fftw_cache_entry;

// Global plan cache
#define FFTW_CACHE_SIZE 64
static fftw_cache_entry* g_plan_cache[FFTW_CACHE_SIZE] = {NULL};
static fftw_cache_entry* g_cache_lru_head = NULL;
static fftw_cache_entry* g_cache_lru_tail = NULL;
static size_t g_cache_count = 0;

// FFTW plan wrapper structure
typedef struct {
    fftw_cache_entry* cache_entry;
    size_t n;
    vv_dsp_fft_type type;
    vv_dsp_fft_dir dir;

    // Input/output buffers (FFTW-aligned)
    void* in_buffer;
    void* out_buffer;
    size_t buffer_size;
} fftw_plan_wrapper;

// Hash function for cache key
static size_t hash_cache_key(const fftw_cache_key* key) {
    size_t hash = key->n;
    hash = hash * 31 + (size_t)key->type;
    hash = hash * 31 + (size_t)key->dir;
    hash = hash * 31 + (size_t)key->flag;
    return hash % FFTW_CACHE_SIZE;
}

// Compare cache keys
static int keys_equal(const fftw_cache_key* a, const fftw_cache_key* b) {
    return (a->n == b->n && a->type == b->type && a->dir == b->dir && a->flag == b->flag);
}

// Move cache entry to front of LRU list
static void move_to_front(fftw_cache_entry* entry) {
    if (entry == g_cache_lru_head) return;

    // Remove from current position
    if (entry->prev) entry->prev->next = entry->next;
    if (entry->next) entry->next->prev = entry->prev;
    if (entry == g_cache_lru_tail) g_cache_lru_tail = entry->prev;

    // Insert at head
    entry->prev = NULL;
    entry->next = g_cache_lru_head;
    if (g_cache_lru_head) g_cache_lru_head->prev = entry;
    g_cache_lru_head = entry;
    if (!g_cache_lru_tail) g_cache_lru_tail = entry;
}

// Remove LRU entry from cache
static void evict_lru_entry(void) {
    if (!g_cache_lru_tail) return;

    fftw_cache_entry* victim = g_cache_lru_tail;
    size_t hash = hash_cache_key(&victim->key);

    // Remove from hash table
    fftw_cache_entry** bucket = &g_plan_cache[hash];
    while (*bucket && *bucket != victim) {
        bucket = &((*bucket)->next);
    }
    if (*bucket) *bucket = victim->next;

    // Remove from LRU list
    if (victim->prev) victim->prev->next = victim->next;
    if (victim->next) victim->next->prev = victim->prev;
    if (victim == g_cache_lru_head) g_cache_lru_head = victim->next;
    if (victim == g_cache_lru_tail) g_cache_lru_tail = victim->prev;

    // Free FFTW plan
    if (victim->key.type == VV_DSP_FFT_C2C) {
        fftwf_destroy_plan(victim->plan.complex_plan);
    } else if (victim->key.type == VV_DSP_FFT_R2C) {
        fftwf_destroy_plan(victim->plan.r2c_plan);
    } else if (victim->key.type == VV_DSP_FFT_C2R) {
        fftwf_destroy_plan(victim->plan.c2r_plan);
    }

    free(victim);
    g_cache_count--;
}

// Find or create cached plan
static fftw_cache_entry* get_cached_plan(const fftw_cache_key* key, void* in_buf, void* out_buf) {
    size_t hash = hash_cache_key(key);

    // Search in cache
    fftw_cache_entry* entry = g_plan_cache[hash];
    while (entry) {
        if (keys_equal(&entry->key, key)) {
            entry->ref_count++;
            move_to_front(entry);
            return entry;
        }
        entry = entry->next;
    }

    // Not found - create new entry
    entry = (fftw_cache_entry*)malloc(sizeof(fftw_cache_entry));
    if (!entry) return NULL;

    entry->key = *key;
    entry->ref_count = 1;
    entry->next = NULL;
    entry->prev = NULL;

    // Convert planning flag to FFTW flag
    unsigned int fftw_flags = FFTW_DESTROY_INPUT;
    switch (key->flag) {
        case VV_DSP_FFTW_ESTIMATE: fftw_flags |= FFTW_ESTIMATE; break;
        case VV_DSP_FFTW_MEASURE:  fftw_flags |= FFTW_MEASURE; break;
        case VV_DSP_FFTW_PATIENT:  fftw_flags |= FFTW_PATIENT; break;
    }

    // Create FFTW plan based on type
    if (key->type == VV_DSP_FFT_C2C) {
        int sign = (key->dir == VV_DSP_FFT_FORWARD) ? FFTW_FORWARD : FFTW_BACKWARD;
        entry->plan.complex_plan = fftwf_plan_dft_1d((int)key->n,
                                                    (fftwf_complex*)in_buf,
                                                    (fftwf_complex*)out_buf,
                                                    sign, fftw_flags);
        if (!entry->plan.complex_plan) {
            free(entry);
            return NULL;
        }
    } else if (key->type == VV_DSP_FFT_R2C) {
        entry->plan.r2c_plan = fftwf_plan_dft_r2c_1d((int)key->n,
                                                     (float*)in_buf,
                                                     (fftwf_complex*)out_buf,
                                                     fftw_flags);
        if (!entry->plan.r2c_plan) {
            free(entry);
            return NULL;
        }
    } else if (key->type == VV_DSP_FFT_C2R) {
        entry->plan.c2r_plan = fftwf_plan_dft_c2r_1d((int)key->n,
                                                     (fftwf_complex*)in_buf,
                                                     (float*)out_buf,
                                                     fftw_flags);
        if (!entry->plan.c2r_plan) {
            free(entry);
            return NULL;
        }
    } else {
        free(entry);
        return NULL;
    }

    // Add to cache
    if (g_cache_count >= FFTW_CACHE_SIZE) {
        evict_lru_entry();
    }

    entry->next = g_plan_cache[hash];
    g_plan_cache[hash] = entry;
    move_to_front(entry);
    g_cache_count++;

    return entry;
}

// Release cached plan reference
static void release_cached_plan(fftw_cache_entry* entry) {
    if (!entry) return;
    entry->ref_count--;
    // Note: We don't immediately delete here - let LRU eviction handle it
}

static vv_dsp_status fftw_make_plan(const struct vv_dsp_fft_plan* spec, void** backend_data) {
    if (!spec || !backend_data) return VV_DSP_ERROR_NULL_POINTER;

    pthread_mutex_lock(&g_fftw_mutex);

    fftw_plan_wrapper* wrapper = (fftw_plan_wrapper*)malloc(sizeof(fftw_plan_wrapper));
    if (!wrapper) {
        pthread_mutex_unlock(&g_fftw_mutex);
        return VV_DSP_ERROR_INTERNAL;
    }

    wrapper->n = spec->n;
    wrapper->type = spec->type;
    wrapper->dir = spec->dir;
    wrapper->cache_entry = NULL;
    wrapper->in_buffer = NULL;
    wrapper->out_buffer = NULL;

    // Calculate buffer sizes and allocate FFTW-aligned memory
    if (spec->type == VV_DSP_FFT_C2C) {
        wrapper->buffer_size = spec->n * sizeof(fftwf_complex);
        wrapper->in_buffer = fftwf_alloc_complex(spec->n);
        wrapper->out_buffer = fftwf_alloc_complex(spec->n);
    } else if (spec->type == VV_DSP_FFT_R2C) {
        wrapper->buffer_size = spec->n * sizeof(float);
        wrapper->in_buffer = fftwf_alloc_real(spec->n);
        wrapper->out_buffer = fftwf_alloc_complex(spec->n/2 + 1);
    } else if (spec->type == VV_DSP_FFT_C2R) {
        wrapper->buffer_size = spec->n * sizeof(float);
        wrapper->in_buffer = fftwf_alloc_complex(spec->n/2 + 1);
        wrapper->out_buffer = fftwf_alloc_real(spec->n);
    } else {
        free(wrapper);
        pthread_mutex_unlock(&g_fftw_mutex);
        return VV_DSP_ERROR_OUT_OF_RANGE;
    }

    if (!wrapper->in_buffer || !wrapper->out_buffer) {
        if (wrapper->in_buffer) fftwf_free(wrapper->in_buffer);
        if (wrapper->out_buffer) fftwf_free(wrapper->out_buffer);
        free(wrapper);
        pthread_mutex_unlock(&g_fftw_mutex);
        return VV_DSP_ERROR_INTERNAL;
    }

    // Get or create cached plan
    fftw_cache_key key = {spec->n, spec->type, spec->dir, g_fftw_planning_flag};
    wrapper->cache_entry = get_cached_plan(&key, wrapper->in_buffer, wrapper->out_buffer);

    if (!wrapper->cache_entry) {
        fftwf_free(wrapper->in_buffer);
        fftwf_free(wrapper->out_buffer);
        free(wrapper);
        pthread_mutex_unlock(&g_fftw_mutex);
        return VV_DSP_ERROR_INTERNAL;
    }

    *backend_data = wrapper;
    pthread_mutex_unlock(&g_fftw_mutex);
    return VV_DSP_OK;
}

static vv_dsp_status fftw_execute(const struct vv_dsp_fft_plan* spec, void* backend_data, const void* in, void* out) {
    if (!spec || !backend_data || !in || !out) return VV_DSP_ERROR_NULL_POINTER;

    fftw_plan_wrapper* wrapper = (fftw_plan_wrapper*)backend_data;
    if (!wrapper->cache_entry) return VV_DSP_ERROR_INTERNAL;

    if (spec->type == VV_DSP_FFT_C2C) {
        // Copy input to FFTW buffer, execute, copy output
        memcpy(wrapper->in_buffer, in, spec->n * sizeof(fftwf_complex));
        fftwf_execute(wrapper->cache_entry->plan.complex_plan);
        memcpy(out, wrapper->out_buffer, spec->n * sizeof(fftwf_complex));

        // FFTW doesn't apply 1/n scaling for backward transforms - apply it ourselves
        if (spec->dir == VV_DSP_FFT_BACKWARD) {
            vv_dsp_cpx* cpx_out = (vv_dsp_cpx*)out;
            vv_dsp_real scale = (vv_dsp_real)1.0 / (vv_dsp_real)spec->n;
            for (size_t i = 0; i < spec->n; ++i) {
                cpx_out[i].re *= scale;
                cpx_out[i].im *= scale;
            }
        }
    } else if (spec->type == VV_DSP_FFT_R2C) {
        // Copy real input, execute R2C, copy complex output
        const vv_dsp_real* real_in = (const vv_dsp_real*)in;
        float* fftw_in = (float*)wrapper->in_buffer;

        for (size_t i = 0; i < spec->n; ++i) {
            fftw_in[i] = (float)real_in[i];
        }

        fftwf_execute(wrapper->cache_entry->plan.r2c_plan);

        vv_dsp_cpx* cpx_out = (vv_dsp_cpx*)out;
        fftwf_complex* fftw_out = (fftwf_complex*)wrapper->out_buffer;
        size_t nh = spec->n/2 + 1;

        for (size_t i = 0; i < nh; ++i) {
            cpx_out[i].re = (vv_dsp_real)fftw_out[i][0];
            cpx_out[i].im = (vv_dsp_real)fftw_out[i][1];
        }
    } else if (spec->type == VV_DSP_FFT_C2R) {
        // Copy Hermitian input, execute C2R, copy real output with scaling
        const vv_dsp_cpx* cpx_in = (const vv_dsp_cpx*)in;
        fftwf_complex* fftw_in = (fftwf_complex*)wrapper->in_buffer;
        size_t nh = spec->n/2 + 1;

        for (size_t i = 0; i < nh; ++i) {
            fftw_in[i][0] = (float)cpx_in[i].re;
            fftw_in[i][1] = (float)cpx_in[i].im;
        }

        fftwf_execute(wrapper->cache_entry->plan.c2r_plan);

        vv_dsp_real* real_out = (vv_dsp_real*)out;
        float* fftw_out = (float*)wrapper->out_buffer;
        vv_dsp_real scale = (vv_dsp_real)1.0 / (vv_dsp_real)spec->n;

        for (size_t i = 0; i < spec->n; ++i) {
            real_out[i] = (vv_dsp_real)fftw_out[i] * scale;
        }
    } else {
        return VV_DSP_ERROR_OUT_OF_RANGE;
    }

    return VV_DSP_OK;
}

static void fftw_free_plan(void* backend_data) {
    if (!backend_data) return;

    pthread_mutex_lock(&g_fftw_mutex);

    fftw_plan_wrapper* wrapper = (fftw_plan_wrapper*)backend_data;

    if (wrapper->cache_entry) {
        release_cached_plan(wrapper->cache_entry);
    }

    if (wrapper->in_buffer) {
        fftwf_free(wrapper->in_buffer);
    }

    if (wrapper->out_buffer) {
        fftwf_free(wrapper->out_buffer);
    }

    free(wrapper);

    pthread_mutex_unlock(&g_fftw_mutex);
}

static int fftw_is_available(void) {
    return 1; // FFTW is available if this file is compiled
}

// FFTW vtable
const vv_dsp_fft_backend_vtable vv_dsp_fft_fftw_vtable = {
    .make_plan = fftw_make_plan,
    .execute = fftw_execute,
    .free_plan = fftw_free_plan,
    .is_available = fftw_is_available,
    .name = "FFTW3"
};

// FFTW-specific configuration functions (defined in fft.c with proper ifdefs)
VV_DSP_NODISCARD vv_dsp_status vv_dsp_fft_set_fftw_flag_impl(vv_dsp_fftw_flag flag) {
    if (flag < VV_DSP_FFTW_ESTIMATE || flag > VV_DSP_FFTW_PATIENT) {
        return VV_DSP_ERROR_OUT_OF_RANGE;
    }

    pthread_mutex_lock(&g_fftw_mutex);
    g_fftw_planning_flag = flag;
    pthread_mutex_unlock(&g_fftw_mutex);

    return VV_DSP_OK;
}

VV_DSP_NODISCARD vv_dsp_status vv_dsp_fft_flush_fftw_cache_impl(void) {
    pthread_mutex_lock(&g_fftw_mutex);

    // Clear all cache entries
    for (size_t i = 0; i < FFTW_CACHE_SIZE; ++i) {
        fftw_cache_entry* entry = g_plan_cache[i];
        while (entry) {
            fftw_cache_entry* next = entry->next;

            // Free FFTW plan
            if (entry->key.type == VV_DSP_FFT_C2C) {
                fftwf_destroy_plan(entry->plan.complex_plan);
            } else if (entry->key.type == VV_DSP_FFT_R2C) {
                fftwf_destroy_plan(entry->plan.r2c_plan);
            } else if (entry->key.type == VV_DSP_FFT_C2R) {
                fftwf_destroy_plan(entry->plan.c2r_plan);
            }

            free(entry);
            entry = next;
        }
        g_plan_cache[i] = NULL;
    }

    g_cache_lru_head = NULL;
    g_cache_lru_tail = NULL;
    g_cache_count = 0;

    // Also cleanup FFTW wisdom
    fftwf_cleanup();

    pthread_mutex_unlock(&g_fftw_mutex);

    return VV_DSP_OK;
}

#else

// FFTW not available - provide dummy vtable
static int fftw_not_available(void) {
    return 0;
}

const vv_dsp_fft_backend_vtable vv_dsp_fft_fftw_vtable = {
    .make_plan = NULL,
    .execute = NULL,
    .free_plan = NULL,
    .is_available = fftw_not_available,
    .name = "FFTW3 (not available)"
};

#endif // VV_DSP_BACKEND_FFT_fftw
