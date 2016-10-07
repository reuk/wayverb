#pragma once

#include "raytracer/image_source/finder.h"
#include "raytracer/image_source/reflection_path_builder.h"
#include "raytracer/postprocess.h"
#include "raytracer/reflector.h"

namespace raytracer {
namespace image_source {

template <typename Func, typename It>
auto run(It begin,  /// Iterators over ray directions.
         It end,
         const compute_context& cc,
         const voxelised_scene_data<cl_float3, surface>& voxelised,
         const glm::vec3& source,
         const glm::vec3& receiver) {
    const auto reflection_depth{raytracer::compute_optimum_reflection_number(
            voxelised.get_scene_data())};

    const scene_buffers buffers{cc.context, voxelised};
    raytracer::reflector ref{
            cc,
            receiver,
            raytracer::get_rays_from_directions(begin, end, source)};

    //  This will collect the first reflections, to a specified depth,
    //  and use them to find unique image-source paths.
    tree tree{};
    {
        raytracer::image_source::reflection_path_builder builder{
                static_cast<size_t>(std::distance(begin, end))};

        //  Run the simulation proper.

        //  Up until the max reflection depth.
        for (auto i{0u}; i != reflection_depth; ++i) {
            //  Get a single step of the reflections.
            const auto reflections{ref.run_step(buffers)};

            //  Find diffuse impulses for these reflections.
            builder.push(reflections);
        }

        for (const auto& path : builder.get_data()) {
            tree.push(path);
        }
    }

    return postprocess<Func>(std::begin(tree.get_branches()),
                             std::end(tree.get_branches()),
                             source,
                             receiver,
                             voxelised);
}

}  // namespace image_source
}  // namespace raytracer
