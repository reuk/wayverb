#include "core/kernel.h"

namespace wayverb {
namespace core {
namespace kernels {

namespace {
template <typename T>
auto square(T t) {
    return t * t;
}
}

double gaussian(double t, double o) {
    return std::exp(-square(t) / (2.0 * square(o)));
}

double sin_modulated_gaussian(double t, double o) {
    return -gaussian(t, o) * std::sin(t / o);
}

double gaussian_dash(double t, double o) {
    return -t * gaussian(t, o) / square(o);
}

double ricker(double t, double f) {
    //  see
    //  http://docs.scipy.org/doc/scipy/reference/generated/scipy.signal.ricker.html
    //  and http://wiki.seg.org/wiki/Dictionary:Ricker_wavelet
    const auto u = square(M_PI * f * t);
    return (1.0 - 2.0 * u) * std::exp(-u);
}

}  // namespace kernels
}  // namespace core
}  // namespace wayverb
