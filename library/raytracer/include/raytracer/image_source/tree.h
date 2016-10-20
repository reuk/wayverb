#pragma once

#include "raytracer/image_source/fast_pressure_calculator.h"
#include "raytracer/multitree.h"

#include "common/cl/include.h"
#include "common/geo/triangle_vec.h"
#include "common/spatial_division/voxelised_scene_data.h"

#include "utilities/aligned/vector.h"

namespace raytracer {
namespace image_source {

/// Each item in the tree references an intersected triangle, which may or
/// may not be visible from the receiver.
struct path_element final {
    cl_uint index;
    bool visible;
};

constexpr auto to_tuple(const path_element& x) {
    return std::tie(x.index, x.visible);
}

constexpr bool operator==(const path_element& a, const path_element& b) {
    return to_tuple(a) == to_tuple(b);
}

constexpr bool operator!=(const path_element& a, const path_element& b) {
    return !(a == b);
}

/// Need this because we'll be storing path_elements in a set.
constexpr bool operator<(const path_element& a, const path_element& b) {
    return a.index < b.index;
}

//----------------------------------------------------------------------------//

class tree final {
public:
    void push(const aligned::vector<path_element>& path);
    const multitree<path_element>::branches_type& get_branches() const;

private:
    multitree<path_element> root_{path_element{}};
};

using postprocessor = std::function<void(
        const glm::vec3&,
        aligned::vector<reflection_metadata>::const_iterator,
        aligned::vector<reflection_metadata>::const_iterator)>;

void find_valid_paths(
        const multitree<path_element>& tree,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const voxelised_scene_data<cl_float3, surface<simulation_bands>>&
                voxelised,
        const postprocessor& callback);

}  // namespace image_source
}  // namespace raytracer
