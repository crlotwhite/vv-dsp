// vv-dsp C++ RAII wrapper (skeleton)
// This header provides modern C++ wrappers around the C API for safer usage.

#pragma once

#include <span>
#include <vector>
#include <memory>
#include <stdexcept>
#include <complex>
#include <type_traits>
#include <limits>

extern "C" {
#include "vv_dsp/vv_dsp_types.h"
#include "vv_dsp/vv_dsp_math.h"
#include "vv_dsp/spectral/fft.h"
#include "vv_dsp/spectral/stft.h"
#include "vv_dsp/window.h"
#include "vv_dsp/resample.h"
#include "vv_dsp/filter.h"
}

namespace vv::dsp {

// Basic exception that maps vv_dsp_status to C++ errors
class DSPException : public std::runtime_error {
public:
    explicit DSPException(const char* msg) : std::runtime_error(msg) {}
};

// Helper to check status and throw on error
inline void check_status(vv_dsp_status st, const char* what) {
    if (st != VV_DSP_OK) {
        throw DSPException(what);
    }
}

using Real = vv_dsp_real;

// Lightweight Complex value type with operators, compatible with vv_dsp_cpx layout
struct Complex {
    Real re{};
    Real im{};
    constexpr Complex() = default;
    constexpr Complex(Real r, Real i) : re(r), im(i) {}
    explicit constexpr Complex(vv_dsp_cpx z) : re(z.re), im(z.im) {}
    constexpr operator vv_dsp_cpx() const { return vv_dsp_cpx{re, im}; }
    // Interop with std::complex
    explicit constexpr Complex(std::complex<Real> z) : re(z.real()), im(z.imag()) {}
    constexpr operator std::complex<Real>() const { return {re, im}; }

    // Basic ops
    constexpr Complex operator+(const Complex& o) const { return {Real(re + o.re), Real(im + o.im)}; }
    constexpr Complex operator-(const Complex& o) const { return {Real(re - o.re), Real(im - o.im)}; }
    constexpr Complex operator*(const Complex& o) const { return {Real(re*o.re - im*o.im), Real(re*o.im + im*o.re)}; }
    constexpr Complex operator/(const Complex& o) const {
        Real denom = o.re*o.re + o.im*o.im;
        return { Real((re*o.re + im*o.im)/denom), Real((im*o.re - re*o.im)/denom) };
    }
    Complex& operator+=(const Complex& o){ re = Real(re+o.re); im = Real(im+o.im); return *this; }
    Complex& operator-=(const Complex& o){ re = Real(re-o.re); im = Real(im-o.im); return *this; }
};

// Math utilities over spans/vectors
namespace Math {
    inline Real sum(std::span<const Real> x){ Real s = 0; for(auto v: x) s = Real(s+v); return s; }
    inline Real mean(std::span<const Real> x){ return x.empty()? Real(0) : Real(sum(x)/Real(x.size())); }

    // Simple strided view wrapper
    template <class T>
    class StridedSpan {
        const T* data_{}; size_t size_{}; size_t stride_{};
    public:
        StridedSpan() = default;
        StridedSpan(const T* data, size_t size, size_t stride) : data_(data), size_(size), stride_(stride) {}
        size_t size() const noexcept { return size_; }
        size_t stride() const noexcept { return stride_; }
        const T& operator[](size_t i) const { return *(data_ + i*stride_); }
        const T* data() const noexcept { return data_; }
    };

    inline Real sum(StridedSpan<const Real> x){ Real s = 0; for(size_t i=0;i<x.size();++i) s = Real(s + x[i]); return s; }
    inline Real mean(StridedSpan<const Real> x){ return x.size()==0? Real(0) : Real(sum(x)/Real(x.size())); }
    inline Real variance(std::span<const Real> x){ if(x.size()<2) return Real(0); Real m=mean(x); Real acc=0; for(auto v:x){ Real d=Real(v-m); acc = Real(acc + d*d);} return Real(acc/Real(x.size()-1)); }
    inline Real variance(StridedSpan<const Real> x){ if(x.size()<2) return Real(0); Real m=mean(x); Real acc=0; for(size_t i=0;i<x.size();++i){ Real d=Real(x[i]-m); acc = Real(acc + d*d);} return Real(acc/Real(x.size()-1)); }
    inline Real min(std::span<const Real> x){ Real mn = std::numeric_limits<Real>::infinity(); for(auto v:x) if(v<mn) mn=v; return x.empty()? Real(0):mn; }
    inline Real max(std::span<const Real> x){ Real mx = -std::numeric_limits<Real>::infinity(); for(auto v:x) if(v>mx) mx=v; return x.empty()? Real(0):mx; }
    inline Real min(StridedSpan<const Real> x){ Real mn = std::numeric_limits<Real>::infinity(); for(size_t i=0;i<x.size();++i){ Real v=x[i]; if(v<mn) mn=v;} return x.size()? mn: Real(0); }
    inline Real max(StridedSpan<const Real> x){ Real mx = -std::numeric_limits<Real>::infinity(); for(size_t i=0;i<x.size();++i){ Real v=x[i]; if(v>mx) mx=v;} return x.size()? mx: Real(0); }
}

// Unique_ptr style deleters for C handles
struct FFTPlanDeleter {
    void operator()(vv_dsp_fft_plan* p) const noexcept { if(p) (void)vv_dsp_fft_destroy(p); }
};

class FFTPlanner {
    std::unique_ptr<vv_dsp_fft_plan, FFTPlanDeleter> plan_{};
    size_t n_{}; vv_dsp_fft_type type_{}; vv_dsp_fft_dir dir_{};
public:
    FFTPlanner() = default;
    FFTPlanner(size_t n, vv_dsp_fft_type type, vv_dsp_fft_dir dir) { reset(n,type,dir); }
    void reset(size_t n, vv_dsp_fft_type type, vv_dsp_fft_dir dir){
        vv_dsp_fft_plan* raw = nullptr;
        check_status(vv_dsp_fft_make_plan(n, type, dir, &raw), "vv_dsp_fft_make_plan");
        plan_.reset(raw); n_ = n; type_ = type; dir_ = dir;
    }
    bool valid() const noexcept { return bool(plan_); }
    size_t size() const noexcept { return n_; }
    vv_dsp_fft_type type() const noexcept { return type_; }
    vv_dsp_fft_dir dir() const noexcept { return dir_; }
    // Execute: overloads for real/complex
    void execute(const Real* in, Complex* out) const {
        check_status(vv_dsp_fft_execute(plan_.get(), in, out), "vv_dsp_fft_execute R2C/C2R");
    }
    void execute(const Complex* in, Complex* out) const {
        check_status(vv_dsp_fft_execute(plan_.get(), in, out), "vv_dsp_fft_execute C2C");
    }
};

struct STFTDeleter { void operator()(vv_dsp_stft* p) const noexcept { if(p) (void)vv_dsp_stft_destroy(p); } };
class STFTProcessor {
    std::unique_ptr<vv_dsp_stft, STFTDeleter> h_{};
public:
    STFTProcessor() = default;
    explicit STFTProcessor(const vv_dsp_stft_params& p){ reset(p); }
    void reset(const vv_dsp_stft_params& p){ vv_dsp_stft* raw=nullptr; check_status(vv_dsp_stft_create(&p,&raw), "vv_dsp_stft_create"); h_.reset(raw);}    
    bool valid() const noexcept { return bool(h_); }
    vv_dsp_status process(const Real* in, Complex* out){ return vv_dsp_stft_process(h_.get(), in, reinterpret_cast<vv_dsp_cpx*>(out)); }
    vv_dsp_status reconstruct(const Complex* in, Real* out_add, Real* norm_add){ return vv_dsp_stft_reconstruct(h_.get(), reinterpret_cast<const vv_dsp_cpx*>(in), out_add, norm_add); }
};

// Window helpers returning std::vector
struct WindowGenerator {
    static std::vector<Real> hann(size_t N){ std::vector<Real> w(N); check_status(vv_dsp_window_hann(N, w.data()), "vv_dsp_window_hann"); return w; }
    static std::vector<Real> hamming(size_t N){ std::vector<Real> w(N); check_status(vv_dsp_window_hamming(N, w.data()), "vv_dsp_window_hamming"); return w; }
    static std::vector<Real> blackman(size_t N){ std::vector<Real> w(N); check_status(vv_dsp_window_blackman(N, w.data()), "vv_dsp_window_blackman"); return w; }
};

// Resampler RAII
struct ResamplerDeleter { void operator()(vv_dsp_resampler* p) const noexcept { if(p) vv_dsp_resampler_destroy(p); } };
class Resampler {
    std::unique_ptr<vv_dsp_resampler, ResamplerDeleter> h_{};
public:
    Resampler() = default;
    Resampler(unsigned num, unsigned den){ reset(num, den); }
    void reset(unsigned num, unsigned den){ auto* raw = vv_dsp_resampler_create(num, den); if(!raw) throw DSPException("vv_dsp_resampler_create"); h_.reset(raw);}    
    void set_ratio(unsigned num, unsigned den){ if(vv_dsp_resampler_set_ratio(h_.get(), num, den)!=0) throw DSPException("vv_dsp_resampler_set_ratio"); }
    void set_quality(bool use_sinc, unsigned taps){ if(vv_dsp_resampler_set_quality(h_.get(), use_sinc?1:0, taps)!=0) throw DSPException("vv_dsp_resampler_set_quality"); }
    size_t process(std::span<const Real> in, std::span<Real> out){ size_t out_n=0; if(vv_dsp_resampler_process_real(h_.get(), in.data(), in.size(), out.data(), out.size(), &out_n)!=0) throw DSPException("vv_dsp_resampler_process_real"); return out_n; }
};

// Minimal FIR state wrapper
class FIRFilter {
    vv_dsp_fir_state state_{};
    std::vector<Real> coeffs_;
public:
    FIRFilter() = default;
    explicit FIRFilter(std::vector<Real> coeffs) : coeffs_(std::move(coeffs)){
        check_status(vv_dsp_fir_state_init(&state_, coeffs_.size()), "vv_dsp_fir_state_init");
    }
    ~FIRFilter(){ vv_dsp_fir_state_free(&state_); }
    std::vector<Real> process(std::span<const Real> input){ std::vector<Real> out(input.size()); check_status(vv_dsp_fir_apply(&state_, coeffs_.data(), input.data(), out.data(), input.size()), "vv_dsp_fir_apply"); return out; }
};

class IIRFilter { /* TODO: add when IIR API is defined */ };
class Biquad {
    vv_dsp_biquad s_{};
public:
    Biquad(Real b0, Real b1, Real b2, Real a1, Real a2){ check_status(vv_dsp_biquad_init(&s_, b0,b1,b2,a1,a2), "vv_dsp_biquad_init"); }
    void reset(){ vv_dsp_biquad_reset(&s_); }
    Real process(Real x){ return vv_dsp_biquad_process(&s_, x); }
};

} // namespace vv::dsp
