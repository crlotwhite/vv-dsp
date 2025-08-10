#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vv_dsp/filter.h"

static void usage(const char* p){
    fprintf(stderr, "Usage: %s --num-taps N --cutoff C --win hann|hamming|boxcar|blackman\n", p);
}

int main(int argc, char** argv){
    size_t num_taps = 33; float cutoff = 0.25f; const char* win="hann";
    for (int i=1;i<argc;++i){
        if(!strcmp(argv[i],"--num-taps")&&i+1<argc) num_taps=strtoul(argv[++i],NULL,10);
        else if(!strcmp(argv[i],"--cutoff")&&i+1<argc) cutoff=strtof(argv[++i],NULL);
        else if(!strcmp(argv[i],"--win")&&i+1<argc) win=argv[++i];
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
    for(size_t i=0;i<num_taps;++i) printf("%g\n", (double)coeffs[i]);
    free(coeffs);
    return 0;
}
