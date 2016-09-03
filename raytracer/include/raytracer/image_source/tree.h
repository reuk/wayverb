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

/// Need this because we'll be storing path_elements in a set.
constexpr bool operator<(const path_element& a, const path_element& b) {
    return a.index < b.index;
}

//----------------------------------------------------------------------------//

multitree<path_element>::branches_type construct_image_source_tree(
        const aligned::vector<aligned::vector<path_element>>& paths);

//----------------------------------------------------------------------------//

/// The trick here is that the callback can be a stateful object...
template <typename T, typename Callback>
void traverse_multitree(const multitree<T>& tree, const Callback& callback) {
    const auto next{callback(tree.item)};
    for (const auto& i : tree.branches) {
        traverse_multitree(i, next);
    }
}

//----------------------------------------------------------------------------//

geo::ray construct_ray(const glm::vec3& from, const glm::vec3& to);

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

//----------------------------------------------------------------------------//

struct constants final {
    const glm::vec3& source;
    const glm::vec3& receiver;
    const voxelised_scene_data& voxelised;
    const double speed_of_sound;
};

struct state final {
    cl_uint index;
    glm::vec3 image_source;
};

class image_source_traversal_callback final {
public:
    using output_callback = std::function<void(impulse)>;

    image_source_traversal_callback(const constants& constants,
                                    aligned::vector<state>& state,
                                    output_callback callback,
                                    const path_element& p);

    ~image_source_traversal_callback() noexcept;

    image_source_traversal_callback operator()(const path_element& p) const;

private:
    std::experimental::optional<impulse> compute_impulse() const;

    const constants& constants_;
    aligned::vector<state>& state_;
    output_callback callback_;
};

//----------------------------------------------------------------------------//

aligned::vector<impulse> compute_impulses(
        const aligned::vector<aligned::vector<path_element>>& paths,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const voxelised_scene_data& voxelised,
        double speed_of_sound);

}//namespace image_source
}  // namespace raytracer
