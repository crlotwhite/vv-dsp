#include <vector>
#include <cassert>
#include "vv_dsp/adapters/cpp_wrapper.hpp"

int main(){
    using namespace vv::dsp;
    std::vector<Real> x{1,2,3,4};
    auto m = Math::mean(std::span<const Real>(x.data(), x.size()));
    assert(m > 2.49 && m < 2.51);
    auto v = Math::variance(std::span<const Real>(x.data(), x.size()));
    // variance of 1..4 is 1.6666.. (sample variance with N-1)
    assert(v > 1.6 && v < 1.8);
    auto mn = Math::min(std::span<const Real>(x.data(), x.size()));
    auto mx = Math::max(std::span<const Real>(x.data(), x.size()));
    assert(mn==1 && mx==4);
    // Strided view: take every other element starting at index 0 => {1,3}
    Math::StridedSpan<const Real> xs(x.data(), 2, 2);
    assert(Math::mean(xs) > 1.9 && Math::mean(xs) < 2.1);
    return 0;
}
