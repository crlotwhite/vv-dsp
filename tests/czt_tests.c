#include <stdio.h>
#include <string.h>
#include <math.h>
#include "vv_dsp/vv_dsp.h"

static int nearly_equal(vv_dsp_real a, vv_dsp_real b, vv_dsp_real tol){
    vv_dsp_real d = a - b; if (d<0) d = -d; return d <= tol;
}

int main(void){
    // Simple sanity: CZT with parameters equivalent to DFT of length N
    // For DFT, choose M=N, A=1, W=exp(-j*2*pi/N)
    // Note: MSVC doesn't treat 'const size_t' as a constant expression for array sizes.
    // Use enum constants to keep sizes compile-time constants across compilers.
    enum { N = 8, M = N };
    vv_dsp_real ang = (vv_dsp_real)(-2.0 * VV_DSP_PI_D / (double)N);
    vv_dsp_real Wre = VV_DSP_COS(ang), Wim = VV_DSP_SIN(ang);
    vv_dsp_real Are = (vv_dsp_real)1, Aim = (vv_dsp_real)0;

    vv_dsp_cpx x[N];
    memset(x, 0, sizeof(x));
    // impulse -> flat spectrum
    x[0].re = (vv_dsp_real)1; x[0].im = (vv_dsp_real)0;

    vv_dsp_cpx X[M];
    if (vv_dsp_czt_exec_cpx(x, N, M, Wre, Wim, Are, Aim, X) != VV_DSP_OK){
        fprintf(stderr, "czt exec failed\n");
        return 1;
    }

    for (size_t k=0;k<M;++k){
        if (!nearly_equal(X[k].re, (vv_dsp_real)1.0, (vv_dsp_real)1e-3) || !nearly_equal(X[k].im, (vv_dsp_real)0.0, (vv_dsp_real)1e-3)){
            fprintf(stderr, "CZT DFT-equivalence failed at k=%zu: got (%g,%g)\n", k, (double)X[k].re, (double)X[k].im);
            return 1;
        }
    }

    printf("czt tests passed\n");
    return 0;
}
