#include <vector>
#include <cassert>
#include "vv_dsp/adapters/cpp_wrapper.hpp"

int main(){
    using namespace vv::dsp;
    // Window
    auto w = WindowGenerator::hann(16);
    assert(w.size()==16);
    // FIR
    std::vector<Real> coeffs(8, 0);
    coeffs[0] = 1; // passthrough (very crude)
    FIRFilter fir(coeffs);
    std::vector<Real> x(16, 1), y;
    y = fir.process(x);
    // Resampler construction
    Resampler rs(2, 1);
    std::vector<Real> out(64, 0);
    size_t outn = rs.process(x, std::span<Real>(out.data(), out.size()));
    (void)outn;
    return 0;
}
