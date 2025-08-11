#include <vector>
#include <cassert>
#include "vv_dsp/adapters/cpp_wrapper.hpp"

int main(){
    using namespace vv::dsp;
    std::vector<Real> x{1,2,3,4};
    auto m = Math::mean(std::span<const Real>(x.data(), x.size()));
    assert(m > Real(2.49f) && m < Real(2.51f));
    auto v = Math::variance(std::span<const Real>(x.data(), x.size()));
    // variance of 1..4 is 1.6666.. (sample variance with N-1)
    assert(v > Real(1.6f) && v < Real(1.8f));
    auto mn = Math::min(std::span<const Real>(x.data(), x.size()));
    auto mx = Math::max(std::span<const Real>(x.data(), x.size()));
    assert(mn==1 && mx==4);
    // Strided view: take every other element starting at index 0 => {1,3}
    Math::StridedSpan<const Real> xs(x.data(), 2, 2);
    assert(Math::mean(xs) > Real(1.9f) && Math::mean(xs) < Real(2.1f));
    return 0;
}
