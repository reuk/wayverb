#pragma once

#include "raytracer/cl/structs.h"
#include "raytracer/image_source/postprocessors.h"
#include "raytracer/image_source/tree.h"

#include "common/aligned/vector.h"
#include "common/callback_accumulator.h"
#include "common/spatial_division/voxelised_scene_data.h"

#include <future>

namespace raytracer {
namespace image_source {

template <typename Func>
auto postprocess(const multitree<path_element>& tree,
                 const glm::vec3& source,
                 const glm::vec3& receiver,
                 const voxelised_scene_data<cl_float3, surface>& voxelised,
                 float acoustic_impedance) {
    callback_accumulator<Func> callback{
            receiver, voxelised.get_scene_data().get_surfaces(), false};
    find_valid_paths(
            tree,
            source,
            receiver,
            voxelised,
            [&](auto img, auto begin, auto end) { callback(img, begin, end); });
    return callback.get_output();
}

template <typename Func>
auto postprocess(const multitree<path_element>::branches_type& branches,
                 const glm::vec3& source,
                 const glm::vec3& receiver,
                 const voxelised_scene_data<cl_float3, surface>& voxelised,
                 float acoustic_impedance) {
    auto futures{map_to_vector(branches, [&](const auto& branch) {
        return std::async(std::launch::async, [&] {
            return postprocess<Func>(
                    branch, source, receiver, voxelised, acoustic_impedance);
        });
    })};

    using value_type = decltype(
            std::declval<typename decltype(futures)::value_type>().get());

    //  Collect futures.
    value_type ret;
    for (auto& fut : futures) {
        const auto thread_results{fut.get()};
        ret.insert(ret.end(), thread_results.begin(), thread_results.end());
    }

    return ret;
}

}  // namespace image_source
}  // namespace raytracer
