#include <chrono>
#include <cstdio>
#include <vector>
#include <complex>
#include <cmath>
#include "vv_dsp/adapters/cpp_wrapper.hpp"

using Clock = std::chrono::high_resolution_clock;

int main(){
    using namespace vv::dsp;
    const size_t N = 1024;
    const size_t trials = 1000;

    // Prepare input
    std::vector<Complex> in(N), out(N);
    const double PI = 3.14159265358979323846;
    for(size_t i=0;i<N;++i) in[i] = Complex{(Real)std::sin(2.0*PI*(double)i/64.0), 0};

    // FFT bench
    FFTPlanner plan(N, VV_DSP_FFT_C2C, VV_DSP_FFT_FORWARD);
    auto t0 = Clock::now();
    for(size_t t=0;t<trials;++t){ plan.execute(in.data(), out.data()); }
    auto t1 = Clock::now();
    auto fft_ms = std::chrono::duration<double, std::milli>(t1-t0).count();

    // STFT bench (analysis only)
    vv_dsp_stft_params p; p.fft_size=256; p.hop_size=128; p.window=VV_DSP_STFT_WIN_HANN;
    STFTProcessor st(p);
    std::vector<Real> frame(p.fft_size);
    std::vector<Complex> spec(p.fft_size);
    t0 = Clock::now();
    for(size_t t=0;t<trials;++t){
        for(size_t i=0;i<p.fft_size;++i) frame[i] = in[i].re;
        if(st.process(frame.data(), spec.data())!=VV_DSP_OK) return 1;
    }
    t1 = Clock::now();
    auto stft_ms = std::chrono::duration<double, std::milli>(t1-t0).count();

    std::printf("vv-dsp bench: N=%zu, trials=%zu\n", N, trials);
    std::printf("FFT C2C total=%.3f ms (avg=%.6f ms)\n", fft_ms, fft_ms/trials);
    std::printf("STFT (analysis) total=%.3f ms (avg=%.6f ms)\n", stft_ms, stft_ms/trials);
    return 0;
}
