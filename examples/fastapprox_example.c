#include <vv_dsp/vv_dsp.h>
#include <stdio.h>
#include <math.h>

/**
 * @brief Example demonstrating fast approximation functions
 * 
 * This example shows how to use fastapprox and other optimization
 * libraries integrated into vv-dsp.
 */
int main(void) {
    printf("vv-dsp Fast Approximation Example\n");
    printf("==================================\n\n");

    // Check which optimization libraries are available
    printf("Available optimization libraries:\n");
    printf("- FastApprox: %s\n", vv_dsp_has_fastapprox() ? "YES" : "NO");
    printf("- Math Approx: %s\n", vv_dsp_has_math_approx() ? "YES" : "NO");
    printf("- Eigen: %s\n", vv_dsp_has_eigen() ? "YES" : "NO");
    printf("\n");

    // Test some common math functions
    double x = 2.0;
    printf("Testing with x = %.3f:\n", x);
    
    // Test exponential function
    double standard_exp = exp(x);
    double fast_exp = VV_DSP_FAST_EXP(x);
    printf("exp(%.3f):\n", x);
    printf("  Standard: %.6f\n", standard_exp);
    printf("  Fast:     %.6f\n", fast_exp);
    printf("  Error:    %.6f\n", fabs(standard_exp - fast_exp));
    printf("\n");

    // Test logarithm function
    double standard_log = log(x);
    double fast_log = VV_DSP_FAST_LOG(x);
    printf("log(%.3f):\n", x);
    printf("  Standard: %.6f\n", standard_log);
    printf("  Fast:     %.6f\n", fast_log);
    printf("  Error:    %.6f\n", fabs(standard_log - fast_log));
    printf("\n");

    // Test trigonometric functions
    double angle = M_PI / 4.0; // 45 degrees
    double standard_sin = sin(angle);
    double fast_sin = VV_DSP_FAST_SIN(angle);
    printf("sin(π/4):\n");
    printf("  Standard: %.6f\n", standard_sin);
    printf("  Fast:     %.6f\n", fast_sin);
    printf("  Error:    %.6f\n", fabs(standard_sin - fast_sin));
    printf("\n");

    double standard_cos = cos(angle);
    double fast_cos = VV_DSP_FAST_COS(angle);
    printf("cos(π/4):\n");
    printf("  Standard: %.6f\n", standard_cos);
    printf("  Fast:     %.6f\n", fast_cos);
    printf("  Error:    %.6f\n", fabs(standard_cos - fast_cos));
    printf("\n");

    // Test power function
    double y = 3.0;
    double standard_pow = pow(x, y);
    double fast_pow = VV_DSP_FAST_POW(x, y);
    printf("pow(%.3f, %.3f):\n", x, y);
    printf("  Standard: %.6f\n", standard_pow);
    printf("  Fast:     %.6f\n", fast_pow);
    printf("  Error:    %.6f\n", fabs(standard_pow - fast_pow));
    printf("\n");

    printf("Note: Enable fast approximation libraries with:\n");
    printf("  -DVV_DSP_USE_FASTAPPROX=ON\n");
    printf("  -DVV_DSP_USE_MATH_APPROX=ON\n");
    printf("  -DVV_DSP_USE_EIGEN=ON\n");
    printf("\nExample: cmake -B build -DVV_DSP_USE_FASTAPPROX=ON\n");

    return 0;
}
