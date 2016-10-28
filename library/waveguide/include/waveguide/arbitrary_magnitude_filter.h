#pragma once

#include "common/filter_coefficients.h"

#include "itpp/signal/filter_design.h"

namespace waveguide {

namespace detail {

template <size_t I>
auto to_itpp_vec(const std::array<double, I>& x) {
    itpp::vec ret(I);
    for (auto i = 0; i != I; ++i) {
        ret[i] = x[i];
    }
    return ret;
}

template <size_t... Ix>
auto to_array(const itpp::vec& x, std::index_sequence<Ix...>) {
    return std::array<double, sizeof...(Ix)>{{x[Ix]...}};
}

template <size_t I>
auto to_array(const itpp::vec& x) {
    if (x.size() != I) {
        throw std::runtime_error{
                "itpp vec cannot be converted to array with specified size"};
    }

    return to_array(x, std::make_index_sequence<I>{});
}

}  // namespace detail

/// Given a sorted array of frequencies from 0-1 (dc to nyquist) and an array of
/// amplitudes, create an IIR filter of order N which approximates this
/// frequency response.
template <size_t N, size_t I>
auto arbitrary_magnitude_filter(const std::array<double, I>& f,
                                const std::array<double, I>& m) {
    itpp::vec b;
    itpp::vec a;
    yulewalk(N, detail::to_itpp_vec(f), detail::to_itpp_vec(m), b, a);
    return make_filter_coefficients(detail::to_array<N + 1>(b),
                                    detail::to_array<N + 1>(a));
}

}  // namespace waveguide
