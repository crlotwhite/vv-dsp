#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vv_dsp/spectral.h"

static void usage(const char* p){
    fprintf(stderr, "Usage: %s --fft N --hop H --win hann|hamming|boxcar --n SAMPLES [--infile PATH] [--seed SEED]\n", p);
}

int main(int argc, char** argv){
    size_t fft=256, hop=128, n=2048; const char* win="hann"; unsigned int seed=0; const char* infile=NULL;
    for(int i=1;i<argc;++i){
        if(!strcmp(argv[i],"--fft")&&i+1<argc) fft=strtoul(argv[++i],NULL,10);
        else if(!strcmp(argv[i],"--hop")&&i+1<argc) hop=strtoul(argv[++i],NULL,10);
        else if(!strcmp(argv[i],"--win")&&i+1<argc) win=argv[++i];
        else if(!strcmp(argv[i],"--n")&&i+1<argc) n=strtoul(argv[++i],NULL,10);
        else if(!strcmp(argv[i],"--seed")&&i+1<argc) seed=(unsigned)strtoul(argv[++i],NULL,10);
        else if(!strcmp(argv[i],"--infile")&&i+1<argc) infile=argv[++i];
        else { usage(argv[0]); return 2; }
    }
    vv_dsp_stft_window w = VV_DSP_STFT_WIN_HANN;
    if(!strcmp(win,"hann")) w = VV_DSP_STFT_WIN_HANN; else if(!strcmp(win,"hamming")) w=VV_DSP_STFT_WIN_HAMMING; else if(!strcmp(win,"boxcar")) w=VV_DSP_STFT_WIN_BOXCAR; else { usage(argv[0]); return 2; }

    srand(seed);
    vv_dsp_real* sig = (vv_dsp_real*)malloc(n*sizeof(vv_dsp_real));
    vv_dsp_real* recon = (vv_dsp_real*)calloc(n, sizeof(vv_dsp_real));
    vv_dsp_real* norm  = (vv_dsp_real*)calloc(n, sizeof(vv_dsp_real));
    if(!sig||!recon||!norm) return 1;
    if(infile){
        FILE* f = fopen(infile, "r"); if(!f){ perror("fopen infile"); return 1; }
        for(size_t i=0;i<n;++i){ if(fscanf(f, "%f", &sig[i])!=1){ fclose(f); return 1; } }
        fclose(f);
    } else {
        for(size_t i=0;i<n;++i) sig[i] = (float)rand()/RAND_MAX*2.f-1.f;
    }

    vv_dsp_stft_params params = { fft, hop, w };
    vv_dsp_stft* h = NULL;
    if(vv_dsp_stft_create(&params, &h)!=VV_DSP_OK || !h) return 1;

    vv_dsp_cpx* spec = (vv_dsp_cpx*)malloc(fft*sizeof(vv_dsp_cpx));
    if(!spec) return 1;
    // Iterate frames
    for(size_t f=0; f*hop + fft <= n; ++f){
        if(vv_dsp_stft_process(h, sig + f*hop, spec)!=VV_DSP_OK) return 1;
        if(vv_dsp_stft_reconstruct(h, spec, recon + f*hop, norm + f*hop)!=VV_DSP_OK) return 1;
    }
    // Normalize OLA and print exactly n samples
    size_t outN = n;
    for(size_t i=0;i<outN;++i){
        vv_dsp_real den = norm[i];
        vv_dsp_real y = den > (vv_dsp_real)1e-12 ? recon[i] / den : (vv_dsp_real)0;
        printf("%g\n", (double)y);
    }
    free(spec); free(sig); free(recon); free(norm);
    vv_dsp_stft_destroy(h);
    return 0;
}
