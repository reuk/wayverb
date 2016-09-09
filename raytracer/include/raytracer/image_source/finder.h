#pragma once

#include "raytracer/cl/structs.h"
#include "raytracer/image_source/postprocessors.h"
#include "raytracer/image_source/tree.h"

#include "common/aligned/vector.h"

#include <future>

class voxelised_scene_data;

namespace raytracer {
namespace image_source {

template <typename Func>
auto postprocess(const multitree<path_element>& tree,
                 const glm::vec3& source,
                 const glm::vec3& receiver,
                 const voxelised_scene_data& voxelised,
                 const Func& callback) {
    using value_type = decltype(
            callback(std::declval<glm::vec3>(),
                     std::declval<aligned::vector<reflection_metadata>>()));
    aligned::vector<value_type> ret{};
    find_valid_paths(tree,
                     source,
                     receiver,
                     voxelised,
                     [&callback, &ret](const auto& a, const auto& b) {
                         ret.emplace_back(callback(a, b));
                     });
    return ret;
}

template <typename Func>
auto postprocess(const multitree<path_element>::branches_type& branches,
                 const glm::vec3& source,
                 const glm::vec3& receiver,
                 const voxelised_scene_data& voxelised,
                 const Func& callback) {
    using value_type = decltype(postprocess(
            *branches.begin(), source, receiver, voxelised, callback));

    auto futures{map_to_vector(branches, [&](const auto& branch) {
        return std::async(std::launch::async, [=] {
            return postprocess(branch, source, receiver, voxelised, callback);
        });
    })};

    //  Collect futures.
    value_type ret;
    for (auto& fut : futures) {
        const auto thread_results{fut.get()};
        ret.insert(ret.end(), thread_results.begin(), thread_results.end());
    }

    //  C++11 is the biz.
    return ret;
}

}  // namespace image_source
}  // namespace raytracer
