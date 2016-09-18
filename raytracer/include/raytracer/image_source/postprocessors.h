#pragma once

#include "raytracer/cl/structs.h"

#include "common/aligned/vector.h"
#include "common/spatial_division/voxelised_scene_data.h"

#include "glm/fwd.hpp"

#include <complex>

/// The image source finder is designed just to ascertain whether or not each
/// 'possible' image-source path is an *actual* image-source path.
/// Once it's worked this out, it delegates to one of these secondary functions
/// to actually compute the pressure or intensity contributed by that particular
/// path.

namespace raytracer {
namespace image_source {

struct reflection_metadata final {
    cl_uint index;    /// The index of the triangle that was intersected.
    float cos_angle;  /// The cosine of the angle against the triangle normal.
};

/// All image-source postprocessors should be convertible to this type.
//----------------------------------------------------------------------------//

class intensity_calculator final {
public:
    intensity_calculator(const glm::vec3& source,
                         const glm::vec3& receiver,
                         const voxelised_scene_data& voxelised,
                         float acoustic_impedance,
                         bool flip_phase);

    impulse operator()(
            const glm::vec3& image_source,
            const aligned::vector<reflection_metadata>& intersections) const;

private:
    const glm::vec3& receiver_;
    const voxelised_scene_data& voxelised_;
    bool flip_phase_;
};

//----------------------------------------------------------------------------//

class fast_pressure_calculator final {
public:
    fast_pressure_calculator(const glm::vec3& source,
                             const glm::vec3& receiver,
                             const voxelised_scene_data& voxelised,
                             float acoustic_impedance,
                             bool flip_phase);

    impulse operator()(
            const glm::vec3& image_source,
            const aligned::vector<reflection_metadata>& intersections) const;

private:
    const glm::vec3& receiver_;
    const voxelised_scene_data& voxelised_;
    float acoustic_impedance_;
    bool flip_phase_;

    aligned::vector<volume_type> surface_impedances_;
};

//----------------------------------------------------------------------------//

class comparison_calculator final {
public:
    comparison_calculator(const glm::vec3& source,
                          const glm::vec3& receiver,
                          const voxelised_scene_data& voxelised,
                          float acoustic_impedance,
                          bool flip_phase);

    impulse operator()(
            const glm::vec3& image_source,
            const aligned::vector<reflection_metadata>& intersections) const;

private:
    intensity_calculator intensity_;
    fast_pressure_calculator fast_pressure_;
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
                std::declval<aligned::vector<reflection_metadata>>())) value;
        size_t depth;
    };

    return_type operator()(
            const glm::vec3& image_source,
            const aligned::vector<reflection_metadata>& intersections) const {
        return {impl_(image_source, intersections), intersections.size()};
    }

private:
    Impl impl_;
};

}  // namespace image_source
}  // namespace raytracer
