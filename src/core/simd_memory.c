/**
 * @file simd_memory.c
 * @brief Cross-platform aligned memory allocation for SIMD operations
 * @ingroup core
 */

#include "vv_dsp/core/simd_utils.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Platform-specific includes for aligned allocation */
#ifdef _WIN32
    #include <malloc.h>
#elif defined(__APPLE__)
    #include <sys/malloc.h>
#else
    #include <malloc.h>
#endif

/* C11 aligned_alloc availability check */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && \
    !defined(__APPLE__) && !defined(_WIN32)
    #define VV_DSP_HAS_ALIGNED_ALLOC 1
#endif

/**
 * @brief Internal structure for tracking allocated memory metadata
 *
 * We store metadata before the returned pointer to track the original
 * allocation for proper cleanup. This approach works across all platforms.
 */
typedef struct {
    void* original_ptr;     ///< Original unaligned pointer from malloc
    size_t original_size;   ///< Original allocation size
    size_t alignment;       ///< Requested alignment
    uint32_t magic;         ///< Magic number for corruption detection
} vv_dsp_aligned_header;

#define VV_DSP_ALIGNED_MAGIC 0xABCD1234

void* vv_dsp_aligned_malloc(size_t size, size_t alignment) {
    if (size == 0) {
        return NULL;
    }

    /* Validate alignment requirement */
    if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
        /* Alignment must be power of 2 */
        return NULL;
    }

    if (alignment < sizeof(void*)) {
        alignment = sizeof(void*);
    }

#ifdef _WIN32
    /* Use _aligned_malloc on Windows */
    void* ptr = _aligned_malloc(size, alignment);
    if (!ptr) {
        return NULL;
    }

    /* Windows doesn't need header tracking since _aligned_free handles it */
    return ptr;

#elif defined(VV_DSP_HAS_ALIGNED_ALLOC)
    /* Use C11 aligned_alloc where available */
    /* Note: aligned_alloc requires size to be multiple of alignment */
    size_t padded_size = ((size + alignment - 1) / alignment) * alignment;
    return aligned_alloc(alignment, padded_size);

#elif defined(__APPLE__)
    /* Use posix_memalign on Apple platforms */
    void* ptr = NULL;
    if (posix_memalign(&ptr, alignment, size) != 0) {
        return NULL;
    }
    return ptr;

#else
    /* Generic implementation using malloc + manual alignment */

    /* Calculate total allocation size:
     * - Space for requested size
     * - Space for header metadata
     * - Extra space for alignment adjustment (worst case: alignment - 1)
     */
    size_t header_size = sizeof(vv_dsp_aligned_header);
    size_t total_size = size + header_size + alignment - 1;

    /* Allocate raw memory */
    void* raw_ptr = malloc(total_size);
    if (!raw_ptr) {
        return NULL;
    }

    /* Calculate aligned address for user data */
    uintptr_t raw_addr = (uintptr_t)raw_ptr;
    uintptr_t header_addr = raw_addr + sizeof(vv_dsp_aligned_header);
    uintptr_t aligned_addr = (header_addr + alignment - 1) & ~(alignment - 1);
    uintptr_t header_final = aligned_addr - sizeof(vv_dsp_aligned_header);

    /* Store metadata in header */
    vv_dsp_aligned_header* header = (vv_dsp_aligned_header*)header_final;
    header->original_ptr = raw_ptr;
    header->original_size = total_size;
    header->alignment = alignment;
    header->magic = VV_DSP_ALIGNED_MAGIC;

    void* aligned_ptr = (void*)aligned_addr;

    /* Verify alignment */
    assert(vv_dsp_is_aligned(aligned_ptr, alignment));

    return aligned_ptr;
#endif
}

void vv_dsp_aligned_free(void* ptr) {
    if (!ptr) {
        return;
    }

#ifdef _WIN32
    /* Use _aligned_free on Windows */
    _aligned_free(ptr);

#elif defined(VV_DSP_HAS_ALIGNED_ALLOC) || defined(__APPLE__)
    /* Standard free works for aligned_alloc and posix_memalign */
    free(ptr);

#else
    /* Generic implementation - retrieve original pointer from header */

    uintptr_t aligned_addr = (uintptr_t)ptr;
    uintptr_t header_addr = aligned_addr - sizeof(vv_dsp_aligned_header);
    vv_dsp_aligned_header* header = (vv_dsp_aligned_header*)header_addr;

    /* Validate magic number to detect corruption */
    if (header->magic != VV_DSP_ALIGNED_MAGIC) {
        /* Memory corruption detected or invalid pointer */
        assert(0 && "Invalid aligned pointer or memory corruption detected");
        return;
    }

    /* Free the original allocation */
    free(header->original_ptr);
#endif
}

const char* vv_dsp_simd_get_features(void) {
    static const char* features =
#ifdef VV_DSP_SIMD_AVX512
        "AVX512F"
#elif defined(VV_DSP_SIMD_AVX2)
        "AVX2"
#elif defined(VV_DSP_SIMD_SSE41)
        "SSE4.1"
#elif defined(VV_DSP_SIMD_NEON)
        "NEON"
#else
        "Scalar"
#endif
        ;

    return features;
}

/* Unit test function for aligned memory allocator */
#ifdef VV_DSP_ENABLE_TESTS
int vv_dsp_test_aligned_memory(void) {
    const size_t test_sizes[] = {16, 32, 64, 128, 256, 1024, 4096};
    const size_t test_alignments[] = {16, 32, 64, 128};
    const size_t num_sizes = sizeof(test_sizes) / sizeof(test_sizes[0]);
    const size_t num_alignments = sizeof(test_alignments) / sizeof(test_alignments[0]);

    int tests_passed = 0;
    int total_tests = 0;

    for (size_t i = 0; i < num_sizes; i++) {
        for (size_t j = 0; j < num_alignments; j++) {
            size_t size = test_sizes[i];
            size_t alignment = test_alignments[j];

            total_tests++;

            /* Test allocation */
            void* ptr = vv_dsp_aligned_malloc(size, alignment);
            if (!ptr) {
                continue; /* Skip this test if allocation failed */
            }

            /* Test alignment */
            if (!vv_dsp_is_aligned(ptr, alignment)) {
                vv_dsp_aligned_free(ptr);
                continue; /* Alignment test failed */
            }

            /* Test memory access (write/read) */
            memset(ptr, 0xAA, size);
            int write_test_passed = 1;
            unsigned char* bytes = (unsigned char*)ptr;
            for (size_t k = 0; k < size; k++) {
                if (bytes[k] != 0xAA) {
                    write_test_passed = 0;
                    break;
                }
            }

            if (!write_test_passed) {
                vv_dsp_aligned_free(ptr);
                continue; /* Memory access test failed */
            }

            /* Free memory */
            vv_dsp_aligned_free(ptr);

            tests_passed++;
        }
    }

    /* Test edge cases */
    total_tests += 3;

    /* Test NULL handling */
    vv_dsp_aligned_free(NULL); /* Should not crash */
    tests_passed++;

    /* Test zero size */
    void* zero_ptr = vv_dsp_aligned_malloc(0, 16);
    if (zero_ptr == NULL) {
        tests_passed++;
    } else {
        vv_dsp_aligned_free(zero_ptr);
    }

    /* Test invalid alignment (not power of 2) */
    void* invalid_ptr = vv_dsp_aligned_malloc(64, 17); /* 17 is not power of 2 */
    if (invalid_ptr == NULL) {
        tests_passed++;
    } else {
        vv_dsp_aligned_free(invalid_ptr);
    }

    return (tests_passed == total_tests) ? 0 : -1;
}
#endif /* VV_DSP_ENABLE_TESTS */
