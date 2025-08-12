/**
 * @file test_simd_memory.c
 * @brief Test program for SIMD aligned memory allocation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "vv_dsp/core/simd_utils.h"

int main(void) {
    printf("Testing SIMD Aligned Memory Allocation\n");
    printf("=====================================\n");

    /* Test SIMD feature detection */
    printf("SIMD Features: %s\n", vv_dsp_simd_get_features());
    printf("Default SIMD alignment: %zu bytes\n", VV_DSP_SIMD_ALIGN_DEFAULT);
    printf("SIMD vector width: %d elements\n", VV_DSP_SIMD_WIDTH);
    printf("\n");

    /* Test different alignment requirements */
    const size_t test_sizes[] = {16, 64, 256, 1024};
    const size_t test_alignments[] = {16, 32, 64};
    const size_t num_sizes = sizeof(test_sizes) / sizeof(test_sizes[0]);
    const size_t num_alignments = sizeof(test_alignments) / sizeof(test_alignments[0]);

    int tests_passed = 0;
    int total_tests = 0;

    for (size_t i = 0; i < num_sizes; i++) {
        for (size_t j = 0; j < num_alignments; j++) {
            size_t size = test_sizes[i];
            size_t alignment = test_alignments[j];

            printf("Testing size=%zu, alignment=%zu... ", size, alignment);
            total_tests++;

            /* Allocate aligned memory */
            void* ptr = vv_dsp_aligned_malloc(size, alignment);
            if (!ptr) {
                printf("FAILED (allocation)\n");
                continue;
            }

            /* Check alignment */
            if (!vv_dsp_is_aligned(ptr, alignment)) {
                printf("FAILED (alignment)\n");
                vv_dsp_aligned_free(ptr);
                continue;
            }

            /* Test memory access */
            memset(ptr, 0xAB, size);
            unsigned char* bytes = (unsigned char*)ptr;
            int memory_ok = 1;
            for (size_t k = 0; k < size; k++) {
                if (bytes[k] != 0xAB) {
                    memory_ok = 0;
                    break;
                }
            }

            if (!memory_ok) {
                printf("FAILED (memory access)\n");
                vv_dsp_aligned_free(ptr);
                continue;
            }

            /* Free memory */
            vv_dsp_aligned_free(ptr);

            printf("PASSED\n");
            tests_passed++;
        }
    }

    printf("\n");

    /* Test default SIMD allocation */
    printf("Testing default SIMD allocation... ");
    total_tests++;
    void* simd_ptr = vv_dsp_aligned_malloc_default(1024);
    if (simd_ptr && vv_dsp_is_simd_aligned(simd_ptr)) {
        printf("PASSED\n");
        tests_passed++;
        vv_dsp_aligned_free(simd_ptr);
    } else {
        printf("FAILED\n");
        if (simd_ptr) vv_dsp_aligned_free(simd_ptr);
    }

    /* Test NULL handling */
    printf("Testing NULL handling... ");
    total_tests++;
    vv_dsp_aligned_free(NULL); /* Should not crash */
    printf("PASSED\n");
    tests_passed++;

    /* Test edge cases */
    printf("Testing edge cases... ");
    total_tests++;

    /* Zero size should return NULL */
    void* zero_ptr = vv_dsp_aligned_malloc(0, 16);

    /* Invalid alignment (not power of 2) should return NULL */
    void* invalid_ptr = vv_dsp_aligned_malloc(64, 17);

    if (zero_ptr == NULL && invalid_ptr == NULL) {
        printf("PASSED\n");
        tests_passed++;
    } else {
        printf("FAILED\n");
        if (zero_ptr) vv_dsp_aligned_free(zero_ptr);
        if (invalid_ptr) vv_dsp_aligned_free(invalid_ptr);
    }

    printf("\n");
    printf("Test Results: %d/%d tests passed\n", tests_passed, total_tests);

    if (tests_passed == total_tests) {
        printf("✅ All tests passed!\n");
        return 0;
    } else {
        printf("❌ Some tests failed!\n");
        return 1;
    }
}
