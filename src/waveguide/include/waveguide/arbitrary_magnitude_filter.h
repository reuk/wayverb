#pragma once

#include "waveguide/frequency_domain_envelope.h"

#include "core/cosine_interp.h"
#include "core/filter_coefficients.h"

#include "itpp/signal/filter_design.h"

namespace waveguide {

namespace detail {

template <typename It>
auto to_itpp_vec(It b, It e) {
    itpp::vec ret(std::distance(b, e));
    for (auto i = 0; b != e; ++b, ++i) {
        ret[i] = *b;
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

////////////////////////////////////////////////////////////////////////////////

constexpr auto make_interp_point(const frequency_domain_envelope::point& pt) {
    struct ret final {
        double x;
        double y;
    };
    return ret{pt.frequency, pt.amplitude};
}

template <typename It>
constexpr auto make_interp_iterator(It it) {
    return util::make_mapping_iterator_adapter(std::move(it),
                                               make_interp_point);
}

/// Given a sorted array of frequencies from 0-1 (dc to nyquist) and an array of
/// amplitudes, create an IIR filter of order N which approximates this
/// frequency response.
template <size_t N>
auto arbitrary_magnitude_filter(frequency_domain_envelope env) {
    remove_outside_frequency_range(env, util::make_range(0.0, 1.0));
    env.insert(frequency_domain_envelope::point{0.0, 0.0});
    env.insert(frequency_domain_envelope::point{1.0, 0.0});

    const auto new_envelope = [&] {
        const auto npts = 256;
        frequency_domain_envelope ret;
        for (auto i = 0; i != npts; ++i) {
            const auto frequency = i / (npts - 1.0);
            ret.insert(frequency_domain_envelope::point{
                    frequency,
                    interp(make_interp_iterator(env.cbegin()),
                           make_interp_iterator(env.cend()),
                           frequency,
                           core::linear_interp_functor{})});
        }
        return ret;
    }();

    itpp::vec b;
    itpp::vec a;
    yulewalk(N,
             detail::to_itpp_vec(make_frequency_iterator(new_envelope.cbegin()),
                                 make_frequency_iterator(new_envelope.cend())),
             detail::to_itpp_vec(make_amplitude_iterator(new_envelope.cbegin()),
                                 make_amplitude_iterator(new_envelope.cend())),
             b,
             a);
    return core::make_filter_coefficients(detail::to_array<N + 1>(b),
                                          detail::to_array<N + 1>(a));
}

}  // namespace waveguide
