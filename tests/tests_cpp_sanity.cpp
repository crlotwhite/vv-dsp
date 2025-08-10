#include <cstdio>
#include <vector>
#include <complex>
#include "vv_dsp/adapters/cpp_wrapper.hpp"

int main() {
    // Just compile-time smoke test for now
    std::vector<vv::dsp::Real> buf(8, 0.0f);
    (void)buf;
    // Touch a C API symbol to ensure linkage
    vv_dsp_fft_plan* plan = nullptr;
    (void)plan;
    std::puts("vv-dsp C++ sanity OK");
    return 0;
}
