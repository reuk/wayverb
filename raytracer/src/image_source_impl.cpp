#include "raytracer/image_source_impl.h"
#include "common/stl_wrappers.h"
#include "raytracer/construct_impulse.h"

#include <numeric>

namespace raytracer {

aligned::vector<TriangleVec3> compute_original_triangles(
        const aligned::vector<cl_ulong>& triangles,
        const CopyableSceneData& scene_data) {
    return map_to_vector(triangles, [&](const auto i) {
        return get_triangle_verts(scene_data.get_triangles()[i],
                                  scene_data.get_vertices());
    });
}

aligned::vector<TriangleVec3> compute_mirrored_triangles(
        const aligned::vector<TriangleVec3>& original) {
    //  prepare output array
    aligned::vector<TriangleVec3> ret;
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
compute_intersection_distances(const aligned::vector<TriangleVec3>& mirrored,
                               const geo::Ray& ray) {
    aligned::vector<float> ret;
    ret.reserve(mirrored.size());

    for (const auto& i : mirrored) {
        const auto intersection = geo::triangle_intersection(i, ray);
        if (!intersection) {
            return std::experimental::nullopt;
        }
        ret.push_back(*intersection);
    }

    return ret;
}

aligned::vector<glm::vec3> compute_intersection_points(
        const aligned::vector<float>& distances, const geo::Ray& ray) {
    return map_to_vector(distances, [&](const auto i) {
        return ray.get_position() + ray.get_direction() * i;
    });
}

aligned::vector<glm::vec3> compute_unmirrored_points(
        const aligned::vector<glm::vec3>& points,
        const aligned::vector<TriangleVec3>& original) {
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

geo::Ray construct_ray(const glm::vec3& from, const glm::vec3& to) {
    if (from == to) {
        throw std::runtime_error(
                "tried to construct a ray pointing towards its starting "
                "location");
    }
    return geo::Ray(from, glm::normalize(to - from));
}

glm::vec3 compute_mirrored_point(const aligned::vector<TriangleVec3>& mirrored,
                                 const glm::vec3& original) {
    return proc::accumulate(
            mirrored, original, [](const auto& ret, const auto& i) {
                return geo::mirror(ret, i);
            });
}

float compute_distance(const aligned::vector<glm::vec3>& unmirrored) {
    return std::inner_product(
            unmirrored.begin(),
            unmirrored.end() - 1,
            unmirrored.begin() + 1,
            0.0f,
            [](const auto& a, const auto& b) { return a + b; },
            [](const auto& a, const auto& b) { return glm::distance(a, b); });
}

VolumeType compute_volume(const CopyableSceneData& scene_data,
                          const aligned::vector<cl_ulong>& triangles) {
    return proc::accumulate(
            triangles,
            VolumeType{{1, 1, 1, 1, 1, 1, 1, 1}},
            [&](const auto& volume, const auto& i) {
                const auto scene_triangle = scene_data.get_triangles()[i];
                const auto surface =
                        scene_data.get_surfaces()[scene_triangle.surface];

                return volume * -surface.specular;
            });
}

Impulse compute_ray_path_impulse(const CopyableSceneData& scene_data,
                                 const aligned::vector<cl_ulong>& triangles,
                                 const aligned::vector<glm::vec3>& unmirrored) {
    return construct_impulse(compute_volume(scene_data, triangles),
                             unmirrored[unmirrored.size() - 2],
                             compute_distance(unmirrored));
}

namespace {
template <typename T>
bool is_near(T a, T b, double tolerance) {
    return std::abs(a - b) < tolerance;
}
}  // namespace

std::experimental::optional<Impulse> follow_ray_path(
        const aligned::vector<cl_ulong>& triangles,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const CopyableSceneData& scene_data,
        const VoxelCollection& vox,
        const VoxelCollection::TriangleTraversalCallback& callback) {
    //  extract triangles from the scene
    const auto original = compute_original_triangles(triangles, scene_data);

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
    const auto ray       = construct_ray(source, receiver_image);
    const auto distances = compute_intersection_distances(mirrored, ray);
    if (!distances) {
        return std::experimental::nullopt;
    }

    //  if we can, find the intersection points with the mirrored surfaces
    const auto points = compute_intersection_points(*distances, ray);

    //  now mirror the intersection points back into scene space
    auto unmirrored = compute_unmirrored_points(points, original);
    unmirrored.insert(unmirrored.begin(), source);

    const auto intersects = [&](const auto& a, const auto& b) {
        const auto dir     = glm::normalize(b - a);
        const auto epsilon = 0.0001f;
        const auto from    = a + dir * epsilon;
        const auto to      = b;
        const auto i       = vox.traverse(geo::Ray(from, dir), callback);
        return i && is_near(i->distance, glm::distance(from, to), epsilon);
    };

    //  attempt to join the dots back in scene space
    for (auto a = unmirrored.begin(), b = unmirrored.begin() + 1;
         b != unmirrored.end();
         ++a, ++b) {
        if (*a == *b) {
            //  the point lies on two joined triangles
            continue;
        }
        if (!intersects(*a, *b)) {
            return std::experimental::nullopt;
        }
    }

    if (!intersects(receiver, unmirrored.back())) {
        return std::experimental::nullopt;
    }

    unmirrored.insert(unmirrored.end(), receiver);

    return compute_ray_path_impulse(scene_data, triangles, unmirrored);
}

}  // namespace raytracer
