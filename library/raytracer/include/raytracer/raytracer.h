#pragma once

#include "raytracer/diffuse/finder.h"
#include "raytracer/image_source/finder.h"
#include "raytracer/image_source/reflection_path_builder.h"
#include "raytracer/image_source/run.h"
#include "raytracer/raytracer.h"
#include "raytracer/reflector.h"
#include "raytracer/results.h"

#include "common/cl/common.h"
#include "common/cl/geometry.h"
#include "common/nan_checking.h"
#include "common/pressure_intensity.h"
#include "common/spatial_division/scene_buffers.h"
#include "common/spatial_division/voxelised_scene_data.h"

#include <experimental/optional>

namespace raytracer {

/// If there is line-of-sight between source and receiver, return the relative
/// time and intensity of the generated impulse.
template <typename Vertex, typename Surface>
auto get_direct(const glm::vec3& source,
                const glm::vec3& receiver,
                const voxelised_scene_data<Vertex, Surface>& scene_data) {
    constexpr auto channels{
            ::detail::components_v<specular_absorption_t<Surface>>};

    if (source == receiver) {
        return std::experimental::optional<impulse<channels>>{};
    }

    const auto source_to_receiver{receiver - source};
    const auto source_to_receiver_length{glm::length(source_to_receiver)};
    const auto direction{glm::normalize(source_to_receiver)};
    const geo::ray to_receiver{source, direction};

    const auto intersection{intersects(scene_data, to_receiver)};

    if (!intersection ||
        (intersection && intersection->inter.t >= source_to_receiver_length)) {
        return std::experimental::make_optional(impulse<channels>{
                unit_constructor_v<
                        ::detail::cl_vector_constructor_t<float, channels>>,
                to_cl_float3(source),
                source_to_receiver_length});
    }

    return std::experimental::optional<impulse<channels>>{};
}

//----------------------------------------------------------------------------//

/// arguments
///     the step number
using per_step_callback = std::function<void(size_t)>;

using reflection_processor =
        std::function<void(const aligned::vector<reflection>&)>;

template <typename It>
std::experimental::optional<results<impulse<8>>> run(
        It b_direction,
        It e_direction,
        const compute_context& cc,
        const voxelised_scene_data<cl_float3, surface>& voxelised,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const std::atomic_bool& keep_going,
        const per_step_callback& callback) {

    const scene_buffers buffers{cc.context, voxelised};

    size_t num_directions = std::distance(b_direction, e_direction);

    //  this is the object that generates first-pass reflections
    reflector ref{cc,
                  receiver,
                  get_rays_from_directions(b_direction, e_direction, source)};

    image_source::reflection_path_builder builder{num_directions};

    const auto reflection_depth{raytracer::compute_optimum_reflection_number(
            voxelised.get_scene_data())};

    //  this will incrementally process diffuse responses
    diffuse::finder dif{cc, source, receiver, num_directions, reflection_depth};

    image_source::tree tree{};

    //  run the simulation proper
    {
        //  up until the max reflection depth
        for (auto i{0ul}; i != reflection_depth; ++i) {
            //  if the user cancelled, return an empty result
            if (!keep_going) {
                return std::experimental::nullopt;
            }

            //  get a single step of the reflections
            const auto reflections{ref.run_step(buffers)};

            //  find diffuse impulses for these reflections
            dif.push(reflections, buffers);

            //  push ray paths
            builder.push(reflections);

            callback(i);
        }

        for (const auto& path : builder.get_data()) {
            tree.push(path);
        }
    }

    if (!keep_going) {
        return std::experimental::nullopt;
    }

    //  fetch image source results
    auto img_src_results{
            image_source::postprocess<image_source::intensity_calculator<>>(
                    begin(tree.get_branches()),
                    end(tree.get_branches()),
                    source,
                    receiver,
                    voxelised)};

    auto direct{get_direct(source, receiver, voxelised)};

    return results<impulse<8>>{std::move(direct),
                               std::move(img_src_results),
                               std::move(dif.get_results()),
                               receiver};
}

}  // namespace raytracer
