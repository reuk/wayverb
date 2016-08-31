#include "raytracer/image_source/tree.h"

#include "common/map_to_vector.h"
#include "common/mapping_iterator_adapter.h"

namespace raytracer {
namespace image_source {

multitree<path_element>::branches_type construct_image_source_tree(
        const aligned::vector<aligned::vector<path_element>>& paths) {
    multitree<path_element> root{path_element{}};
    for (const auto& i : paths) {
        add_path(root, i.begin(), i.end());
    }
    return std::move(root.branches);
}

//----------------------------------------------------------------------------//

geo::ray construct_ray(const glm::vec3& from, const glm::vec3& to) {
    if (from == to) {
        throw std::runtime_error(
                "tried to construct a ray pointing towards its starting "
                "location");
    }
    return geo::ray(from, glm::normalize(to - from));
}

aligned::vector<glm::vec3> compute_intersection_points(
        const aligned::vector<float>& distances, const geo::ray& ray) {
    return map_to_vector(distances, [&](const auto i) {
        return ray.get_position() + ray.get_direction() * i;
    });
}

float compute_distance(const glm::vec3& source,
                       const aligned::vector<glm::vec3>& unmirrored,
                       const glm::vec3& receiver) {
    return glm::distance(source, unmirrored.front()) +
           std::inner_product(
                   unmirrored.begin(),
                   unmirrored.end() - 1,
                   unmirrored.begin() + 1,
                   0.0f,
                   [](const auto& a, const auto& b) { return a + b; },
                   [](const auto& a, const auto& b) {
                       return glm::distance(a, b);
                   }) +
           glm::distance(unmirrored.back(), receiver);
}

//----------------------------------------------------------------------------//

template <typename It>
auto extract_index_iterator(It it) {
    struct extract_index final {
        constexpr auto& operator()(state& s) const { return s.index; }
    };
    return make_mapping_iterator_adapter(std::move(it), extract_index{});
}

template <typename It>
image_source_traversal_callback<It>::image_source_traversal_callback(
        const constants& constants,
        aligned::vector<state>& s,
        It output_iterator,
        const path_element& p)
        : constants_(constants)
        , state_(s)
        , output_iterator_(std::move(output_iterator)) {
    const auto& scene{constants_.voxelised.get_scene_data()};

    //  find the image source position
    //  i.e. the previous image source, reflected in the current triangle
    const auto image_source{geo::mirror(
            state_.empty() ? constants_.source : state_.back().image_source,
            geo::get_triangle_vec3(scene.get_triangles()[p.index],
                                   scene.get_vertices()))};

    //  append to state vector
    state_.push_back(state{p.index, image_source});

    //  finally, if there is an impulse here, add it to the output
    if (p.visible) {
        if (const auto impulse{compute_impulse()}) {
            *output_iterator_++ = *impulse;
        }
    }
}

template <typename It>
std::experimental::optional<impulse>
image_source_traversal_callback<It>::compute_impulse() const {
    //  now we have to compute the impulse to return
    //
    //  in weird scenarios the image source might end up getting plastered over
    //  the receiver, which is bad, so we quit with null in that case
    const auto final_image_source{state_.back().image_source};
    if (constants_.receiver == final_image_source) {
        return std::experimental::nullopt;
    }

    //  check that we can cast a ray to the receiver from all of the image
    //  sources, through the correct triangles
    auto prev_intersection{constants_.receiver};  //  trace from here
    auto prev_surface{~cl_uint{0}};               //  ignore this triangle
    for (auto i{state_.crbegin()}, end{state_.crend()}; i != end; ++i) {
        //  find the ray from the receiver to the image source
        const auto ray{construct_ray(prev_intersection, i->image_source)};

        //  now check for intersections with the scene
        const auto intersection{
                intersects(constants_.voxelised, ray, prev_surface)};

        //  if we didn't find an intersection
        //  or if the intersected triangle isn't the correct one
        if (! intersection || intersection->index != i->index) {
            //  there's no valid path, return null
            return std::experimental::nullopt;
        }

        //  this path segment is valid, update accumulators
        prev_intersection = ray.get_position() +
                            ray.get_direction() * intersection->inter.t;
        prev_surface = i->index;
    }

    //  if we got here, the path is a valid image-source path
    //  we compute the impulse and push it onto the output collection
    const auto& scene_data{constants_.voxelised.get_scene_data()};
    const auto volume{std::accumulate(
            extract_index_iterator(state_.begin()),
            extract_index_iterator(state_.end()),
            make_volume_type(1),
            [&](const auto& volume, const auto& i) {
                const auto scene_triangle{scene_data.get_triangles()[i]};
                const auto surface{
                        scene_data.get_surfaces()[scene_triangle.surface]};
                return volume * surface.specular;
            })};

    //  TODO this is wrong
    //  return pressure rather than intensity
    return construct_impulse(
            volume,
            final_image_source,
            glm::distance(final_image_source, constants_.receiver),
            constants_.speed_of_sound);
}

template <typename It>
image_source_traversal_callback<
        It>::~image_source_traversal_callback() noexcept {
    state_.pop_back();
}

template <typename It>
image_source_traversal_callback<It> image_source_traversal_callback<It>::
operator()(const path_element& p) const {
    return image_source_traversal_callback{
            constants_, state_, output_iterator_, p};
}

//----------------------------------------------------------------------------//

template <typename It>
class initial_callback final {
public:
    initial_callback(const constants& constants,
                     aligned::vector<state>& state,
                     It output_iterator)
            : constants_(constants)
            , state_(state)
            , output_iterator_(std::move(output_iterator)) {}

    image_source_traversal_callback<It> operator()(
            const path_element& p) const {
        return image_source_traversal_callback<It>{
                constants_, state_, output_iterator_, p};
    }

private:
    const constants& constants_;
    aligned::vector<state>& state_;
    It output_iterator_;
};

template <typename It>
auto make_initial_callback(const constants& constants,
                           aligned::vector<state>& state,
                           It& output_iterator) {
    return initial_callback<It>{constants, state, output_iterator};
}

aligned::vector<impulse> compute_impulses(
        const aligned::vector<aligned::vector<path_element>>& paths,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const voxelised_scene_data& voxelised,
        double speed_of_sound) {
    //  build path tree
    const auto tree{construct_image_source_tree(paths)};

    //  iterate on tree
    aligned::vector<impulse> ret{};
    auto output_iterator{std::back_inserter(ret)};

    //  for each starting node
    for (const auto& branch : tree) {
        //  set up a state array
        aligned::vector<state> state{};

        //  set up the callback
        const auto callback{make_initial_callback(
                constants{source, receiver, voxelised, speed_of_sound},
                state,
                output_iterator)};

        //  traverse all paths on this branch
        traverse_multitree(branch, callback);
    }

    return ret;
}

}  // namespace image_source
}  // namespace raytracer
