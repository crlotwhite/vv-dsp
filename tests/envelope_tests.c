#include <stdio.h>
#include <math.h>
#include <string.h>
#include "vv_dsp/envelope.h"

static int nearly_equal(float a, float b, float eps){ return fabsf(a-b) < eps; }

int main(void){
    // Simple impulse -> cepstrum should have c[0] ~ log(1)=0 and others ~ 0
    enum { N = 16 };
    float x[N]; memset(x, 0, sizeof(x)); x[0] = 1.0f;
    float c[N];
    if (vv_dsp_cepstrum_real(x, N, c) != VV_DSP_OK) { fprintf(stderr, "cepstrum failed\n"); return 1; }
    // looser checks due to naive DFT backend
    if (!nearly_equal(c[0], 0.0f, 1e-3f)) { fprintf(stderr, "c[0]=%f\n", c[0]); return 1; }
    for (size_t i=1;i<N;i++) if (!nearly_equal(c[i], 0.0f, 1e-2f)) { fprintf(stderr, "c[%zu]=%f\n", i, c[i]); return 1; }

    // Min-phase from cepstrum -> spectrum then back to time via icepstrum
    vv_dsp_cpx H[N];
    if (vv_dsp_minphase_from_cepstrum(c, N, H) != VV_DSP_OK) { fprintf(stderr, "minphase failed\n"); return 1; }
    float xr[N];
    if (vv_dsp_icepstrum_minphase(c, N, xr) != VV_DSP_OK) { fprintf(stderr, "icepstrum failed\n"); return 1; }
    // Impulse should reconstruct approximately impulse (first sample ~1)
    if (!nearly_equal(xr[0], 1.0f, 1e-2f)) { fprintf(stderr, "xr[0]=%f\n", xr[0]); return 1; }

    // LPC on a simple AR(1)-like signal: x[n]=0.9*x[n-1]
    float s[N]; s[0]=1.0f; for (size_t i=1;i<N;i++) s[i]=0.9f*s[i-1];
    enum { ORDER = 1 };
    float a[ORDER+1]; float err;
    if (vv_dsp_lpc(s, N, ORDER, a, &err) != VV_DSP_OK) { fprintf(stderr, "lpc failed\n"); return 1; }
    // For AR(1) with pole at 0.9, the predictor a[1] should be close to -0.9
    if (!(a[0]==1.0f) || fabsf(a[1] + 0.9f) > 0.2f) { fprintf(stderr, "a0=%f a1=%f\n", a[0], a[1]); return 1; }

    // LPC spectrum magnitude sampling
    float mag[N]; if (vv_dsp_lpspec(a, ORDER, 1.0f, N, mag) != VV_DSP_OK) return 1;
    // DC bin should be reasonably bounded
    if (!(mag[0] > 0.0f)) { fprintf(stderr, "mag[0]=%f\n", mag[0]); return 1; }

    printf("envelope tests passed\n");
    return 0;
}
