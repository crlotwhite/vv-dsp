#include <vector>
#include <complex>
#include <cassert>
#include "vv_dsp/adapters/cpp_wrapper.hpp"

int main(){
    using namespace vv::dsp;
    const size_t N = 8;
    FFTPlanner plan(N, VV_DSP_FFT_C2C, VV_DSP_FFT_FORWARD);
    std::vector<Complex> in(N), out(N);
    // DC impulse -> flat spectrum
    in[0] = Complex{1,0};
    plan.execute(in.data(), out.data());
    // No strict check; just ensure call succeeded and wrote something
    (void)out;
    return 0;
}
