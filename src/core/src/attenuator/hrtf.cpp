#include "core/attenuator/hrtf.h"
#include "core/vector_look_up_table.h"
#include "hrtf_entries.h"

#include <algorithm>
#include <numeric>

namespace wayverb {
namespace core {
namespace hrtf_look_up_table {

struct azimuth_functor final {
    template <typename T>
    constexpr auto operator()(const T& t) const {
        return t.azimuth;
    }
};

struct elevation_functor final {
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
        throw std::runtime_error{"Items must be equally spaced."};
    }
    return ret;
}

constexpr auto b = std::begin(hrtf_data::entries);
constexpr auto e = std::end(hrtf_data::entries);

constexpr auto az_num = 360 / find_inc(b, e, azimuth_functor{});
constexpr auto el_num = (180 / find_inc(b, e, elevation_functor{})) - 1;

constexpr auto generate_hrtf_table() {
    using hrtf_table =
            vector_look_up_table<std::array<std::array<double, 8>, 2>,
                                 az_num,
                                 el_num>;

    hrtf_table ret{};

    for (auto it = b; it != e; ++it) {
        const auto azel = az_el{static_cast<float>(azimuth_functor{}(*it)),
                                static_cast<float>(elevation_functor{}(*it))};
        ret.at(hrtf_table::angles_to_indices(azel)) = it->energy;
    }

    return ret;
}

constexpr auto table = generate_hrtf_table();

}  // namespace hrtf_look_up_table

namespace attenuator {

hrtf::hrtf(const orientation_t& o, channel channel, float radius)
        : orientation{o}
        , channel_{channel}
        , radius_{radius} {}

hrtf::channel hrtf::get_channel() const { return channel_; }
float hrtf::get_radius() const { return radius_; }

void hrtf::set_channel(channel channel) { channel_ = channel; }

void hrtf::set_radius(float radius) {
    if (radius < 0 || 1 < radius) {
        throw std::runtime_error{"Hrtf radius outside reasonable range."};
    }
    radius_ = radius;
}

////////////////////////////////////////////////////////////////////////////////

bands_type attenuation(const hrtf& hrtf, const glm::vec3& incident) {
    if (const auto l = glm::length(incident)) {
        const auto transformed = transform(hrtf.orientation, incident / l);

        using table = decltype(hrtf_look_up_table::table);
        const auto channels =
                hrtf_look_up_table::table.at(table::index(transformed));
        const auto channel =
                channels[hrtf.get_channel() == hrtf::channel::left ? 0 : 1];
        return to_cl_float_vector(channel);
    }
    return bands_type{};
}

glm::vec3 get_ear_position(const hrtf& hrtf, const glm::vec3& base_position) {
    const auto x = hrtf.get_channel() == hrtf::channel::left
                           ? -hrtf.get_radius()
                           : hrtf.get_radius();
    return base_position + transform(hrtf.orientation, glm::vec3{x, 0, 0});
}

}  // namespace attenuator
}  // namespace core
}  // namespace wayverb
