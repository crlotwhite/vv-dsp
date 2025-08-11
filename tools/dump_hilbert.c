#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "vv_dsp/spectral.h"

static void usage(const char* prog) {
    fprintf(stderr, "Usage: %s -n N --fs FS --f F0 --phase PHASE\n", prog);
}

int main(int argc, char** argv) {
    size_t n = 256;
    double fs = 1000.0;
    double f0 = 123.0;
    double ph = 0.0;
    for (int i=1;i<argc;++i) {
        if (!strcmp(argv[i], "-n") && i+1<argc) n = (size_t)strtoul(argv[++i], NULL, 10);
        else if (!strcmp(argv[i], "--fs") && i+1<argc) fs = atof(argv[++i]);
        else if (!strcmp(argv[i], "--f") && i+1<argc) f0 = atof(argv[++i]);
        else if (!strcmp(argv[i], "--phase") && i+1<argc) ph = atof(argv[++i]);
        else { usage(argv[0]); return 2; }
    }
    vv_dsp_real* x = (vv_dsp_real*)malloc(n*sizeof(vv_dsp_real));
    vv_dsp_cpx* xa = (vv_dsp_cpx*)malloc(n*sizeof(vv_dsp_cpx));
    if (!x || !xa) return 1;
    for (size_t t=0;t<n;++t) x[t] = (vv_dsp_real)sin(2.0*M_PI*f0*((double)t/fs) + ph);
    if (vv_dsp_hilbert_analytic(x, n, xa) != VV_DSP_OK) return 3;
    // Compute phase and freq for sanity
    vv_dsp_real* phi = (vv_dsp_real*)malloc(n*sizeof(vv_dsp_real));
    vv_dsp_real* freq = (vv_dsp_real*)malloc(n*sizeof(vv_dsp_real));
    if (phi && freq) {
        vv_dsp_instantaneous_phase(xa, n, phi);
        vv_dsp_instantaneous_frequency(phi, n, fs, freq);
        double avg=0; size_t cnt=0; for (size_t i=1;i<n;++i){ avg += (double)freq[i]; cnt++; }
    avg/= (cnt? (double)cnt:1.0);
    double avgd=0; for (size_t i=1;i<n;++i){ avgd += (double)(phi[i]-phi[i-1]); }
    avgd /= (cnt? (double)cnt:1.0);
    double manual = avgd * fs / (2.0*M_PI);
    fprintf(stderr, "avg_ifreq=%g Hz manual=%g\n", avg, manual);
        fprintf(stderr, "phi[0..5]:");
        for (int i=0;i<6 && i<(int)n;++i) fprintf(stderr, " %g", (double)phi[i]);
        fprintf(stderr, "\nΔphi[1..5]:");
        for (int i=1;i<6 && i<(int)n;++i) fprintf(stderr, " %g", (double)(phi[i]-phi[i-1]));
        fprintf(stderr, "\n");
    }
    for (size_t i=0;i<n;++i) printf("%g,%g\n", (double)xa[i].re, (double)xa[i].im);
    free(phi); free(freq);
    free(x); free(xa);
    return 0;
}
