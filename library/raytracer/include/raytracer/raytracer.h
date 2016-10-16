#pragma once

#include "raytracer/diffuse/finder.h"
#include "raytracer/diffuse/postprocessing.h"
#include "raytracer/get_direct.h"
#include "raytracer/image_source/finder.h"
#include "raytracer/image_source/reflection_path_builder.h"
#include "raytracer/image_source/run.h"
#include "raytracer/raytracer.h"
#include "raytracer/reflector.h"

#include "common/cl/common.h"
#include "common/cl/geometry.h"
#include "common/model/parameters.h"
#include "common/nan_checking.h"
#include "common/pressure_intensity.h"
#include "common/spatial_division/scene_buffers.h"
#include "common/spatial_division/voxelised_scene_data.h"

#include <experimental/optional>
#include <iostream>

namespace raytracer {

struct results final {
    aligned::vector<impulse<8>> img_src;
    aligned::vector<impulse<8>> diffuse;
    aligned::vector<aligned::vector<reflection>> visual;
    model::parameters parameters;
};

/// arguments
///     the step number
template <typename It, typename PerStepCallback>
std::experimental::optional<results> run(
        It b_direction,
        It e_direction,
        const compute_context& cc,
        const voxelised_scene_data<cl_float3, surface>& voxelised,
        const model::parameters& params,
        size_t rays_to_visualise,
        bool flip_phase,
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
    const auto make_ray_iterator{[&](auto it) {
        return make_mapping_iterator_adapter(std::move(it), [&](const auto& i) {
            return geo::ray{params.source, i};
        });
    }};

    reflector ref{cc,
                  params.receiver,
                  make_ray_iterator(b_direction),
                  make_ray_iterator(e_direction)};

    //  This collects the valid image source paths.
    image_source::reflection_path_builder builder{num_directions};

    //  This will incrementally process diffuse responses.
    //  TODO Slow, but gives more flexibility for posptprocessing.
    diffuse::finder dif{cc, params, 1.0f, num_directions};
    aligned::vector<impulse<8>> diffuse_results;

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
            reflection_data.push(begin(reflections),
                                 begin(reflections) + rays_to_visualise);

            const auto dif_step_output{
                    dif.process(begin(reflections), end(reflections), buffers)};
            diffuse_results.insert(end(diffuse_results),
                                   begin(dif_step_output),
                                   end(dif_step_output));

            callback(i, reflection_depth);
        }

        for (const auto& path : builder.get_data()) {
            tree.push(path);
        }
    }

    if (!keep_going) {
        return std::experimental::nullopt;
    }

    const auto img_src_results{[&] {
        //  fetch image source results
        auto ret{image_source::postprocess<
                image_source::fast_pressure_calculator<>>(
                begin(tree.get_branches()),
                end(tree.get_branches()),
                params.source,
                params.receiver,
                voxelised,
                flip_phase)};

        if (const auto direct{
                    get_direct(params.source, params.receiver, voxelised)}) {
            // img_src_results.insert(img_src_results.begin(), *direct);
            ret.emplace_back(*direct);
        }

        //  Correct for distance travelled.
        for (auto& imp : ret) {
            imp.volume *= pressure_for_distance(imp.distance,
                                                params.acoustic_impedance);
        }

        return ret;
    }()};

    return results{std::move(img_src_results),
                   std::move(diffuse_results),
                   std::move(reflection_data.get_data()),
                   params};
}

}  // namespace raytracer
