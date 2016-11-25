#pragma once

#include "frequency_domain/envelope.h"
#include "frequency_domain/filter.h"

#include "utilities/foldl.h"
#include "utilities/map.h"
#include "utilities/map_to_vector.h"
#include "utilities/mapping_iterator_adapter.h"
#include "utilities/range.h"

namespace frequency_domain {

template <size_t N>
struct edges_and_width_factor final {
    std::array<double, N> edges;
    double width_factor;
};

template <size_t N>
constexpr auto make_edges_and_width_factor(const std::array<double, N>& edges,
                                           double width_factor) {
    return edges_and_width_factor<N>{edges, width_factor};
}

template <size_t N>
auto compute_multiband_params(util::range<double> audible_range,
                              double overlap) {
    return make_edges_and_width_factor(band_edges<N>(audible_range),
                                       width_factor(audible_range, N, overlap));
}

////////////////////////////////////////////////////////////////////////////////

template <typename T>
size_t next_power_of_two(T t) {
    return std::ceil(std::log2(t));
}

template <typename T>
size_t best_fft_length(T t) {
    return std::pow(2, next_power_of_two(t));
}

////////////////////////////////////////////////////////////////////////////////

template <size_t bands_plus_one, typename It, typename Callback>
auto multiband_filter(It b,
                      It e,
                      const edges_and_width_factor<bands_plus_one>& params,
                      const Callback& callback,
                      size_t l = 0) {
    constexpr auto bands = bands_plus_one - 1;

    //  A bit of extra padding here so that discontinuities at the end get
    //  truncated away.
    const auto bins = best_fft_length(std::distance(b, e)) << 2;
    filter filt{bins};

    //  Will store the area under each frequency-domain window.
    std::array<double, bands> summed_squared{};
    std::array<double, bands> integrated_envelopes{};

    for (auto i = 0ul; i != bands; ++i) {
        const auto mapping_b = callback(b, i);
        const auto mapping_e = callback(e, i);
        filt.run(mapping_b, mapping_e, mapping_b, [&](auto cplx, auto freq) {
            const auto amp = compute_bandpass_magnitude(
                    freq,
                    util::make_range(params.edges[i + 0], params.edges[i + 1]),
                    params.width_factor,
                    l);
            integrated_envelopes[i] += amp;
            const auto ret = cplx * static_cast<float>(amp);
            const auto abs_ret = std::abs(ret);
            summed_squared[i] += abs_ret * abs_ret;
            return ret;
        });
    }

    std::array<double, bands> normalized_rms{};
    for (auto i = 0; i != bands; ++i) {
        normalized_rms[i] = integrated_envelopes[i] ? 
                std::sqrt(summed_squared[i] / integrated_envelopes[i]) : 0;
    }

    return normalized_rms;
}

template <typename It>
auto square_sum(It b, It e) {
    return std::accumulate(
            b, e, std::decay_t<decltype(*b)>(), [](auto a, auto b) {
                return a + b * b;
            });
}

template <typename It>
auto rms(It b, It e) {
    using std::sqrt;
    return sqrt(square_sum(b, e));
}

template <size_t bands, typename T>
auto init_array(const T& t) {
    std::array<T, bands> ret{};
    std::fill(begin(ret), end(ret), t);
    return ret;
}

template <size_t bands, typename It>
auto make_multiband(It begin, It end) {
    return util::map_to_vector(
            begin, end, [](const auto& i) { return init_array<bands>(i); });
}

class indexer final {
public:
    constexpr explicit indexer(size_t index)
            : index_{index} {}

    template <typename T>
    constexpr auto& operator()(T& t) const {
        return t[index_];
    }

    template <typename T>
    constexpr const auto& operator()(const T& t) const {
        return t[index_];
    }

private:
    size_t index_;
};

struct make_indexer_iterator final {
    template <typename It>
    constexpr auto operator()(It it, size_t index) const {
        return util::make_mapping_iterator_adapter(std::move(it),
                                                   indexer{index});
    }
};

template <size_t bands_plus_one, typename It>
auto per_band_energy(It begin,
                     It end,
                     const edges_and_width_factor<bands_plus_one>& params) {
    constexpr auto bands = bands_plus_one - 1;

    auto multiband = make_multiband<bands>(begin, end);

    const auto rms = multiband_filter(std::begin(multiband),
                                      std::end(multiband),
                                      params,
                                      make_indexer_iterator{});

    return rms;
}

}  // namespace frequency_domain
