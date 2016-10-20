#pragma once

#include "hrtf_entries.h"

#include "utilities/reduce.h"

#include <algorithm>
#include <numeric>

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

constexpr auto az_inc = find_inc(std::begin(hrtf_data::entries),
                                 std::end(hrtf_data::entries),
                                 azimuth{});
constexpr auto el_inc = find_inc(std::begin(hrtf_data::entries),
                                 std::end(hrtf_data::entries),
                                 elevation{});

constexpr auto az_num = 360 / az_inc;
constexpr auto el_num = 360 / el_inc;

constexpr size_t index_from_azimuth(int az) { return (az % 360) / az_inc; }

constexpr int azimuth_from_index(size_t ind) { return ind * az_inc; }

constexpr size_t index_from_elevation(int el) { return (el % 360) / el_inc; }

constexpr int elevation_from_index(size_t ind) { return ind * el_inc; }

constexpr auto energy_for_angles(const hrtf_data::entry* it,
                                 const hrtf_data::entry* end,
                                 int azimuth,
                                 int elevation) {
    for (; it != end; ++it) {
        if (it->azimuth == azimuth && it->elevation == elevation) {
            return it->energy;
        }
    }
    return decltype(it->energy){};
}

constexpr auto energy_for_indices(const hrtf_data::entry* it,
                                  const hrtf_data::entry* end,
                                  size_t az,
                                  size_t el) {
    return energy_for_angles(
            it, end, azimuth_from_index(az), elevation_from_index(el));
}

template <size_t... Ix>
constexpr auto generate_look_up_array(const hrtf_data::entry* it,
                                      const hrtf_data::entry* end,
                                      size_t azimuth_index,
                                      std::index_sequence<Ix...>) {
    using value_type = decltype(hrtf_data::entries[0].energy);
    return std::array<value_type, sizeof...(Ix)>{
            {energy_for_indices(it, end, azimuth_index, Ix)...}};
}

constexpr auto generate_look_up_array(const hrtf_data::entry* it,
                                      const hrtf_data::entry* end,
                                      size_t azimuth_index) {
    return generate_look_up_array(
            it, end, azimuth_index, std::make_index_sequence<el_num>{});
}

template <size_t... Ix>
constexpr auto generate_look_up_table(const hrtf_data::entry* it,
                                      const hrtf_data::entry* end,
                                      std::index_sequence<Ix...>) {
    using value_type = decltype(generate_look_up_array(it, end, 0));
    return std::array<value_type, sizeof...(Ix)>{
            {generate_look_up_array(it, end, Ix)...}};
}

constexpr auto generate_look_up_table(const hrtf_data::entry* it,
                                      const hrtf_data::entry* end) {
    return generate_look_up_table(it, end, std::make_index_sequence<az_num>{});
}

constexpr auto look_up_table = generate_look_up_table(
        std::begin(hrtf_data::entries), std::end(hrtf_data::entries));

constexpr auto look_up_angles(int azimuth, int elevation) {
    return look_up_table[index_from_azimuth(azimuth)]
                        [index_from_elevation(elevation)];
}

}  // namespace hrtf_look_up_table
