#pragma once

#include "raytracer/image_source/postprocess_branches.h"
#include "raytracer/image_source/reflection_path_builder.h"
#include "raytracer/raytracer.h"
#include "raytracer/reflection_processor/image_source.h"
#include "raytracer/reflector.h"

namespace wayverb {
namespace raytracer {
namespace image_source {

//  Has to take voxelised_scene_data<cl_float3, surface> because it uses GPU.
template <typename It>
auto run(
        It b,  /// Iterators over ray directions.
        It e,
        const core::compute_context& cc,
        const core::voxelised_scene_data<cl_float3,
                                         core::surface<core::simulation_bands>>&
                voxelised,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const core::environment& environment) {
    const auto callbacks =
            std::make_tuple(raytracer::reflection_processor::make_image_source{
                    std::numeric_limits<size_t>::max()});

    auto results = raytracer::run(b,
                                  e,
                                  cc,
                                  voxelised,
                                  source,
                                  receiver,
                                  environment,
                                  true,
                                  [](auto /*i*/, auto /*steps*/) {},
                                  callbacks);

    if (!results) {
        throw std::runtime_error{"Raytracer failed to generate results."};
    }

    return std::move(std::get<0>(*results));
}

}  // namespace image_source
}  // namespace raytracer
}  // namespace wayverb
