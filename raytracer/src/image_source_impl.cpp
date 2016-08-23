#include "common/almost_equal.h"
#include "common/spatial_division/voxelised_scene_data.h"
#include "common/stl_wrappers.h"
#include "raytracer/construct_impulse.h"
#include "raytracer/image_source_impl.h"

#include <numeric>

namespace raytracer {

aligned::vector<geo::triangle_vec3> compute_original_triangles(
        const aligned::vector<cl_ulong>& triangles,
        const copyable_scene_data& scene_data) {
    return map_to_vector(triangles, [&](const auto i) {
        return geo::get_triangle_vec3(scene_data.get_triangles()[i],
                                      scene_data.get_vertices());
    });
}

aligned::vector<geo::triangle_vec3> compute_mirrored_triangles(
        const aligned::vector<geo::triangle_vec3>& original) {
    //  prepare output array
    aligned::vector<geo::triangle_vec3> ret;
    ret.reserve(original.size());

    //  for each triangle in the path
    for (auto reflected : original) {
        //  mirror it in the previous mirrored triangles that we've found
        for (const auto& previous_triangle : ret) {
            reflected = geo::mirror(reflected, previous_triangle);
        }
        //  then add it to the back of the output vector
        ret.push_back(reflected);
    }

    return ret;
}

std::experimental::optional<aligned::vector<float>>
compute_intersection_distances(
        const aligned::vector<geo::triangle_vec3>& mirrored,
        const geo::ray& ray) {
    aligned::vector<float> ret;
    ret.reserve(mirrored.size());

    for (const auto& i : mirrored) {
        const auto intersection = geo::triangle_intersection(i, ray);
        if (!intersection) {
            return std::experimental::nullopt;
        }
        ret.push_back(intersection->t);
    }

    return ret;
}

aligned::vector<glm::vec3> compute_intersection_points(
        const aligned::vector<float>& distances, const geo::ray& ray) {
    return map_to_vector(distances, [&](const auto i) {
        return ray.get_position() + ray.get_direction() * i;
    });
}

aligned::vector<glm::vec3> compute_unmirrored_points(
        const aligned::vector<glm::vec3>& points,
        const aligned::vector<geo::triangle_vec3>& original) {
    assert(points.size() == original.size());

    aligned::vector<glm::vec3> ret = points;

    const auto lim = ret.size();
    for (auto i = 0u; i != lim; ++i) {
        const auto triangle = original[i];
        for (auto j = i + 1; j != lim; ++j) {
            ret[j] = geo::mirror(ret[j], triangle);
        }
    }

    return ret;
}

geo::ray construct_ray(const glm::vec3& from, const glm::vec3& to) {
    if (from == to) {
        throw std::runtime_error(
                "tried to construct a ray pointing towards its starting "
                "location");
    }
    return geo::ray(from, glm::normalize(to - from));
}

glm::vec3 compute_mirrored_point(
        const aligned::vector<geo::triangle_vec3>& mirrored,
        const glm::vec3& original) {
    return proc::accumulate(
            mirrored, original, [](const auto& ret, const auto& i) {
                return geo::mirror(ret, i);
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

volume_type compute_volume(const copyable_scene_data& scene_data,
                           const aligned::vector<cl_ulong>& triangles) {
    return proc::accumulate(
            triangles,
            volume_type{{1, 1, 1, 1, 1, 1, 1, 1}},
            [&](const auto& volume, const auto& i) {
                const auto scene_triangle = scene_data.get_triangles()[i];
                const auto surface =
                        scene_data.get_surfaces()[scene_triangle.surface];

                return volume * surface.specular;
            });
}

impulse compute_ray_path_impulse(const copyable_scene_data& scene_data,
                                 const aligned::vector<cl_ulong>& triangles,
                                 const glm::vec3& source,
                                 const aligned::vector<glm::vec3>& unmirrored,
                                 const glm::vec3& receiver,
                                 double speed_of_sound) {
    return construct_impulse(compute_volume(scene_data, triangles),
                             unmirrored.back(),
                             compute_distance(source, unmirrored, receiver),
                             speed_of_sound);
}

std::experimental::optional<impulse> follow_ray_path(
        const aligned::vector<cl_ulong>& triangles,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const voxelised_scene_data& voxelised,
        double speed_of_sound) {
    //  extract triangles from the scene
    const auto original =
            compute_original_triangles(triangles, voxelised.get_scene_data());

    //  mirror them into image-source space
    const auto mirrored = compute_mirrored_triangles(original);

    //  find mirrored receiver
    const auto receiver_image = compute_mirrored_point(mirrored, receiver);

    //  if the receiver is somehow at the same point as the source (don't ask)
    //  we bury our heads in the sand and return nullopt
    if (source == receiver_image) {
        return std::experimental::nullopt;
    }

    //  check that we can cast a ray through all the mirrored triangles to the
    //  receiver
    const auto ray = construct_ray(source, receiver_image);
    const auto distances = compute_intersection_distances(mirrored, ray);
    if (!distances) {
        return std::experimental::nullopt;
    }

    //  if we can, find the intersection points with the mirrored surfaces
    const auto points = compute_intersection_points(*distances, ray);

    //  now mirror the intersection points back into scene space
    const auto unmirrored{compute_unmirrored_points(points, original)};

    const auto does_intersect = [&](
            const auto& a, const auto& b, const auto& tri_to_ignore) {
        if (a == b) {
            return true;
        }
        const auto i{intersects(voxelised, geo::ray(a, b - a), tri_to_ignore)};
        const auto dist{glm::distance(a, b)};
        return i && almost_equal(i->inter.t, dist, 10);
    };

    if (!does_intersect(source, unmirrored.front(), ~size_t{0})) {
        return std::experimental::nullopt;
    }

    //  attempt to join the dots back in scene space
    {
        auto a{unmirrored.begin()};
        auto b{unmirrored.begin() + 1};
        auto c{triangles.begin()};
        for (; b != unmirrored.end(); ++a, ++b, ++c) {
            if (!does_intersect(*a, *b, *c)) {
                return std::experimental::nullopt;
            }
        }
    }

    if (!does_intersect(receiver, unmirrored.back(), ~size_t{0})) {
        return std::experimental::nullopt;
    }

    return compute_ray_path_impulse(voxelised.get_scene_data(),
                                    triangles,
                                    source,
                                    unmirrored,
                                    receiver,
                                    speed_of_sound);
}

}  // namespace raytracer
