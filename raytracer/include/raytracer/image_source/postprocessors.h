#pragma once

#include "raytracer/cl/structs.h"

#include "common/aligned/vector.h"
#include "common/conversions.h"
#include "common/map_to_vector.h"
#include "common/scene_data.h"
#include "common/specular_absorption.h"
#include "common/surfaces.h"
#include "common/unit_constructor.h"

#include "glm/fwd.hpp"

#include <complex>

/// The image source finder is designed just to ascertain whether or not each
/// 'possible' image-source path is an *actual* image-source path.
/// Once it's worked this out, it delegates to one of these secondary functions
/// to actually compute the pressure or intensity contributed by that particular
/// path.

namespace raytracer {
namespace image_source {

template <typename Vol>
struct generic_impulse final {
    Vol volume;
    glm::vec3 position;
    double distance;
};

struct reflection_metadata final {
    cl_uint surface_index;  /// The index of the triangle that was intersected.
    float cos_angle;  /// The cosine of the angle against the triangle normal.
};

//----------------------------------------------------------------------------//

template <typename Surface, typename It>
generic_impulse<specular_absorption_t<Surface>> compute_intensity(
        const glm::vec3& receiver,
        const aligned::vector<Surface>& surfaces,
        bool flip_phase,
        const glm::vec3& image_source,
        It begin,
        It end) {
    const auto surface_attenuation{std::accumulate(
            begin,
            end,
            unit_constructor_v<specular_absorption_t<Surface>>,
            [&](auto i, auto j) {
                const auto reflectance{absorption_to_energy_reflectance(
                        get_specular_absorption(surfaces[j.surface_index]))};
                return i * reflectance * (flip_phase ? -1 : 1);
            })};

    return {surface_attenuation,
            image_source,
            glm::distance(image_source, receiver)};
}

template <typename Surface = surface>
class intensity_calculator final {
public:
    intensity_calculator(const glm::vec3& receiver,
                         aligned::vector<Surface>
                                 surfaces,
                         bool flip_phase)
            : receiver_{receiver}
            , surfaces_{std::move(surfaces)}
            , flip_phase_{flip_phase} {}

    using return_type = generic_impulse<specular_absorption_t<Surface>>;

    template <typename It>
    return_type operator()(const glm::vec3& image_source,
                           It begin,
                           It end) const {
        return compute_intensity(
                receiver_, surfaces_, flip_phase_, image_source, begin, end);
    }

private:
    glm::vec3 receiver_;
    aligned::vector<Surface> surfaces_;
    bool flip_phase_;
};

//----------------------------------------------------------------------------//

template <typename Imp, typename It>
generic_impulse<Imp> compute_fast_pressure(
        const glm::vec3& receiver,
        const aligned::vector<Imp>& surface_impedances,
        bool flip_phase,
        const glm::vec3& image_source,
        It begin,
        It end) {
    const auto surface_attenuation{std::accumulate(
            begin, end, unit_constructor_v<Imp>, [&](auto i, auto j) {
                const auto reflectance{
                        average_wall_impedance_to_pressure_reflectance(
                                surface_impedances[j.surface_index],
                                j.cos_angle)};
                return i * reflectance * (flip_phase ? -1 : 1);
            })};
    return {surface_attenuation,
            image_source,
            glm::distance(image_source, receiver)};
}

/// A simple pressure calculator which accumulates pressure responses in
/// the time domain.
/// Uses equation 9.22 from the kuttruff book, assuming single-sample
/// reflection/convolution kernels.
template <typename Surface = surface>
class fast_pressure_calculator final {
public:
    fast_pressure_calculator(const glm::vec3& receiver,
                             const aligned::vector<Surface>& surfaces,
                             bool flip_phase)
            : receiver_{receiver}
            , surface_impedances_{map_to_vector(
                      surfaces,
                      [](auto material) {
                          return pressure_reflectance_to_average_wall_impedance(
                                  absorption_to_pressure_reflectance(
                                          get_specular_absorption(material)));
                      })}
            , flip_phase_{flip_phase} {}

    using return_type = generic_impulse<specular_absorption_t<Surface>>;

    template <typename It>
    return_type operator()(const glm::vec3& image_source,
                           It begin,
                           It end) const {
        return compute_fast_pressure(receiver_,
                                     surface_impedances_,
                                     flip_phase_,
                                     image_source,
                                     begin,
                                     end);
    }

private:
    glm::vec3 receiver_;
    aligned::vector<decltype(get_specular_absorption(std::declval<Surface>()))>
            surface_impedances_;
    bool flip_phase_;
};

//----------------------------------------------------------------------------//

template <typename Impl>
class depth_counter_calculator final {
public:
    template <typename... Ts>
    depth_counter_calculator(Ts&&... ts)
            : impl_{std::forward<Ts>(ts)...} {}

    struct return_type final {
        decltype(std::declval<Impl>().operator()(
                std::declval<glm::vec3>(),
                std::declval<
                        aligned::vector<reflection_metadata>::const_iterator>(),
                std::declval<aligned::vector<
                        reflection_metadata>::const_iterator>())) value;
        std::ptrdiff_t depth;
    };

    template <typename It>
    auto operator()(const glm::vec3& image_source, It begin, It end) const {
        return return_type{impl_(image_source, begin, end),
                           std::distance(begin, end)};
    }

private:
    Impl impl_;
};

}  // namespace image_source
}  // namespace raytracer
