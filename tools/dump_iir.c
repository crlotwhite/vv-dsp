#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vv_dsp/filter.h"

static void usage(const char* p){
    fprintf(stderr, "Usage: %s --b0 B0 --b1 B1 --b2 B2 --a1 A1 --a2 A2 --n SAMPLES --seed SEED\n", p);
}

int main(int argc, char** argv){
    float b0=1.0f,b1=0.0f,b2=0.0f,a1=0.0f,a2=0.0f; size_t n=128; unsigned int seed=0; const char* infile=NULL;
    for(int i=1;i<argc;++i){
        if(!strcmp(argv[i],"--b0")&&i+1<argc) b0=strtof(argv[++i],NULL);
        else if(!strcmp(argv[i],"--b1")&&i+1<argc) b1=strtof(argv[++i],NULL);
        else if(!strcmp(argv[i],"--b2")&&i+1<argc) b2=strtof(argv[++i],NULL);
        else if(!strcmp(argv[i],"--a1")&&i+1<argc) a1=strtof(argv[++i],NULL);
        else if(!strcmp(argv[i],"--a2")&&i+1<argc) a2=strtof(argv[++i],NULL);
        else if(!strcmp(argv[i],"--n")&&i+1<argc) n=strtoul(argv[++i],NULL,10);
    else if(!strcmp(argv[i],"--seed")&&i+1<argc) seed=(unsigned)strtoul(argv[++i],NULL,10);
    else if(!strcmp(argv[i],"--infile")&&i+1<argc) infile=argv[++i];
        else { usage(argv[0]); return 2; }
    }
    // Python reference uses lfilter(b, [1, -a1, -a2], x), and our DF2T uses
    // y = ... - A1*y[n-1] - A2*y[n-2]; so set A1=-a1, A2=-a2 to match.
    vv_dsp_biquad biq; if(vv_dsp_biquad_init(&biq, b0,b1,b2, -a1, -a2)!=VV_DSP_OK) return 1; vv_dsp_biquad_reset(&biq);
    srand(seed);
    vv_dsp_real* in=(vv_dsp_real*)malloc(n*sizeof(vv_dsp_real));
    vv_dsp_real* out=(vv_dsp_real*)malloc(n*sizeof(vv_dsp_real));
    if(!in||!out) return 1;
    if(infile){
        FILE* f = fopen(infile, "r"); if(!f){ perror("fopen"); return 1; }
        for(size_t i=0;i<n;++i){ if(fscanf(f, "%f", &in[i])!=1){ fclose(f); return 1; } }
        fclose(f);
    } else {
        for(size_t i=0;i<n;++i) in[i] = (float)rand()/RAND_MAX*2.f-1.f;
    }
    if(vv_dsp_iir_apply(&biq, 1, in, out, n)!=VV_DSP_OK) return 1;
    for(size_t i=0;i<n;++i) printf("%g\n", (double)out[i]);
    free(in); free(out);
    return 0;
}
