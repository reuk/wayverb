#pragma once

#include "frequency_domain/envelope.h"
#include "frequency_domain/filter.h"

#include "utilities/map_to_vector.h"
#include "utilities/mapping_iterator_adapter.h"
#include "utilities/range.h"

namespace frequency_domain {

template <size_t bands_plus_one, typename It, typename Callback>
void multiband_filter(It begin,
                      It end,
                      const std::array<double, bands_plus_one>& edges,
                      const std::array<double, bands_plus_one>& widths,
                      const Callback& callback) {
    constexpr auto bands{bands_plus_one - 1};
    filter filt{static_cast<size_t>(std::distance(begin, end)) * 2};

    for (auto i{0ul}; i != bands; ++i) {
        const auto b{callback(begin, i)};
        const auto e{callback(end, i)};
        filt.run(b, e, b, [&](auto cplx, auto freq) {
            return cplx * static_cast<float>(compute_bandpass_magnitude(
                                  freq,
                                  range<double>{edges[i], edges[i + 1]},
                                  widths[i + 0],
                                  widths[i + 1],
                                  0));
        });
    }
}

template <size_t bands, typename It, typename Callback>
void multiband_filter(It begin,
                      It end,
                      range<double> audible,
                      const Callback& callback) {
    multiband_filter(begin,
                     end,
                     band_edge_frequencies<bands>(audible),
                     band_edge_widths<bands>(audible, 1),
                     callback);
}

template <typename It>
auto rms(It begin, It end) {
    return std::sqrt(std::accumulate(
            begin, end, 0.0, [](auto a, auto b) { return a + b * b; }));
}

template <size_t bands, typename It, typename Callback>
auto multiband_rms(It begin, It end, const Callback& callback) {
    std::array<double, bands> ret{};
    for (auto i{0ul}; i != bands; ++i) {
        const auto b{callback(begin, i)};
        const auto e{callback(end, i)};
        ret[i] = rms(b, e);
    }
    return ret;
}

template <size_t bands, typename T>
auto init_array(const T& t) {
    std::array<T, bands> ret{};
    std::fill(begin(ret), end(ret), t);
    return ret;
}

template <size_t bands, typename It>
auto make_multiband(It begin, It end) {
    return map_to_vector(
            begin, end, [](const auto& i) { return init_array<bands>(i); });
}

class indexer final {
public:
    indexer(size_t index)
            : index_{index} {}

    template <typename T>
    auto& operator()(T& t) const {
        return t[index_];
    }

    template <typename T>
    const auto& operator()(const T& t) const {
        return t[index_];
    }

private:
    size_t index_;
};

template <size_t bands, typename It>
auto per_band_energy(It begin, It end, range<double> audible) {
    auto multiband{make_multiband<bands>(begin, end)};
    const auto edges{band_edge_frequencies<bands>(audible)};
    const auto widths{band_edge_widths<bands>(audible, 1)};

    const auto callback{[](auto it, auto index) {
        return make_mapping_iterator_adapter(std::move(it), indexer{index});
    }};

    multiband_filter(std::begin(multiband),
                     std::end(multiband),
                     edges,
                     widths,
                     callback);
    auto rms{multiband_rms<bands>(
            std::begin(multiband), std::end(multiband), callback)};

    for (auto i{0ul}; i != bands; ++i) {
        const auto width{edges[i + 1] - edges[i]};
        rms[i] /= width;
    }

    return rms;
}

}  // namespace frequency_domain
