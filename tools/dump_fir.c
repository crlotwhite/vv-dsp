#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vv_dsp/filter.h"
#include "vv_dsp/window.h"

static void usage(const char* p){
    fprintf(stderr, "Usage: %s --num-taps N --cutoff C --win hann|hamming|boxcar --n SAMPLES --seed SEED\n", p);
}

int main(int argc, char** argv){
    size_t num_taps = 33; float cutoff = 0.25f; const char* win="hann"; size_t n=128; unsigned int seed=0; const char* infile=NULL; const char* dump_coeffs=NULL;
    for (int i=1;i<argc;++i){
        if(!strcmp(argv[i],"--num-taps")&&i+1<argc) num_taps=strtoul(argv[++i],NULL,10);
        else if(!strcmp(argv[i],"--cutoff")&&i+1<argc) cutoff=strtof(argv[++i],NULL);
        else if(!strcmp(argv[i],"--win")&&i+1<argc) win=argv[++i];
        else if(!strcmp(argv[i],"--n")&&i+1<argc) n=strtoul(argv[++i],NULL,10);
    else if(!strcmp(argv[i],"--seed")&&i+1<argc) seed=(unsigned)strtoul(argv[++i],NULL,10);
    else if(!strcmp(argv[i],"--infile")&&i+1<argc) infile=argv[++i];
    else if(!strcmp(argv[i],"--dump-coeffs")&&i+1<argc) dump_coeffs=argv[++i];
        else { usage(argv[0]); return 2; }
    }
    vv_dsp_window_type w = VV_DSP_WINDOW_HANNING;
    if(!strcmp(win,"hann")) w = VV_DSP_WINDOW_HANNING;
    else if(!strcmp(win,"hamming")) w = VV_DSP_WINDOW_HAMMING;
    else if(!strcmp(win,"boxcar") || !strcmp(win, "rect")) w = VV_DSP_WINDOW_RECTANGULAR;
    else if(!strcmp(win,"blackman")) w = VV_DSP_WINDOW_BLACKMAN;
    else { usage(argv[0]); return 2; }

    vv_dsp_real* coeffs = (vv_dsp_real*)malloc(num_taps*sizeof(vv_dsp_real));
    if(!coeffs) return 1;
    if(vv_dsp_fir_design_lowpass(coeffs, num_taps, cutoff, w)!=VV_DSP_OK) return 1;
    if(dump_coeffs){
        FILE* f = fopen(dump_coeffs, "w"); if(!f){ perror("fopen coeffs"); return 1; }
        for(size_t i=0;i<num_taps;++i) fprintf(f, "%g\n", (double)coeffs[i]);
        fclose(f);
    }

    vv_dsp_fir_state st; if(vv_dsp_fir_state_init(&st, num_taps)!=VV_DSP_OK) return 1;
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
    // Use direct FIR to match lfilter reference precisely for this harness
    if(vv_dsp_fir_apply(&st, coeffs, in, out, n)!=VV_DSP_OK) return 1;
    for(size_t i=0;i<n;++i) printf("%g\n", (double)out[i]);
    vv_dsp_fir_state_free(&st);
    free(coeffs); free(in); free(out);
    return 0;
}
