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

template <typename DirectionIt, typename CallbackIt>
void process_reflections(DirectionIt b_direction,
                         DirectionIt e_direction,
                         const scene_buffers& buffers,
                         const cl::Device& device,
                         const model::parameters& params,
                         const size_t reflection_depth,
                         const std::atomic_bool& keep_going,
                         CallbackIt b_callbacks,
                         CallbackIt e_callbacks) {
    const auto make_ray_iterator{[&](auto it) {
        return make_mapping_iterator_adapter(std::move(it), [&](const auto& i) {
            return geo::ray{params.source, i};
        });
    }};

    reflector ref{compute_context{buffers.get_context(), device},
                  params.receiver,
                  make_ray_iterator(b_direction),
                  make_ray_iterator(e_direction)};

    for (auto i{0ul}; i != reflection_depth; ++i) {
        if (!keep_going) {
            return;
        }

        const auto reflections{ref.run_step(buffers)};
        for (auto it{b_callbacks}; it != e_callbacks; ++it) {
            (*it)(begin(reflections), end(reflections), i, reflection_depth);
        }
    }
}

struct raytraced final {
    double sample_rate;
    aligned::vector<volume_type> diffuse_histogram;
    aligned::vector<volume_type> specular_histogram;
};

struct results final {
    aligned::vector<impulse<8>> img_src;
    raytraced raytraced;
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
                "run: can't visualise more rays than will be "
                "traced"};
    }

    const scene_buffers buffers{cc.context, voxelised};

    using reflection_it = aligned::vector<reflection>::const_iterator;
    using postprocessor =
            std::function<void(reflection_it, reflection_it, size_t, size_t)>;

    //  This collects the valid image source paths.
    image_source::reflection_path_builder builder{num_directions};

    //  This collects raw reflections to be visualised.
    iterative_builder<reflection> visual_results{rays_to_visualise};

    //  This will incrementally process and collect raytraced impulses.
    diffuse::finder dif{cc, params, 1.0f, num_directions};
    raytraced raytraced_results{1000.0};

    //  These are all the reflection-processing callbacks that we want to run.
    const aligned::vector<postprocessor> postprocessors{
            [&](auto b, auto e, auto step, auto reflections) {
                builder.push(b, e);
            },
            [&](auto b, auto e, auto step, auto reflections) {
                visual_results.push(b, b + rays_to_visualise);
            },
            [&](auto b, auto e, auto step, auto reflections) {
                auto output{dif.process(b, e, buffers)};
                const auto to_histogram{[&](auto& in, auto& out) {
                    std::for_each(begin(in), end(in), [&](auto& impulse) {
                        impulse.volume = pressure_to_intensity(
                                impulse.volume,
                                static_cast<float>(params.acoustic_impedance));
                    });

                    const auto make_iterator{[&](auto it) {
                        return make_histogram_iterator(std::move(it),
                                                       params.speed_of_sound);
                    }};
                    constexpr auto max_time{60.0};
                    incremental_histogram(out,
                                          make_iterator(begin(in)),
                                          make_iterator(end(in)),
                                          raytraced_results.sample_rate,
                                          max_time,
                                          dirac_sum_functor{});
                }};
                to_histogram(output.diffuse,
                             raytraced_results.diffuse_histogram);
                to_histogram(output.specular,
                             raytraced_results.specular_histogram);
            },
            [&](auto b, auto e, auto step, auto reflections) {
                callback(step, reflections);
            }};

    process_reflections(
            b_direction,
            e_direction,
            buffers,
            cc.device,
            params,
            compute_optimum_reflection_number(voxelised.get_scene_data()),
            keep_going,
            begin(postprocessors),
            end(postprocessors));

    if (!keep_going) {
        return std::experimental::nullopt;
    }

    image_source::tree tree{};
    for (const auto& path : builder.get_data()) {
        tree.push(path);
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
                   std::move(raytraced_results),
                   std::move(visual_results.get_data()),
                   params};
}

}  // namespace raytracer
