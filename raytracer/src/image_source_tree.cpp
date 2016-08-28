#include "raytracer/image_source_tree.h"

#include "common/map_to_vector.h"
#include "common/mapping_iterator_adapter.h"

namespace raytracer {

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
auto extract_mirrored_iterator(It it) {
    struct extract_mirrored_triangle final {
        constexpr auto& operator()(state& s) const {
            return s.mirrored_triangle;
        }
    };
    return make_mapping_iterator_adapter(std::move(it),
                                         extract_mirrored_triangle{});
}

template <typename It>
auto extract_original_iterator(It it) {
    struct extract_original_triangle final {
        constexpr auto& operator()(state& s) const {
            return s.original_triangle;
        }
    };
    return make_mapping_iterator_adapter(std::move(it),
                                         extract_original_triangle{});
}

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

    //  find the scene-space position of this triangle
    const auto original_triangle{geo::get_triangle_vec3(
            scene.get_triangles()[p.index], scene.get_vertices())};

    //  find its image-source-space position
    const auto mirrored_triangle{
            compute_mirrored_triangle(extract_mirrored_iterator(state_.begin()),
                                      extract_mirrored_iterator(state_.end()),
                                      original_triangle)};

    //  find the receiver image position
    const auto mirrored_receiver{
            geo::mirror(state_.empty() ? constants_.receiver
                                       : state_.back().mirrored_receiver,
                        mirrored_triangle)};

    //  append to state vector
    state_.push_back(state{
            p.index, original_triangle, mirrored_triangle, mirrored_receiver});

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

    if (constants_.source == state_.back().mirrored_receiver) {
        return std::experimental::nullopt;
    }

    //  check that we can cast a ray through all the mirrored triangles to the
    //  receiver
    const auto ray{
            construct_ray(constants_.source, state_.back().mirrored_receiver)};
    const auto distances{compute_intersection_distances(
            extract_mirrored_iterator(state_.begin()),
            extract_mirrored_iterator(state_.end()),
            ray)};

    if (!distances) {
        return std::experimental::nullopt;
    }

    //  if we can, find the intersection points with the mirrored surfaces
    const auto points{compute_intersection_points(*distances, ray)};

    //  now mirror the intersection points back into scene space
    const auto unmirrored{
            compute_unmirrored_points(extract_original_iterator(state_.begin()),
                                      extract_original_iterator(state_.end()),
                                      points)};

    const auto does_intersect{[&](
            const auto& a, const auto& b, const auto& tri_to_ignore) {
        if (a == b) {
            return true;
        }
        const auto i{intersects(
                constants_.voxelised, geo::ray(a, b - a), tri_to_ignore)};
        const auto dist{glm::distance(a, b)};
        return i && almost_equal(i->inter.t, dist, 10);
    }};

    if (!does_intersect(constants_.source, unmirrored.front(), ~size_t{0})) {
        return std::experimental::nullopt;
    }

    //  attempt to join the dots back in scene space
    {
        auto a{unmirrored.begin()};
        auto b{unmirrored.begin() + 1};
        auto c{extract_index_iterator(state_.begin())};
        for (; b != unmirrored.end(); ++a, ++b, ++c) {
            if (!does_intersect(*a, *b, *c)) {
                return std::experimental::nullopt;
            }
        }
    }

    if (!does_intersect(constants_.receiver, unmirrored.back(), ~size_t{0})) {
        return std::experimental::nullopt;
    }

    return compute_ray_path_impulse(extract_index_iterator(state_.begin()),
                                    extract_index_iterator(state_.end()),
                                    constants_.voxelised.get_scene_data(),
                                    constants_.source,
                                    unmirrored,
                                    constants_.receiver,
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

}  // namespace raytracer
