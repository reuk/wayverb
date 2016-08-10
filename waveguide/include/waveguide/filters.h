#pragma once

#include "common/cl_include.h"
#include "common/decibels.h"
#include "common/stl_wrappers.h"

#include <functional>

namespace waveguide {
namespace filters {

using real = cl_double;

constexpr size_t biquad_order = 2;

/// Just an array of reals to use as a delay line.
template <size_t o>
struct alignas(1 << 3) memory final {
    static constexpr size_t order = o;

    real array[order]{};
};

template <size_t D>
inline bool operator==(const memory<D>& a, const memory<D>& b) {
    return proc::equal(a.array, std::begin(b.array));
}

template <size_t D>
inline bool operator!=(const memory<D>& a, const memory<D>& b) {
    return !(a == b);
}

using biquad_memory = memory<biquad_order>;

//----------------------------------------------------------------------------//

/// IIR filter coefficient storage.
template <size_t o>
struct alignas(1 << 3) coefficients final {
    static constexpr size_t order = o;

    real b[order + 1]{};
    real a[order + 1]{};
};

template <size_t D>
inline bool operator==(const coefficients<D>& a, const coefficients<D>& b) {
    return proc::equal(a.a, std::begin(b.a)) &&
           proc::equal(a.b, std::begin(b.b));
}

template <size_t D>
inline bool operator!=(const coefficients<D>& a, const coefficients<D>& b) {
    return !(a == b);
}

//----------------------------------------------------------------------------//

using biquad_coefficients = coefficients<biquad_order>;

/// Several biquad delay lines in a row.
struct alignas(1 << 3) biquad_memory_array final {
    static constexpr size_t biquad_sections{3};
    biquad_memory array[biquad_sections]{};
};

constexpr auto biquad_sections = biquad_memory_array::biquad_sections;

//----------------------------------------------------------------------------//

/// Several sets of biquad parameters.
struct alignas(1 << 3) biquad_coefficients_array final {
    static constexpr size_t biquad_sections =
            biquad_memory_array::biquad_sections;
    biquad_coefficients array[biquad_sections]{};
};

//----------------------------------------------------------------------------//

using canonical_memory =
        memory<biquad_memory::order * biquad_memory_array::biquad_sections>;
using canonical_coefficients =
        coefficients<biquad_coefficients::order *
                     biquad_coefficients_array::biquad_sections>;

//----------------------------------------------------------------------------//

struct descriptor final {
    double gain{0};
    double centre{0};
    double Q{0};
};

//----------------------------------------------------------------------------//

biquad_coefficients get_peak_coefficients(const descriptor& n, double sr);

using coefficient_generator = std::function<filters::biquad_coefficients(
        const filters::descriptor&, double)>;

template <size_t... Ix>
inline biquad_coefficients_array get_biquads_array(
        std::index_sequence<Ix...>,
        const std::array<descriptor,
                         biquad_coefficients_array::biquad_sections>& n,
        double sr,
        const coefficient_generator& callback) {
    return biquad_coefficients_array {{callback(std::get<Ix>(n), sr)...}};
}
inline biquad_coefficients_array get_biquads_array(
        const std::array<descriptor,
                         biquad_coefficients_array::biquad_sections>& n,
        double sr,
        const coefficient_generator& callback) {
    return get_biquads_array(
            std::make_index_sequence<
                    biquad_coefficients_array::biquad_sections>(),
            n,
            sr,
            callback);
}

biquad_coefficients_array get_peak_biquads_array(
        const std::array<descriptor,
                         biquad_coefficients_array::biquad_sections>& n,
        double sr);

template <size_t A, size_t B>
constexpr coefficients<A + B> convolve(const coefficients<A>& a,
                                       const coefficients<B>& b) {
    auto ret = coefficients<A + B>{};
    for (auto i = 0; i != A + 1; ++i) {
        for (auto j = 0; j != B + 1; ++j) {
            ret.b[i + j] += a.b[i] * b.b[j];
            ret.a[i + j] += a.a[i] * b.a[j];
        }
    }
    return ret;
}

filters::canonical_coefficients convolve(const biquad_coefficients_array& a);

template <size_t L>
constexpr bool is_stable(const std::array<double, L>& a) {
    auto rci = a[L - 1];
    if (std::abs(rci) >= 1) {
        return false;
    }

    constexpr auto next_size = L - 1;
    std::array<double, next_size> next_array;
    for (auto i = 0; i != next_size; ++i) {
        next_array[i] = (a[i] - rci * a[next_size - i]) / (1 - rci * rci);
    }
    return is_stable(next_array);
}

template <>
constexpr bool is_stable(const std::array<double, 1>& a) {
    return true;
}

template <size_t L>
constexpr bool is_stable(const coefficients<L>& coeffs) {
    std::array<double, L + 1> denom;
    proc::copy(coeffs.a, denom.begin());
    return is_stable(denom);
}

//----------------------------------------------------------------------------//

filters::canonical_coefficients to_impedance_coefficients(
        const filters::canonical_coefficients& c);

}  // namespace filters
}  // namespace waveguide
