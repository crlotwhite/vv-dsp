#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "vv_dsp/spectral.h"

static void usage(const char* prog) {
    fprintf(stderr, "Usage: %s --type c2c|r2c|c2r --dir fwd|inv -n N --seed S\n", prog);
}

int main(int argc, char** argv) {
    size_t n = 16;
    const char* type_s = "c2c";
    const char* dir_s = "fwd";
    unsigned int seed = 0;
    const char* infile = NULL;

    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-n") && i+1 < argc) { n = (size_t)strtoul(argv[++i], NULL, 10); }
        else if (!strcmp(argv[i], "--type") && i+1 < argc) { type_s = argv[++i]; }
        else if (!strcmp(argv[i], "--dir") && i+1 < argc) { dir_s = argv[++i]; }
    else if (!strcmp(argv[i], "--seed") && i+1 < argc) { seed = (unsigned)strtoul(argv[++i], NULL, 10); }
    else if (!strcmp(argv[i], "--infile") && i+1 < argc) { infile = argv[++i]; }
        else { usage(argv[0]); return 2; }
    }

    vv_dsp_fft_type type = VV_DSP_FFT_C2C;
    if (!strcmp(type_s, "c2c")) type = VV_DSP_FFT_C2C;
    else if (!strcmp(type_s, "r2c")) type = VV_DSP_FFT_R2C;
    else if (!strcmp(type_s, "c2r")) type = VV_DSP_FFT_C2R;
    else { usage(argv[0]); return 2; }

    vv_dsp_fft_dir dir = (!strcmp(dir_s, "fwd")) ? VV_DSP_FFT_FORWARD : VV_DSP_FFT_BACKWARD;

    vv_dsp_fft_plan* plan = NULL;
    if (vv_dsp_fft_make_plan(n, type, dir, &plan) != VV_DSP_OK || !plan) {
        fprintf(stderr, "plan error\n");
        return 1;
    }

    srand(seed);
    if (type == VV_DSP_FFT_C2C) {
        vv_dsp_cpx* in = (vv_dsp_cpx*)malloc(n * sizeof(vv_dsp_cpx));
        vv_dsp_cpx* out = (vv_dsp_cpx*)malloc(n * sizeof(vv_dsp_cpx));
        if (!in || !out) return 1;
        if (infile) {
            FILE* f = fopen(infile, "r");
            if (!f) { perror("fopen"); return 1; }
            for (size_t i=0;i<n;++i) {
                if (fscanf(f, "%f,%f", &in[i].re, &in[i].im) != 2) { fclose(f); return 1; }
            }
            fclose(f);
        } else {
            for (size_t i=0;i<n;++i) { in[i].re = (float)rand()/RAND_MAX; in[i].im = (float)rand()/RAND_MAX; }
        }
        if (vv_dsp_fft_execute(plan, in, out) != VV_DSP_OK) return 1;
        for (size_t i=0;i<n;++i) printf("%g,%g\n", (double)out[i].re, (double)out[i].im);
        free(in); free(out);
    } else if (type == VV_DSP_FFT_R2C) {
        vv_dsp_real* in = (vv_dsp_real*)malloc(n * sizeof(vv_dsp_real));
        size_t m = n/2 + 1;
        vv_dsp_cpx* out = (vv_dsp_cpx*)malloc(m * sizeof(vv_dsp_cpx));
        if (!in || !out) return 1;
        if (infile) {
            FILE* f = fopen(infile, "r");
            if (!f) { perror("fopen"); return 1; }
            for (size_t i=0;i<n;++i) {
                if (fscanf(f, "%f", &in[i]) != 1) { fclose(f); return 1; }
            }
            fclose(f);
        } else {
            for (size_t i=0;i<n;++i) { in[i] = (float)rand()/RAND_MAX; }
        }
        if (vv_dsp_fft_execute(plan, in, out) != VV_DSP_OK) return 1;
        for (size_t i=0;i<m;++i) printf("%g,%g\n", (double)out[i].re, (double)out[i].im);
        free(in); free(out);
    } else { // C2R
        size_t m = n/2 + 1;
        vv_dsp_cpx* in = (vv_dsp_cpx*)malloc(m * sizeof(vv_dsp_cpx));
        vv_dsp_real* out = (vv_dsp_real*)malloc(n * sizeof(vv_dsp_real));
        if (!in || !out) return 1;
        if (infile) {
            FILE* f = fopen(infile, "r");
            if (!f) { perror("fopen"); return 1; }
            for (size_t i=0;i<m;++i) {
                if (fscanf(f, "%f,%f", &in[i].re, &in[i].im) != 2) { fclose(f); return 1; }
            }
            fclose(f);
        } else {
            for (size_t i=0;i<m;++i) { in[i].re = (float)rand()/RAND_MAX; in[i].im = (float)rand()/RAND_MAX; }
        }
        if (vv_dsp_fft_execute(plan, in, out) != VV_DSP_OK) return 1;
        for (size_t i=0;i<n;++i) printf("%g\n", (double)out[i]);
        free(in); free(out);
    }
    vv_dsp_fft_destroy(plan);
    return 0;
}
