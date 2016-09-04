#pragma once

#include "raytracer/cl/structs.h"

#include "common/aligned/vector.h"
#include "common/spatial_division/voxelised_scene_data.h"

#include "glm/fwd.hpp"

/// The image source finder is designed just to ascertain whether or not each
/// 'possible' image-source path is an *actual* image-source path.
/// Once it's worked this out, it delegates to one of these secondary functions
/// to actually compute the pressure or intensity contributed by that particular
/// path.

namespace raytracer {
namespace image_source {

struct reflection_metadata final {
    cl_uint index;  /// The index of the triangle that was intersected.
    float angle;    /// The angle against the triangle normal.
};

/// All image-source postprocessors should be convertible to this type.
using postprocessor = std::function<void(
        const glm::vec3&, const aligned::vector<reflection_metadata>&)>;

//----------------------------------------------------------------------------//

class intensity_calculator final {
public:
    intensity_calculator(const glm::vec3& receiver,
                         const voxelised_scene_data& voxelised,
                         float speed_of_sound);

    impulse operator()(
            const glm::vec3& image_source,
            const aligned::vector<reflection_metadata>& intersections) const;

private:
    const glm::vec3& receiver_;
    const voxelised_scene_data& voxelised_;
    float speed_of_sound_;
};

}  // namespace image_source
}  // namespace raytracer
