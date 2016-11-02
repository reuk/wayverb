#pragma once

#include <algorithm>
#include <array>
#include <cstdlib>

#include "core/orientable.h"

#include "glm/glm.hpp"

namespace core {

template <typename U>
static constexpr const auto& clamp(const U& x, const U& mini, const U& maxi) {
    using std::min;
    using std::max;
    return max(mini, min(maxi, x));
}

constexpr float degrees(float radians) { return radians * 180 / M_PI; }
constexpr float radians(float degrees) { return degrees * M_PI / 180; }

template <typename T, size_t azimuth_divisions, size_t elevation_divisions>
struct vector_look_up_table final {
    static_assert(elevation_divisions % 2, "elevation_divisions must be odd");

    struct index_pair final {
        size_t azimuth;
        size_t elevation;
    };

    //  look up

    constexpr auto& at(index_pair i) { return table[i.azimuth][i.elevation]; }
    constexpr const auto& at(index_pair i) const {
        return table[i.azimuth][i.elevation];
    }

    //  data

    static constexpr auto get_azimuth_angle() {
        return 360.0 / azimuth_divisions;
    }

    static constexpr auto get_elevation_angle() {
        return 180.0 / (elevation_divisions + 1);
    }

    //  angles to indices

    static constexpr size_t azimuth_to_index(double azimuth) {
        azimuth += get_azimuth_angle() / 2;
        while (azimuth < 0) {
            azimuth += 360;
        }
        return static_cast<size_t>(azimuth / get_azimuth_angle()) %
               azimuth_divisions;
    }

    static constexpr size_t elevation_to_index(double elevation) {
        elevation += 90 + (get_elevation_angle() / 2);
        while (elevation < 0) {
            elevation += 360;
        }

        const auto adjusted =
                static_cast<size_t>(elevation / get_elevation_angle()) %
                (2 * (elevation_divisions + 1));

        if (elevation_divisions + 1 < adjusted) {
            throw std::runtime_error{"elevation out of range"};
        }

        return clamp(adjusted, 1ul, elevation_divisions) - 1;
    }

    static constexpr index_pair angles_to_indices(az_el azel) {
        return {azimuth_to_index(azel.azimuth),
                elevation_to_index(azel.elevation)};
    }

    //  indices to angles

    static constexpr auto index_to_azimuth(size_t index) {
        if (azimuth_divisions <= index) {
            throw std::runtime_error{"index out of range"};
        }
        return (index * get_azimuth_angle());
    }

    static constexpr auto index_to_elevation(size_t index) {
        if (elevation_divisions <= index) {
            throw std::runtime_error{"index out of range"};
        }
        return ((index + 1) * get_elevation_angle()) - 90;
    }

    static constexpr az_el indices_to_angles(index_pair i) {
        return {index_to_azimuth(i.azimuth), index_to_elevation(i.elevation)};
    }

    //  vectors and angles

    static auto pointing(index_pair i) {
        return compute_pointing(
                az_el{-radians(index_to_azimuth(i.azimuth)),
                      radians(index_to_elevation(i.elevation))});
    }

    static auto index(const glm::vec3& i) {
        const auto radians = compute_azimuth_elevation(i);
        return index_pair{azimuth_to_index(degrees(-radians.azimuth)),
                          elevation_to_index(degrees(radians.elevation))};
    }

    /// Using a C array because it's more constexpr-ready than std::array.
    T table[azimuth_divisions][elevation_divisions]{};
};

}  // namespace core
