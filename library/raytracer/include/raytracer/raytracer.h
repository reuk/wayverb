#pragma once

#include "raytracer/diffuse/finder.h"
#include "raytracer/image_source/finder.h"
#include "raytracer/image_source/reflection_path_builder.h"
#include "raytracer/image_source/run.h"
#include "raytracer/raytracer.h"
#include "raytracer/reflector.h"
#include "raytracer/results.h"

#include "common/channels.h"
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
    constexpr auto channels{channels_v<Surface>};

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

struct raytracer_output final {
    using audio_t = results<impulse<8>>;
    using visual_t = aligned::vector<aligned::vector<reflection>>;

    audio_t audio;
    visual_t visual;
};

/// arguments
///     the step number
template <typename It, typename PerStepCallback>
std::experimental::optional<raytracer_output> run(
        It b_direction,
        It e_direction,
        const compute_context& cc,
        const voxelised_scene_data<cl_float3, surface>& voxelised,
        const glm::vec3& source,
        const glm::vec3& receiver,
        size_t rays_to_visualise,
        const std::atomic_bool& keep_going,
        const PerStepCallback& callback) {
    const size_t num_directions = std::distance(b_direction, e_direction);
    if (num_directions < rays_to_visualise) {
        throw std::runtime_error{
                "raytracer::run: can't visualise more rays than will be "
                "traced"};
    }

    const scene_buffers buffers{cc.context, voxelised};

    //  This is the object that generates first-pass reflections.
    reflector ref{cc,
                  receiver,
                  get_rays_from_directions(b_direction, e_direction, source)};

    //  This collects the valid image source paths.
    image_source::reflection_path_builder builder{num_directions};

    //  This will incrementally process diffuse responses.
    diffuse::finder dif{cc, source, receiver, num_directions};

    //  This collects raw reflections to be visualised.
    iterative_builder<reflection> reflection_data{rays_to_visualise};

    image_source::tree tree{};

    //  run the simulation proper
    {
        //  up until the max reflection depth
        const auto reflection_depth{
                raytracer::compute_optimum_reflection_number(
                        voxelised.get_scene_data())};
        for (auto i{0ul}; i != reflection_depth; ++i) {
            //  if the user cancelled, return an empty result
            if (!keep_going) {
                return std::experimental::nullopt;
            }

            const auto reflections{ref.run_step(buffers)};
            builder.push(begin(reflections), end(reflections));
            dif.push(begin(reflections), end(reflections), buffers);
            reflection_data.push(begin(reflections),
                                 begin(reflections) + rays_to_visualise);

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
            image_source::postprocess<image_source::fast_pressure_calculator<>>(
                    begin(tree.get_branches()),
                    end(tree.get_branches()),
                    source,
                    receiver,
                    voxelised)};

    if (const auto direct{get_direct(source, receiver, voxelised)}) {
        // img_src_results.insert(img_src_results.begin(), *direct);
        img_src_results.emplace_back(*direct);
    }

    return raytracer_output{raytracer_output::audio_t{begin(img_src_results),
                                                      end(img_src_results),
                                                      begin(dif.get_data()),
                                                      end(dif.get_data()),
                                                      receiver},
                            std::move(reflection_data.get_data())};
}

}  // namespace raytracer
