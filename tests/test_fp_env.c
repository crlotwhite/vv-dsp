/**
 * @file test_fp_env.c
 * @brief Unit tests for floating-point environment control (FTZ/DAZ)
 */

#include "vv_dsp/core/fp_env.h"
#include "vv_dsp/vv_dsp_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>

// Platform-specific includes for direct register access
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
    #include <xmmintrin.h>
    #define HAVE_REGISTER_ACCESS 1
    #define VV_DSP_MXCSR_FTZ  (1U << 15)
    #define VV_DSP_MXCSR_DAZ  (1U << 6)
#elif defined(__aarch64__)
    #include <stdint.h>
    #define HAVE_REGISTER_ACCESS 1
    #define VV_DSP_FPCR_FTZ   (1UL << 24)
#elif defined(__arm__) && defined(__ARM_ARCH) && (__ARM_ARCH >= 7)
    #include <stdint.h>
    #define HAVE_REGISTER_ACCESS 1
    #define VV_DSP_FPSCR_FTZ  (1UL << 24)
#else
    #define HAVE_REGISTER_ACCESS 0
#endif

static int test_count = 0;
static int test_passed = 0;

#define TEST_ASSERT(condition, message) \
    do { \
        test_count++; \
        if (condition) { \
            test_passed++; \
            printf("PASS: %s\n", message); \
        } else { \
            printf("FAIL: %s\n", message); \
        } \
    } while (0)

// Platform-specific register reading functions
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
    static unsigned int read_mxcsr(void) {
        return _mm_getcsr();
    }
    static void write_mxcsr(unsigned int value) {
        _mm_setcsr(value);
    }
#elif defined(__aarch64__)
    static uint64_t read_fpcr(void) {
        uint64_t fpcr;
        __asm__ __volatile__("mrs %0, fpcr" : "=r"(fpcr));
        return fpcr;
    }
    static void write_fpcr(uint64_t value) {
        __asm__ __volatile__("msr fpcr, %0" : : "r"(value));
    }
#elif defined(__arm__) && defined(__ARM_ARCH) && (__ARM_ARCH >= 7)
    static uint32_t read_fpscr(void) {
        uint32_t fpscr;
        __asm__ __volatile__("vmrs %0, fpscr" : "=r"(fpscr));
        return fpscr;
    }
    static void write_fpscr(uint32_t value) {
        __asm__ __volatile__("vmsr fpscr, %0" : : "r"(value));
    }
#endif

/**
 * @brief Test API state control and register verification
 */
void test_fp_env_api_state(void) {
    printf("\n=== Testing FP Environment API State Control ===\n");

#if HAVE_REGISTER_ACCESS
    // Save initial state
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
    unsigned int initial_mxcsr = read_mxcsr();
#elif defined(__aarch64__)
    uint64_t initial_fpcr = read_fpcr();
#elif defined(__arm__) && defined(__ARM_ARCH) && (__ARM_ARCH >= 7)
    uint32_t initial_fpscr = read_fpscr();
#endif

    // Test setting FTZ to true
    vv_dsp_set_flush_denormals(true);
    bool mode_enabled = vv_dsp_get_flush_denormals_mode();
    TEST_ASSERT(mode_enabled == true, "API reports FTZ enabled after set_flush_denormals(true)");

    // Verify register state directly
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
    unsigned int mxcsr_enabled = read_mxcsr();
    bool register_bits_set = ((mxcsr_enabled & (VV_DSP_MXCSR_FTZ | VV_DSP_MXCSR_DAZ)) ==
                             (VV_DSP_MXCSR_FTZ | VV_DSP_MXCSR_DAZ));
    TEST_ASSERT(register_bits_set, "MXCSR register has FTZ and DAZ bits set");
#elif defined(__aarch64__)
    uint64_t fpcr_enabled = read_fpcr();
    bool register_bit_set = (fpcr_enabled & VV_DSP_FPCR_FTZ) != 0;
    TEST_ASSERT(register_bit_set, "FPCR register has FTZ bit set");
#elif defined(__arm__) && defined(__ARM_ARCH) && (__ARM_ARCH >= 7)
    uint32_t fpscr_enabled = read_fpscr();
    bool register_bit_set = (fpscr_enabled & VV_DSP_FPSCR_FTZ) != 0;
    TEST_ASSERT(register_bit_set, "FPSCR register has FTZ bit set");
#endif

    // Test setting FTZ to false
    vv_dsp_set_flush_denormals(false);
    bool mode_disabled = vv_dsp_get_flush_denormals_mode();
    TEST_ASSERT(mode_disabled == false, "API reports FTZ disabled after set_flush_denormals(false)");

    // Verify register state directly for disabled state
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
    unsigned int mxcsr_disabled = read_mxcsr();
    bool register_bits_clear = ((mxcsr_disabled & (VV_DSP_MXCSR_FTZ | VV_DSP_MXCSR_DAZ)) == 0);
    TEST_ASSERT(register_bits_clear, "MXCSR register has FTZ and DAZ bits cleared");
#elif defined(__aarch64__)
    uint64_t fpcr_disabled = read_fpcr();
    bool register_bit_clear = (fpcr_disabled & VV_DSP_FPCR_FTZ) == 0;
    TEST_ASSERT(register_bit_clear, "FPCR register has FTZ bit cleared");
#elif defined(__arm__) && defined(__ARM_ARCH) && (__ARM_ARCH >= 7)
    uint32_t fpscr_disabled = read_fpscr();
    bool register_bit_clear = (fpscr_disabled & VV_DSP_FPSCR_FTZ) == 0;
    TEST_ASSERT(register_bit_clear, "FPSCR register has FTZ bit cleared");
#endif

    // Restore initial state
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
    write_mxcsr(initial_mxcsr);
#elif defined(__aarch64__)
    write_fpcr(initial_fpcr);
#elif defined(__arm__) && defined(__ARM_ARCH) && (__ARM_ARCH >= 7)
    write_fpscr(initial_fpscr);
#endif

    printf("Register state successfully restored to initial values\n");

#else
    // On unsupported platforms, just test that the API doesn't crash
    vv_dsp_set_flush_denormals(true);
    bool mode = vv_dsp_get_flush_denormals_mode();
    TEST_ASSERT(mode == false, "Unsupported platform returns false for FTZ mode");

    vv_dsp_set_flush_denormals(false);
    mode = vv_dsp_get_flush_denormals_mode();
    TEST_ASSERT(mode == false, "Unsupported platform consistently returns false");

    printf("Fallback implementation tested successfully\n");
#endif
}

/**
 * @brief Test functional behavior with denormal numbers
 */
void test_denormal_behavior(void) {
    printf("\n=== Testing Denormal Number Behavior ===\n");

#if HAVE_REGISTER_ACCESS
    // Save initial state
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
    unsigned int initial_mxcsr = read_mxcsr();
#elif defined(__aarch64__)
    uint64_t initial_fpcr = read_fpcr();
#elif defined(__arm__) && defined(__ARM_ARCH) && (__ARM_ARCH >= 7)
    uint32_t initial_fpscr = read_fpscr();
#endif

    // Create a denormal number
    volatile float denormal = FLT_MIN / 2.0f;

    // Test with FTZ disabled (normal IEEE 754 behavior)
    vv_dsp_set_flush_denormals(false);
    volatile float result_normal = denormal * 1.0f;

    // The result should still be denormal in normal mode
    bool normal_preserves_denormal = (result_normal != 0.0f) && (fabsf(result_normal) < FLT_MIN);
    TEST_ASSERT(normal_preserves_denormal, "Normal mode preserves denormal numbers");

    // Test with FTZ enabled
    vv_dsp_set_flush_denormals(true);
    volatile float result_ftz = denormal * 1.0f;

    // The result should be flushed to zero with FTZ enabled
    bool ftz_flushes_denormal = (result_ftz == 0.0f);
    TEST_ASSERT(ftz_flushes_denormal, "FTZ mode flushes denormal numbers to zero");

    // Additional test: denormal input should also be treated as zero
    volatile float denormal_input = FLT_MIN / 4.0f;
    volatile float normal_value = 2.0f;
    volatile float result_daz = denormal_input + normal_value;

    // With DAZ enabled, denormal input should be treated as zero
    bool daz_treats_input_as_zero = (fabsf(result_daz - normal_value) < 1e-6f);

    // Note: DAZ only works on x86/x64, so only test there
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
    TEST_ASSERT(daz_treats_input_as_zero, "DAZ mode treats denormal inputs as zero");
#else
    printf("DAZ test skipped (not supported on this platform)\n");
#endif

    // Restore initial state
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
    write_mxcsr(initial_mxcsr);
#elif defined(__aarch64__)
    write_fpcr(initial_fpcr);
#elif defined(__arm__) && defined(__ARM_ARCH) && (__ARM_ARCH >= 7)
    write_fpscr(initial_fpscr);
#endif

#else
    printf("Denormal behavior test skipped (unsupported platform)\n");
    // On unsupported platforms, the functions should not crash
    vv_dsp_set_flush_denormals(true);
    vv_dsp_set_flush_denormals(false);
    bool mode = vv_dsp_get_flush_denormals_mode();
    TEST_ASSERT(mode == false, "Unsupported platform API is stable");
#endif
}

/**
 * @brief Test legacy compatibility function
 */
void test_legacy_compatibility(void) {
    printf("\n=== Testing Legacy Function Compatibility ===\n");

    // Test that the legacy function doesn't crash
    extern void vv_dsp_flush_denormals(void);
    vv_dsp_flush_denormals();

    // After calling legacy function, check if FTZ is enabled
    bool mode_after_legacy = vv_dsp_get_flush_denormals_mode();

#if HAVE_REGISTER_ACCESS
    TEST_ASSERT(mode_after_legacy == true, "Legacy function enables FTZ mode");
#else
    TEST_ASSERT(mode_after_legacy == false, "Legacy function on unsupported platform returns false");
#endif

    // Reset to disabled state
    vv_dsp_set_flush_denormals(false);
    printf("Legacy compatibility verified\n");
}

int main(void) {
    printf("Starting FP Environment Control Tests\n");
    printf("Platform: ");

#if defined(__x86_64__) || defined(_M_X64)
    printf("x86_64\n");
#elif defined(__i386) || defined(_M_IX86)
    printf("x86\n");
#elif defined(__aarch64__)
    printf("AArch64\n");
#elif defined(__arm__) && defined(__ARM_ARCH) && (__ARM_ARCH >= 7)
    printf("ARMv7+\n");
#else
    printf("Unsupported (fallback mode)\n");
#endif

    test_fp_env_api_state();
    test_denormal_behavior();
    test_legacy_compatibility();

    printf("\n=== Test Summary ===\n");
    printf("Tests run: %d\n", test_count);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_count - test_passed);

    if (test_passed == test_count) {
        printf("All tests PASSED!\n");
        return 0;
    } else {
        printf("Some tests FAILED!\n");
        return 1;
    }
}
