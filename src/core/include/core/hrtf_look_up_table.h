#pragma once

#include "hrtf_entries.h"

#include "core/vector_look_up_table.h"

#include <algorithm>
#include <numeric>

namespace wayverb {
namespace core {
namespace hrtf_look_up_table {

struct azimuth final {
    template <typename T>
    constexpr auto operator()(const T& t) const {
        return t.azimuth;
    }
};

struct elevation final {
    template <typename T>
    constexpr auto operator()(const T& t) const {
        return t.elevation;
    }
};

template <typename Extractor>
constexpr auto smallest_nonzero(const hrtf_data::entry* it,
                                const hrtf_data::entry* end,
                                Extractor extractor) {
    auto ret = 360;
    for (; it != end; ++it) {
        const auto extracted = extractor(*it);
        ret = extracted ? std::min(ret, extracted) : ret;
    }
    return ret;
}

template <typename Extractor>
constexpr auto all_divisible(const hrtf_data::entry* it,
                             const hrtf_data::entry* end,
                             int divisor,
                             Extractor extractor) {
    for (; it != end; ++it) {
        if (extractor(*it) % divisor) {
            return false;
        }
    }
    return true;
}

template <typename Extractor>
constexpr auto find_inc(const hrtf_data::entry* it,
                        const hrtf_data::entry* end,
                        Extractor extractor) {
    const auto ret = smallest_nonzero(it, end, extractor);
    if (!all_divisible(it, end, ret, extractor)) {
        throw std::runtime_error{"items must be equally spaced"};
    }
    return ret;
}

constexpr auto b = std::begin(hrtf_data::entries);
constexpr auto e = std::end(hrtf_data::entries);

constexpr auto az_num = 360 / find_inc(b, e, azimuth{});
constexpr auto el_num = (180 / find_inc(b, e, elevation{})) - 1;

constexpr auto generate_hrtf_table() {
    using hrtf_table =
            vector_look_up_table<std::array<std::array<double, 8>, 2>,
                                 az_num,
                                 el_num>;

    hrtf_table ret{};

    for (auto it = b; it != e; ++it) {
        const auto azel = az_el{static_cast<float>(azimuth{}(*it)),
                                static_cast<float>(elevation{}(*it))};
        ret.at(hrtf_table::angles_to_indices(azel)) = it->energy;
    }

    return ret;
}

constexpr auto table = generate_hrtf_table();

}  // namespace hrtf_look_up_table
}  // namespace core
}  // namespace wayverb
