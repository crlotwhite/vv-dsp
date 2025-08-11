#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "vv_dsp/vv_dsp.h"

static double now_sec(void){
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

int main(int argc, char** argv){
    size_t N = 1024, M = 1024, iters = 50; double fs = 48000.0, f0 = 1000.0, bw = 2000.0;
    if (argc > 1) N = (size_t)strtoul(argv[1], NULL, 10);
    if (argc > 2) M = (size_t)strtoul(argv[2], NULL, 10);
    if (argc > 3) iters = (size_t)strtoul(argv[3], NULL, 10);

    vv_dsp_real *x = (vv_dsp_real*)malloc(N * sizeof(vv_dsp_real));
    vv_dsp_cpx *X = (vv_dsp_cpx*)malloc(M * sizeof(vv_dsp_cpx));
    if (!x || !X){ fprintf(stderr, "alloc fail\n"); return 2; }

    // simple sinusoid
    for (size_t n=0;n<N;++n) x[n] = (vv_dsp_real)cos(2.0 * VV_DSP_PI_D * f0 * (double)n / fs);

    vv_dsp_real Wre, Wim, Are, Aim;
    vv_dsp_czt_params_for_freq_range((vv_dsp_real)(f0 - bw/2), (vv_dsp_real)(f0 + bw/2), M, (vv_dsp_real)fs, &Wre,&Wim,&Are,&Aim);

    // warmup
    vv_dsp_czt_exec_real(x, N, M, Wre, Wim, Are, Aim, X);

    double t0 = now_sec();
    for (size_t i=0;i<iters;++i){
        vv_dsp_czt_exec_real(x, N, M, Wre, Wim, Are, Aim, X);
    }
    double t1 = now_sec();

    double avg_ms = (t1 - t0) * 1e3 / (double)iters;
    printf("CZT bench: N=%zu M=%zu iters=%zu avg=%.3f ms\n", N, M, iters, avg_ms);

    // print peak bin
    size_t argmax = 0; vv_dsp_real maxmag = 0;
    for (size_t k=0;k<M;++k){ vv_dsp_real m = X[k].re*X[k].re + X[k].im*X[k].im; if (m>maxmag){maxmag=m; argmax=k;} }
    printf("Peak bin: %zu\n", argmax);

    free(x); free(X); return 0;
}
