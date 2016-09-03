#include "raytracer/image_source/tree.h"

#include "common/map_to_vector.h"
#include "common/mapping_iterator_adapter.h"
#include "common/output_iterator_callback.h"

namespace raytracer {
namespace image_source {

geo::ray construct_ray(const glm::vec3& from, const glm::vec3& to) {
    if (from == to) {
        throw std::runtime_error(
                "tried to construct a ray pointing towards its starting "
                "location");
    }
    return geo::ray(from, glm::normalize(to - from));
}

//----------------------------------------------------------------------------//

class image_source_traversal_callback final {
public:
    struct state final {
        cl_uint index;
        glm::vec3 image_source;
    };

    image_source_traversal_callback(const glm::vec3& receiver,
                                    const voxelised_scene_data& voxelised,
                                    image_source_tree::callback& callback,
                                    aligned::vector<state>& state,
                                    const path_element& element)
            : receiver_(receiver)
            , voxelised_(voxelised)
            , callback_(callback)
            , state_(state) {
        //  Find the image source location and intersected triangle.
        state_.push_back(path_element_to_state(voxelised_, state_, element));

        //  Find whether this is a valid path, and if it is, call the callback.
        if (const auto valid{find_valid_path(receiver_, voxelised_, state_)}) {
            callback_(valid->image_source, valid->intersections);
        }
    }

    ~image_source_traversal_callback() noexcept {
        state_.pop_back();
    }

    image_source_traversal_callback operator()(const path_element& p) const {
        return image_source_traversal_callback{
                receiver_, voxelised_, callback_, state_, p};
    }

private:
    static auto get_triangle(const voxelised_scene_data& voxelised,
                             const cl_uint triangle_index) {
        const auto& scene{voxelised.get_scene_data()};
        return geo::get_triangle_vec3(scene.get_triangles()[triangle_index],
                                      scene.get_vertices());
    }

    static glm::vec3 find_image_source(const glm::vec3& previous_source,
                                       const voxelised_scene_data& voxelised,
                                       const cl_uint triangle_index) {
        return geo::mirror(previous_source,
                           get_triangle(voxelised, triangle_index));
    }

    static state path_element_to_state(const voxelised_scene_data& voxelised,
                                       const aligned::vector<state>& state,
                                       const path_element& p) {
        if (state.empty())  {
            throw std::runtime_error("state vector is empty");
        }
        return {p.index,
                find_image_source(
                        state.back().image_source, voxelised, p.index)};
    }

    struct valid_path final {
        glm::vec3 image_source;
        aligned::vector<image_source_tree::intersection> intersections;
    };

    static std::experimental::optional<valid_path> find_valid_path(
            const glm::vec3& receiver,
            const voxelised_scene_data& voxelised,
            const aligned::vector<state>& state) {
        //  in weird scenarios the image source might end up getting plastered
        //  over the receiver, which is bad, so we quit with null in that case
        const auto final_image_source{state.back().image_source};
        if (receiver == final_image_source) {
            return std::experimental::nullopt;
        }

        //  check that we can cast a ray to the receiver from all of the image
        //  sources, through the correct triangles
        glm::vec3 prev_intersection{receiver};  //  trace from here
        aligned::vector<image_source_tree::intersection> intersections;

        for (auto i{state.crbegin()}, end{state.crend()}; i != end; ++i) {
            //  find the ray from the receiver to the image source
            const auto ray{construct_ray(i->image_source, prev_intersection)};

            //  now check for intersections with the scene
            const auto intersection{intersects(voxelised, ray)};

            //  If we didn't find an intersection or if the intersected triangle
            //  isn't the correct one.
            if (!intersection || intersection->index != i->index) {
                //  There's no valid path, return null.
                return std::experimental::nullopt;
            }

            //  This path segment is valid.
            //  Find angle between ray and triangle normal at intersection.
            //  Add appropriate intersection to ret.
            intersections.push_back(image_source_tree::intersection{
                    i->index,
                    std::acos(glm::dot(
                            ray.get_direction(),
                            geo::normal(get_triangle(voxelised, i->index))))});

            //  Update accumulator.
            prev_intersection = ray.get_position() +
                                ray.get_direction() * intersection->inter.t;
        }

        if (intersections.size() < 2) {
            throw std::runtime_error(
                    "intersection number should always be two or more (from "
                    "source to intersection, and from receiver to "
                    "intersection)");
        }

        //  Final intersections is from the source and we don't care.
        intersections.pop_back();

        //  If we got here, the path is a valid image-source path.
        //  We compute the impulse and push it onto the output collection.
        return valid_path{final_image_source, intersections};
    }

    std::experimental::optional<impulse> compute_impulse() const;

    const glm::vec3& receiver_;
    const voxelised_scene_data& voxelised_;

    image_source_tree::callback& callback_;
    aligned::vector<state>& state_;
};

//----------------------------------------------------------------------------//

multitree<path_element>::branches_type construct_image_source_tree(
        const aligned::vector<aligned::vector<path_element>>& paths) {
    multitree<path_element> root{path_element{}};
    for (const auto& i : paths) {
        add_path(root, i.begin(), i.end());
    }
    return std::move(root.branches);
}

image_source_tree::image_source_tree(
        const aligned::vector<aligned::vector<path_element>>& paths)
        : branches_(construct_image_source_tree(paths)) {}

void image_source_tree::find_valid_paths(const glm::vec3& source,
                                         const glm::vec3& receiver,
                                         const voxelised_scene_data& voxelised,
                                         callback callback) const {
    //  iterate on tree
    //  for each starting node
    for (const auto& branch : branches_) {
        //  set up a state array
        aligned::vector<image_source_traversal_callback::state> state{
                image_source_traversal_callback::state{branch.item.index,
                                                       source}};

        //  traverse all paths on this branch
        traverse_multitree(
                branch,
                image_source_traversal_callback{
                        receiver, voxelised, callback, state, branch.item});
    }
}

const multitree<path_element>::branches_type& image_source_tree::get_branches()
        const {
    return branches_;
}

}  // namespace image_source
}  // namespace raytracer
