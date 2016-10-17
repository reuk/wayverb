#pragma once

#include "frequency_domain/envelope.h"
#include "frequency_domain/filter.h"

#include "utilities/map_to_vector.h"
#include "utilities/mapping_iterator_adapter.h"
#include "utilities/range.h"

namespace frequency_domain {

template <size_t bands_plus_one, typename It, typename Callback>
void multiband_filter(
        It begin,
        It end,
        const std::array<edge_and_width, bands_plus_one>& edges_and_widths,
        const Callback& callback) {
    constexpr auto bands{bands_plus_one - 1};
    filter filt{2 * static_cast<size_t>(std::distance(begin, end))};

    for (auto i{0ul}; i != bands; ++i) {
        const auto b{callback(begin, i)};
        const auto e{callback(end, i)};
        filt.run(b, e, b, [&](auto cplx, auto freq) {
            return cplx * static_cast<float>(compute_bandpass_magnitude(
                                  freq,
                                  edges_and_widths[i + 0],
                                  edges_and_widths[i + 1],
                                  0));
        });
    }
}

template <typename It>
auto square_sum(It b, It e) {
    return std::accumulate(b, e, std::decay_t<decltype(*b)>(),
    [](auto a, auto b) { return a + b * b; });
}

template <typename It>
auto rms(It b, It e) {
    using std::sqrt;
    return sqrt(square_sum(b, e));
}

template <typename It, typename Callback>
auto band_rms(It begin, It end, const Callback& callback, size_t band) {
    const auto b{callback(begin, band)};
    const auto e{callback(end, band)};
    return rms(b, e);
}

template <typename It, typename Callback, size_t... Ix>
auto multiband_rms(It begin,
                   It end,
                   const Callback& callback,
                   std::index_sequence<Ix...>) {
    return std::array<double, sizeof...(Ix)>{
            {band_rms(begin, end, callback, Ix)...}};
}

template <size_t bands, typename It, typename Callback>
auto multiband_rms(It begin, It end, const Callback& callback) {
    return multiband_rms(
            begin, end, callback, std::make_index_sequence<bands>{});
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

template <size_t bands_plus_one, typename It>
auto per_band_energy(
        It begin,
        It end,
        const std::array<edge_and_width, bands_plus_one>& edges_and_widths) {
    constexpr auto bands{bands_plus_one - 1};

    auto multiband{make_multiband<bands>(begin, end)};

    const auto callback{[](auto it, auto index) {
        return make_mapping_iterator_adapter(std::move(it), indexer{index});
    }};

    multiband_filter(std::begin(multiband),
                     std::end(multiband),
                     edges_and_widths,
                     callback);
    auto rms{multiband_rms<bands>(
            std::begin(multiband), std::end(multiband), callback)};

    for (auto i{0ul}; i != bands; ++i) {
        const auto width{edges_and_widths[i + 1].edge -
                         edges_and_widths[i].edge};
        rms[i] /= width;
    }

    return rms;
}

}  // namespace frequency_domain
