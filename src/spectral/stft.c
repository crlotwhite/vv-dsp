#include "vv_dsp/spectral/stft.h"
#include "vv_dsp/window.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

struct vv_dsp_stft {
    size_t nfft;
    size_t hop;
    vv_dsp_stft_window win_type;
    vv_dsp_real* win;      // window coefficients length nfft
    vv_dsp_real* timebuf;  // temp buffer length nfft
    vv_dsp_fft_plan* plan_f;
    vv_dsp_fft_plan* plan_b;
};

static vv_dsp_status make_window(vv_dsp_stft_window wt, size_t n, vv_dsp_real* out) {
    switch (wt) {
        case VV_DSP_STFT_WIN_BOXCAR: return vv_dsp_window_boxcar(n, out);
        case VV_DSP_STFT_WIN_HANN: return vv_dsp_window_hann(n, out);
        case VV_DSP_STFT_WIN_HAMMING: return vv_dsp_window_hamming(n, out);
        default: return VV_DSP_ERROR_OUT_OF_RANGE;
    }
}

VV_DSP_NODISCARD vv_dsp_status vv_dsp_stft_create(const vv_dsp_stft_params* params, vv_dsp_stft** out) {
    if (!out || !params) return VV_DSP_ERROR_NULL_POINTER;
    *out = NULL;
    if (params->fft_size == 0 || params->hop_size == 0 || params->hop_size > params->fft_size)
        return VV_DSP_ERROR_INVALID_SIZE;
    vv_dsp_stft* h = (vv_dsp_stft*)calloc(1, sizeof(*h));
    if (!h) return VV_DSP_ERROR_INTERNAL;
    h->nfft = params->fft_size;
    h->hop  = params->hop_size;
    h->win_type = params->window;
    h->win = (vv_dsp_real*)malloc(sizeof(vv_dsp_real)*h->nfft);
    h->timebuf = (vv_dsp_real*)malloc(sizeof(vv_dsp_real)*h->nfft);
    if (!h->win || !h->timebuf) { free(h->win); free(h->timebuf); free(h); return VV_DSP_ERROR_INTERNAL; }
    vv_dsp_status s = make_window(h->win_type, h->nfft, h->win);
    if (s != VV_DSP_OK) { free(h->win); free(h->timebuf); free(h); return s; }

    // Per-sample normalization will be accumulated at reconstruction time via norm_add buffer.

    // FFT plans for C2C forward/backward
    if (vv_dsp_fft_make_plan(h->nfft, VV_DSP_FFT_C2C, VV_DSP_FFT_FORWARD, &h->plan_f) != VV_DSP_OK) {
        free(h->win); free(h->timebuf); free(h); return VV_DSP_ERROR_INTERNAL; }
    if (vv_dsp_fft_make_plan(h->nfft, VV_DSP_FFT_C2C, VV_DSP_FFT_BACKWARD, &h->plan_b) != VV_DSP_OK) {
        vv_dsp_fft_destroy(h->plan_f); free(h->win); free(h->timebuf); free(h); return VV_DSP_ERROR_INTERNAL; }

    *out = h; return VV_DSP_OK;
}

vv_dsp_status vv_dsp_stft_destroy(vv_dsp_stft* h) {
    if (!h) return VV_DSP_ERROR_NULL_POINTER;
    vv_dsp_fft_destroy(h->plan_f);
    vv_dsp_fft_destroy(h->plan_b);
    free(h->win);
    free(h->timebuf);
    free(h);
    return VV_DSP_OK;
}

VV_DSP_NODISCARD vv_dsp_status vv_dsp_stft_process(vv_dsp_stft* h,
                                                   const vv_dsp_real* in,
                                                   vv_dsp_cpx* out) {
    if (!h || !in || !out) return VV_DSP_ERROR_NULL_POINTER;
    // Apply window into temp complex buffer (real as re, 0 as im)
    vv_dsp_cpx* tmp = (vv_dsp_cpx*)malloc(sizeof(vv_dsp_cpx)*h->nfft);
    if (!tmp) return VV_DSP_ERROR_INTERNAL;
    for (size_t i=0;i<h->nfft;++i) {
        vv_dsp_real v = in[i] * h->win[i];
        tmp[i].re = v; tmp[i].im = 0;
    }
    vv_dsp_status s = vv_dsp_fft_execute(h->plan_f, tmp, out);
    free(tmp);
    return s;
}

// Reconstruct: IFFT then window and overlap-add into out_add
VV_DSP_NODISCARD vv_dsp_status vv_dsp_stft_reconstruct(vv_dsp_stft* h,
                                                       const vv_dsp_cpx* in,
                                                       vv_dsp_real* out_add,
                                                       vv_dsp_real* norm_add) {
    if (!h || !in || !out_add) return VV_DSP_ERROR_NULL_POINTER;
    vv_dsp_cpx* time = (vv_dsp_cpx*)malloc(sizeof(vv_dsp_cpx)*h->nfft);
    if (!time) return VV_DSP_ERROR_INTERNAL;
    vv_dsp_status s = vv_dsp_fft_execute(h->plan_b, in, time);
    if (s != VV_DSP_OK) { free(time); return s; }
    for (size_t i=0;i<h->nfft;++i) {
        vv_dsp_real w = h->win[i];
        vv_dsp_real v = time[i].re * w;
        out_add[i] += v; // caller manages buffer position and zeroing
        if (norm_add) norm_add[i] += w*w;
    }
    free(time);
    return VV_DSP_OK;
}

VV_DSP_NODISCARD vv_dsp_status vv_dsp_stft_spectrogram(vv_dsp_stft* h,
                                                       const vv_dsp_real* signal,
                                                       size_t n,
                                                       vv_dsp_real* out_mag,
                                                       size_t* out_frames) {
    if (!h || !signal || !out_mag || !out_frames) return VV_DSP_ERROR_NULL_POINTER;
    if (h->nfft == 0 || h->hop == 0) return VV_DSP_ERROR_INVALID_SIZE;
    size_t frames = (n < h->nfft) ? 1 : (1 + (n - h->nfft + h->hop) / h->hop);
    *out_frames = frames;
    vv_dsp_cpx* spec = (vv_dsp_cpx*)malloc(sizeof(vv_dsp_cpx)*h->nfft);
    vv_dsp_real* frame = (vv_dsp_real*)malloc(sizeof(vv_dsp_real)*h->nfft);
    if (!spec || !frame) { free(spec); free(frame); return VV_DSP_ERROR_INTERNAL; }
    for (size_t f=0; f<frames; ++f) {
        size_t start = f * h->hop;
        // gather frame with zero-padding at end
        for (size_t i=0;i<h->nfft;++i) {
            size_t idx = start + i;
            frame[i] = (idx < n) ? signal[idx] : (vv_dsp_real)0;
        }
        vv_dsp_status s = vv_dsp_stft_process(h, frame, spec);
        if (s != VV_DSP_OK) { free(spec); free(frame); return s; }
        for (size_t k=0;k<h->nfft;++k) {
            vv_dsp_real re = spec[k].re, im = spec[k].im;
            out_mag[f*h->nfft + k] = (vv_dsp_real)sqrt(re*re + im*im);
        }
    }
    free(spec); free(frame);
    return VV_DSP_OK;
}
