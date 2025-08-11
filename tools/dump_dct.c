#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vv_dsp/spectral.h"

static void usage(const char* prog) {
    fprintf(stderr, "Usage: %s --type 2|3|4 --dir fwd|inv -n N --infile path\n", prog);
}

int main(int argc, char** argv) {
    size_t n = 16; int type = 2; const char* dir_s = "fwd"; const char* infile = NULL;
    for (int i=1;i<argc;++i) {
        if (!strcmp(argv[i], "-n") && i+1<argc) n = (size_t)strtoul(argv[++i], NULL, 10);
        else if (!strcmp(argv[i], "--type") && i+1<argc) type = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--dir") && i+1<argc) dir_s = argv[++i];
        else if (!strcmp(argv[i], "--infile") && i+1<argc) infile = argv[++i];
        else { usage(argv[0]); return 2; }
    }
    vv_dsp_dct_type t = (type==2)?VV_DSP_DCT_II:(type==3)?VV_DSP_DCT_III:VV_DSP_DCT_IV;
    vv_dsp_dct_dir dir = (!strcmp(dir_s, "fwd"))?VV_DSP_DCT_FORWARD:VV_DSP_DCT_BACKWARD;
    vv_dsp_dct_plan* p = NULL; if (vv_dsp_dct_make_plan(n, t, dir, &p) != VV_DSP_OK) { fprintf(stderr, "plan error\n"); return 1; }
    vv_dsp_real* in = (vv_dsp_real*)malloc(n*sizeof(vv_dsp_real));
    vv_dsp_real* out = (vv_dsp_real*)malloc(n*sizeof(vv_dsp_real));
    if (!in || !out) return 1;
    if (infile) {
        FILE* f = fopen(infile, "r"); if (!f) { perror("fopen"); return 1; }
        for (size_t i=0;i<n;++i) if (fscanf(f, "%f", &in[i]) != 1) { fclose(f); return 1; }
        fclose(f);
    } else {
        for (size_t i=0;i<n;++i) in[i] = (vv_dsp_real)(i % 7);
    }
    if (vv_dsp_dct_execute(p, in, out) != VV_DSP_OK) return 1;
    for (size_t i=0;i<n;++i) printf("%g\n", (double)out[i]);
    vv_dsp_dct_destroy(p); free(in); free(out); return 0;
}
