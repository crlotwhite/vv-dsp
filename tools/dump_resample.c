#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vv_dsp/resample.h"
#include <math.h>

static void usage(const char* p){
    fprintf(stderr, "Usage: %s --num NUM --den DEN --quality linear|sinc[:taps] --n IN_SAMPLES --seed SEED [--infile PATH]\n", p);
}

int main(int argc, char** argv){
    unsigned int num=2, den=1; const char* qual="linear"; unsigned int taps=32; size_t n=256; unsigned int seed=0; const char* infile=NULL;
    for(int i=1;i<argc;++i){
        if(!strcmp(argv[i],"--num")&&i+1<argc) num=(unsigned)strtoul(argv[++i],NULL,10);
        else if(!strcmp(argv[i],"--den")&&i+1<argc) den=(unsigned)strtoul(argv[++i],NULL,10);
        else if(!strcmp(argv[i],"--quality")&&i+1<argc) {
            qual=argv[++i];
            const char* c = strchr(qual, ':');
            if(c){ taps = (unsigned)strtoul(c+1,NULL,10); }
        }
    else if(!strcmp(argv[i],"--n")&&i+1<argc) n=strtoul(argv[++i],NULL,10);
    else if(!strcmp(argv[i],"--infile")&&i+1<argc) infile=argv[++i];
        else if(!strcmp(argv[i],"--seed")&&i+1<argc) seed=(unsigned)strtoul(argv[++i],NULL,10);
    else { /* ignore unknown token to be robust for harness */ }
    }
    vv_dsp_resampler* rs = vv_dsp_resampler_create(num, den);
    if(!rs) return 1;
    int use_sinc = strncmp(qual, "sinc", 4)==0 ? 1 : 0;
    if(vv_dsp_resampler_set_quality(rs, use_sinc, taps)!=0) return 1;

    vv_dsp_real* in=NULL;
    if(infile){
        // read lines count first
        FILE* f = fopen(infile, "r"); if(!f){ perror("fopen"); return 1; }
        // count lines
        size_t cnt=0; char buf[256];
        while(fgets(buf, sizeof(buf), f)) cnt++;
        rewind(f);
        n = cnt ? cnt : n;
        in = (vv_dsp_real*)malloc(n*sizeof(vv_dsp_real)); if(!in){ fclose(f); return 1; }
        for(size_t i=0;i<n;++i){ if(fscanf(f, "%f", &in[i])!=1){ fclose(f); free(in); return 1; } }
        fclose(f);
    } else {
        srand(seed);
        in=(vv_dsp_real*)malloc(n*sizeof(vv_dsp_real));
        for(size_t i=0;i<n;++i) in[i] = (float)rand()/RAND_MAX*2.f-1.f;
    }

    size_t out_cap = (size_t)ceil((double)n * (double)num / (double)den) + 8;
    vv_dsp_real* out=(vv_dsp_real*)malloc(out_cap*sizeof(vv_dsp_real));
    size_t out_n=0;
    if(vv_dsp_resampler_process_real(rs, in, n, out, out_cap, &out_n)!=0) return 1;
    for(size_t i=0;i<out_n;++i) printf("%g\n", (double)out[i]);

    free(in); free(out); vv_dsp_resampler_destroy(rs);
    return 0;
}
