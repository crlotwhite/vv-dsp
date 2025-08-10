#include <vector>
#include <cassert>
#include "vv_dsp/adapters/cpp_wrapper.hpp"

int main(){
    using namespace vv::dsp;
    // Simple biquad: pass-through (b0=1, b1=b2=a1=a2=0)
    Biquad biq(1,0,0,0,0);
    std::vector<Real> x(16, 1), y(16, 0);
    for(size_t i=0;i<x.size();++i) y[i] = biq.process(x[i]);
    // Expect output ~1
    for(auto v: y) assert(v==1);
    return 0;
}
