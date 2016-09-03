#pragma once

/// The image source finder is designed just to ascertain whether or not each
/// 'possible' image-source path is an *actual* image-source path.
/// Once it's worked this out, it delegates to one of these secondary functions
/// to actually compute the pressure or intensity contributed by that particular
/// path.

#include "raytracer/image_source/tree.h"

namespace raytracer {
namespace image_source {

class intensity_calculator final {
public:
    intensity_calculator(const glm::vec3& receiver,
                         const voxelised_scene_data& voxelised,
                         float speed_of_sound);

    impulse operator()(const glm::vec3& image_source,
                       const aligned::vector<image_source_tree::intersection>&
                               intersections) const;

private:
    const glm::vec3& receiver_;
    const voxelised_scene_data& voxelised_;
    float speed_of_sound_;
};

}  // namespace image_source
}  // namespace raytracer
