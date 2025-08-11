#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "vv_dsp/vv_dsp.h"

static void usage(const char* prog){
    fprintf(stderr, "Usage: %s --N <N> --M <M> --Wre <wre> --Wim <wim> --Are <are> --Aim <aim> --infile <path>\n", prog);
}

int main(int argc, char** argv){
    size_t N=0, M=0; float Wre=0, Wim=0, Are=1, Aim=0; const char* infile=NULL; int real_input=1;
    for (int i=1;i<argc;i++){
        if (!strcmp(argv[i], "--N") && i+1<argc) { N = (size_t)strtoul(argv[++i], NULL, 10); }
        else if (!strcmp(argv[i], "--M") && i+1<argc) { M = (size_t)strtoul(argv[++i], NULL, 10); }
        else if (!strcmp(argv[i], "--Wre") && i+1<argc) { Wre = strtof(argv[++i], NULL); }
        else if (!strcmp(argv[i], "--Wim") && i+1<argc) { Wim = strtof(argv[++i], NULL); }
        else if (!strcmp(argv[i], "--Are") && i+1<argc) { Are = strtof(argv[++i], NULL); }
        else if (!strcmp(argv[i], "--Aim") && i+1<argc) { Aim = strtof(argv[++i], NULL); }
        else if (!strcmp(argv[i], "--infile") && i+1<argc) { infile = argv[++i]; }
        else if (!strcmp(argv[i], "--complex")) { real_input = 0; }
    }
    if (N==0 || M==0 || !infile){ usage(argv[0]); return 2; }

    FILE* f = fopen(infile, "r");
    if (!f){ perror("fopen"); return 2; }

    vv_dsp_cpx* out = (vv_dsp_cpx*)malloc(M*sizeof(vv_dsp_cpx));
    if (!out){ fclose(f); return 2; }

    int rc = 0;
    if (real_input){
        vv_dsp_real* x = (vv_dsp_real*)malloc(N*sizeof(vv_dsp_real));
        if (!x){ free(out); fclose(f); return 2; }
        for (size_t n=0;n<N;++n){ if (fscanf(f, "%f", &x[n])!=1){ rc=2; break; } }
        fclose(f);
        if (!rc){
            vv_dsp_status st = vv_dsp_czt_exec_real(x, N, M, Wre, Wim, Are, Aim, out);
            if (st != VV_DSP_OK) rc = 3;
        }
        free(x);
    } else {
        vv_dsp_cpx* xc = (vv_dsp_cpx*)malloc(N*sizeof(vv_dsp_cpx));
        if (!xc){ free(out); fclose(f); return 2; }
        for (size_t n=0;n<N;++n){ if (fscanf(f, "%f,%f", &xc[n].re, &xc[n].im)!=2){ rc=2; break; } }
        fclose(f);
        if (!rc){
            vv_dsp_status st = vv_dsp_czt_exec_cpx(xc, N, M, Wre, Wim, Are, Aim, out);
            if (st != VV_DSP_OK) rc = 3;
        }
        free(xc);
    }

    if (rc==0){
        for (size_t k=0;k<M;++k){ printf("%.8g,%.8g\n", (double)out[k].re, (double)out[k].im); }
    }
    free(out);
    return rc;
}
