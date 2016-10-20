#pragma once

#include "raytracer/image_source/postprocess_branches.h"
#include "raytracer/image_source/reflection_path_builder.h"
#include "raytracer/raytracer.h"
#include "raytracer/reflection_processor/image_source.h"
#include "raytracer/reflector.h"

namespace raytracer {
namespace image_source {

//  Has to take voxelised_scene_data<cl_float3, surface> because it uses GPU.
template <typename It>
auto run(It b,  /// Iterators over ray directions.
         It e,
         const compute_context& cc,
         const voxelised_scene_data<cl_float3, surface<simulation_bands>>&
                 voxelised,
         const model::parameters& params,
         bool flip_phase) {
    const auto callbacks =
            std::make_tuple(raytracer::reflection_processor::make_image_source{
                    std::numeric_limits<size_t>::max(), flip_phase});
    auto results = raytracer::run(b,
                                  e,
                                  cc,
                                  voxelised,
                                  params,
                                  true,
                                  [](auto i, auto steps) {},
                                  callbacks);

    if (!results) {
        throw std::runtime_error{"raytracer failed to generate results"};
    }

    return std::move(std::get<0>(*results));
}

}  // namespace image_source
}  // namespace raytracer
