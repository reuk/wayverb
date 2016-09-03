#pragma once

#include "raytracer/construct_impulse.h"
#include "raytracer/image_source/finder.h"
#include "raytracer/multitree.h"

#include "common/aligned/vector.h"
#include "common/cl/include.h"
#include "common/geo/triangle_vec.h"
#include "common/spatial_division/voxelised_scene_data.h"

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

class image_source_tree final {
public:
    image_source_tree(
            const aligned::vector<aligned::vector<path_element>>& paths);

    struct intersection final {
        cl_uint surface;
        float angle;
    };

    /// Callback will be called with the location of the image source,
    /// and the series of surface/angle pairs along the image path.
    using callback = std::function<void(const glm::vec3&,
                                        const aligned::vector<intersection>&)>;

    void find_valid_paths(const glm::vec3& source,
                          const glm::vec3& receiver,
                          const voxelised_scene_data& voxelised,
                          callback callback) const;

    const multitree<path_element>::branches_type& get_branches() const;

private:
    multitree<path_element>::branches_type branches_;

};

//----------------------------------------------------------------------------//

template <typename It>
volume_type compute_volume(It begin, It end, const scene_data& scene_data) {
    return std::accumulate(
            begin,
            end,
            make_volume_type(1),
            [&](const auto& volume, const auto& i) {
                const auto scene_triangle{scene_data.get_triangles()[i]};
                const auto surface{
                        scene_data.get_surfaces()[scene_triangle.surface]};
                return volume * surface.specular;
            });
}

}  // namespace image_source
}  // namespace raytracer
