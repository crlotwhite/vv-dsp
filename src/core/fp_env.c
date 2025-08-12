/**
 * @file fp_env.c
 * @brief Implementation of floating-point environment control
 */

#include "vv_dsp/core/fp_env.h"

// Platform detection and implementation
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
    // x86/x64 implementation using SSE intrinsics
    #include <xmmintrin.h>
    #define VV_DSP_HAS_FTZ_SUPPORT 1

    // MXCSR register bit definitions
    #define VV_DSP_MXCSR_FTZ  (1U << 15)  // Flush to Zero (bit 15)
    #define VV_DSP_MXCSR_DAZ  (1U << 6)   // Denormals Are Zero (bit 6)

    void vv_dsp_set_flush_denormals(bool enable) {
        unsigned int mxcsr = _mm_getcsr();
        if (enable) {
            // Set FZ (Flush to Zero, bit 15) and DAZ (Denormals Are Zero, bit 6)
            mxcsr |= (VV_DSP_MXCSR_FTZ | VV_DSP_MXCSR_DAZ);
        } else {
            // Clear FZ and DAZ bits to restore normal IEEE 754 behavior
            mxcsr &= ~(VV_DSP_MXCSR_FTZ | VV_DSP_MXCSR_DAZ);
        }
        _mm_setcsr(mxcsr);
    }

    bool vv_dsp_get_flush_denormals_mode(void) {
        unsigned int mxcsr = _mm_getcsr();
        // Both FZ and DAZ must be set for full denormal handling
        return (mxcsr & (VV_DSP_MXCSR_FTZ | VV_DSP_MXCSR_DAZ)) ==
               (VV_DSP_MXCSR_FTZ | VV_DSP_MXCSR_DAZ);
    }

#elif defined(__aarch64__)
    // AArch64 implementation using FPCR register
    #include <stdint.h>
    #define VV_DSP_HAS_FTZ_SUPPORT 1

    void vv_dsp_set_flush_denormals(bool enable) {
        uint64_t fpcr;
        // Read FPCR (Floating-Point Control Register)
        __asm__ __volatile__("mrs %0, fpcr" : "=r"(fpcr));

        if (enable) {
            // Set FZ bit (bit 24) - Flush to Zero
            fpcr |= (1UL << 24);
        } else {
            // Clear FZ bit to restore normal IEEE 754 behavior
            fpcr &= ~(1UL << 24);
        }

        // Write back to FPCR
        __asm__ __volatile__("msr fpcr, %0" : : "r"(fpcr));
    }

    bool vv_dsp_get_flush_denormals_mode(void) {
        uint64_t fpcr;
        __asm__ __volatile__("mrs %0, fpcr" : "=r"(fpcr));
        // Check if FZ bit (bit 24) is set
        return (fpcr & (1UL << 24)) != 0;
    }

#elif defined(__arm__) && defined(__ARM_ARCH) && (__ARM_ARCH >= 7)
    // ARMv7+ implementation using VFP FPSCR register
    #include <stdint.h>
    #define VV_DSP_HAS_FTZ_SUPPORT 1

    void vv_dsp_set_flush_denormals(bool enable) {
        uint32_t fpscr;
        // Read FPSCR (Floating-Point Status and Control Register)
        __asm__ __volatile__("vmrs %0, fpscr" : "=r"(fpscr));

        if (enable) {
            // Set FZ bit (bit 24) - Flush to Zero
            fpscr |= (1UL << 24);
        } else {
            // Clear FZ bit to restore normal IEEE 754 behavior
            fpscr &= ~(1UL << 24);
        }

        // Write back to FPSCR
        __asm__ __volatile__("vmsr fpscr, %0" : : "r"(fpscr));
    }

    bool vv_dsp_get_flush_denormals_mode(void) {
        uint32_t fpscr;
        __asm__ __volatile__("vmrs %0, fpscr" : "=r"(fpscr));
        // Check if FZ bit (bit 24) is set
        return (fpscr & (1UL << 24)) != 0;
    }

#else
    // Fallback implementation for unsupported platforms
    #define VV_DSP_HAS_FTZ_SUPPORT 0

    void vv_dsp_set_flush_denormals(bool enable) {
        // No-op for unsupported platforms
        (void)enable; // Suppress unused parameter warning
    }

    bool vv_dsp_get_flush_denormals_mode(void) {
        // Always return false for unsupported platforms
        return false;
    }

#endif
