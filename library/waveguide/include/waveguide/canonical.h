#pragma once

#include "waveguide/calibration.h"
#include "waveguide/postprocessor/directional_receiver.h"
#include "waveguide/preprocessor/hard_source.h"
#include "waveguide/waveguide.h"

#include "common/callback_accumulator.h"
#include "common/reverb_time.h"

#include <cmath>

/// The waveguide algorithm in waveguide.h is modular, in that
/// you can supply different combinations of sources and receivers.
/// The method below drives the combination deemed to be most approriate for
/// single-run simulation.

namespace waveguide {

namespace detail {
template <typename Callback>
std::experimental::optional<
        aligned::vector<postprocessor::directional_receiver::output>>
canonical_impl(const compute_context& cc,
               const mesh& mesh,
               double sample_rate,
               double simulation_time,
               const model::parameters& params,
               const std::atomic_bool& keep_going,
               Callback&& callback) {
    const auto compute_mesh_index = [&](const auto& pt) {
        const auto ret = compute_index(mesh.get_descriptor(), pt);
        if (!waveguide::is_inside(
                    mesh.get_structure().get_condensed_nodes()[ret])) {
            throw std::runtime_error{
                    "source/receiver node position appears to be outside mesh"};
        }
        return ret;
    };

    const auto ideal_steps = std::ceil(sample_rate * simulation_time);

    const auto input = [&] {
        auto raw = aligned::vector<float>(ideal_steps, 0.0f);
        raw.front() = waveguide::rectilinear_calibration_factor(
                mesh.get_descriptor().spacing, params.acoustic_impedance);
        return raw;
    }();

    auto output_accumulator = callback_accumulator<
            waveguide::postprocessor::directional_receiver>{
            mesh.get_descriptor(),
            sample_rate,
            params.acoustic_impedance / params.speed_of_sound,
            compute_mesh_index(params.receiver)};

    const auto steps =
            waveguide::run(cc,
                           mesh,
                           waveguide::preprocessor::make_hard_source(
                                   compute_mesh_index(params.source),
                                   begin(input),
                                   end(input)),
                           [&](auto& queue, const auto& buffer, auto step) {
                               output_accumulator(queue, buffer, step);
                               callback(step, ideal_steps);
                           },
                           keep_going);

    if (steps != ideal_steps) {
        return std::experimental::nullopt;
    }

    return std::move(output_accumulator.get_output());
}

}  // namespace detail

////////////////////////////////////////////////////////////////////////////////

/// Run a waveguide using:
///     specified sample rate
///     receiver at specified location
///     source at closest available location
///     single hard source
///     single directional receiver
template <typename Callback>
std::experimental::optional<
        aligned::vector<postprocessor::directional_receiver::output>>
canonical_single_band(
        const compute_context& cc,
        const generic_scene_data<cl_float3, surface<simulation_bands>>& scene,
        double sample_rate,
        double simulation_time,
        const model::parameters& params,
        const std::atomic_bool& keep_going,
        Callback&& callback) {
    return detail::canonical_impl(cc,
                                  compute_voxels_and_mesh(cc,
                                                          scene,
                                                          params.receiver,
                                                          sample_rate,
                                                          params.speed_of_sound)
                                          .mesh,
                                  sample_rate,
                                  simulation_time,
                                  params,
                                  keep_going,
                                  std::forward<Callback>(callback));
}

////////////////////////////////////////////////////////////////////////////////

/// This method shows a different approach which more accurately simulates
/// frequency-dependent boundaries, but which runs several times slower.
template <typename Callback>
std::experimental::optional<aligned::vector<
        aligned::vector<postprocessor::directional_receiver::output>>>
canonical_multiple_band(
        const compute_context& cc,
        const generic_scene_data<cl_float3, surface<simulation_bands>>& scene,
        double sample_rate,
        double simulation_time,
        size_t max_bands,
        const model::parameters& params,
        const std::atomic_bool& keep_going,
        Callback&& callback) {
    aligned::vector<
            aligned::vector<postprocessor::directional_receiver::output>>
            ret;

    for (auto band = 0; band != max_bands; ++band) {
        const auto mesh = [&] {
            auto ret = compute_voxels_and_mesh(cc,
                                               scene,
                                               params.receiver,
                                               sample_rate,
                                               params.speed_of_sound);

            //  By default, the mesh will attempt to generate a fitted boundary
            //  filter for each surface.
            //  We're going to replace that filter with a flat-response filter
            //  for this frequency band.
            ret.mesh.set_coefficients(map_to_vector(
                    begin(ret.voxels.get_scene_data().get_surfaces()),
                    end(ret.voxels.get_scene_data().get_surfaces()),
                    [&](const auto& surface) {
                        return to_flat_coefficients(surface.absorption.s[band]);
                    }));

            return ret.mesh;
        }();

        ret.emplace_back(detail::canonical_impl(cc,
                                                mesh,
                                                sample_rate,
                                                simulation_time,
                                                params,
                                                keep_going,
                                                callback));
    }

    return ret;
}

}  // namespace waveguide
