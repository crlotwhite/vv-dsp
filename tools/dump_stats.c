#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vv_dsp/vv_dsp.h"

int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "usage: %s autocorr <n> [biased]\n", argv[0]);
        return 2;
    }
    const char* cmd = argv[1];
    size_t n = (size_t)strtoul(argv[2], NULL, 10);
    vv_dsp_real* x = (vv_dsp_real*)malloc(sizeof(vv_dsp_real)*n);
    if (!x) return 3;
    for (size_t i = 0; i < n; ++i) {
        double v;
        if (scanf("%lf", &v) != 1) { free(x); return 4; }
        x[i] = (vv_dsp_real)v;
    }
    if (strcmp(cmd, "autocorr") == 0) {
        int biased = 1;
        if (argc >= 4) biased = atoi(argv[3]);
        size_t rlen = n; // full lags 0..n-1
        vv_dsp_real* r = (vv_dsp_real*)malloc(sizeof(vv_dsp_real)*rlen);
        if (!r) { free(x); return 5; }
        vv_dsp_status st = vv_dsp_autocorrelation(x, n, r, rlen, biased);
        if (st != VV_DSP_OK) { free(x); free(r); return 6; }
        for (size_t i = 0; i < rlen; ++i) {
            printf("%.17g\n", (double)r[i]);
        }
        free(r); free(x); return 0;
    }
    free(x);
    fprintf(stderr, "unknown command\n");
    return 2;
}
