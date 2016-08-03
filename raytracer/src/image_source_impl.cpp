#include "raytracer/image_source_impl.h"
#include "raytracer/construct_impulse.h"

#include <numeric>

namespace raytracer {

aligned::vector<TriangleVec3> compute_original_triangles(
        const aligned::vector<cl_ulong>& triangles,
        const CopyableSceneData& scene_data) {
    aligned::vector<TriangleVec3> ret;
    ret.reserve(triangles.size());

    for (const auto& triangle_index : triangles) {
        ret.push_back(
                get_triangle_verts(scene_data.get_triangles()[triangle_index],
                                   scene_data.get_vertices()));
    }

    return ret;
}

aligned::vector<TriangleVec3> compute_mirrored_triangles(
        const aligned::vector<TriangleVec3>& original,
        const CopyableSceneData& scene_data) {
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
        auto intersection = geo::triangle_intersection(i, ray);
        if (!intersection) {
            return std::experimental::nullopt;
        }
        ret.push_back(*intersection);
    }

    return ret;
}

aligned::vector<glm::vec3> compute_intersection_points(
        const aligned::vector<float>& distances, const geo::Ray& ray) {
    aligned::vector<glm::vec3> ret;
    ret.reserve(distances.size());

    for (const auto& i : distances) {
        ret.push_back(ray.get_position() + ray.get_direction() * i);
    }

    return ret;
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
    return geo::Ray(from, glm::normalize(to - from));
}

glm::vec3 compute_mirrored_point(const aligned::vector<TriangleVec3>& mirrored,
                                 const glm::vec3& original) {
    auto ret = original;

    for (const auto& i : mirrored) {
        ret = geo::mirror(ret, i);
    }

    return ret;
}

float compute_distance(const aligned::vector<glm::vec3>& unmirrored) {
    return std::inner_product(
            unmirrored.begin(),
            unmirrored.end() - 1,
            unmirrored.begin() + 1,
            0.0f,
            [](auto a, auto b) { return a + b; },
            [](auto a, auto b) { return glm::distance(a, b); });
}

VolumeType compute_volume(const CopyableSceneData& scene_data,
                          const aligned::vector<cl_ulong>& triangles) {
    VolumeType volume{{1, 1, 1, 1, 1, 1, 1, 1}};
    for (auto i : triangles) {
        const auto scene_triangle = scene_data.get_triangles()[i];
        const auto surface = scene_data.get_surfaces()[scene_triangle.surface];

        volume *= -surface.specular;
    }
    return volume;
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
    const auto mirrored = compute_mirrored_triangles(original, scene_data);

    //  find mirrored receiver
    const auto receiver_image = compute_mirrored_point(mirrored, receiver);

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

    auto intersects = [&](auto a, auto b) {
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
