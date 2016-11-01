#pragma once

#include "waveguide/calibration.h"
#include "waveguide/fitted_boundary.h"
#include "waveguide/postprocessor/directional_receiver.h"
#include "waveguide/preprocessor/hard_source.h"
#include "waveguide/simulation_parameters.h"
#include "waveguide/waveguide.h"

#include "common/callback_accumulator.h"
#include "common/model/parameters.h"
#include "common/reverb_time.h"

#include "hrtf/multiband.h"

#include <cmath>

/// The waveguide algorithm in waveguide.h is modular, in that
/// you can supply different combinations of sources and receivers.
/// The method below drives the combination deemed to be most approriate for
/// single-run simulation.

namespace waveguide {

struct band final {
    aligned::vector<postprocessor::directional_receiver::output> directional;
    double sample_rate;
};

namespace detail {

template <typename Callback>
std::experimental::optional<band> canonical_impl(
        const compute_context& cc,
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

    return band{std::move(output_accumulator.get_output()), sample_rate};
}

}  // namespace detail

////////////////////////////////////////////////////////////////////////////////

struct simulation_results final {
    aligned::vector<band> bands;
    double usable_portion;
};

double compute_cutoff_frequency(const simulation_results& results);

/// Run a waveguide using:
///     specified sample rate
///     receiver at specified location
///     source at closest available location
///     single hard source
///     single directional receiver
template <typename Callback>
std::experimental::optional<simulation_results> canonical(
        const compute_context& cc,
        const generic_scene_data<cl_float3, surface<simulation_bands>>& scene,
        const model::parameters& params,
        const single_band_parameters& sim_params,
        double simulation_time,
        const std::atomic_bool& keep_going,
        Callback&& callback) {
    auto voxelised = compute_voxels_and_mesh(cc,
                                             scene,
                                             params.receiver,
                                             sim_params.sample_rate,
                                             params.speed_of_sound);

    if (auto ret = detail::canonical_impl(cc,
                                          voxelised.mesh,
                                          sim_params.sample_rate,
                                          simulation_time,
                                          params,
                                          keep_going,
                                          std::forward<Callback>(callback))) {
        return simulation_results{{std::move(*ret)}, sim_params.usable_portion};
    }

    return std::experimental::nullopt;
}

////////////////////////////////////////////////////////////////////////////////

/// This method shows a different approach which more accurately simulates
/// frequency-dependent boundaries, but which runs several times slower.
template <typename Callback>
std::experimental::optional<simulation_results> canonical(
        const compute_context& cc,
        const generic_scene_data<cl_float3, surface<simulation_bands>>& scene,
        const model::parameters& params,
        const multiple_band_parameters& sim_params,
        double simulation_time,
        const std::atomic_bool& keep_going,
        Callback&& callback) {
    const auto band_params = hrtf_data::hrtf_band_params_hz();

    simulation_results ret{{}, sim_params.usable_portion};

    //  For each band, up to the maximum band specified.
    for (auto band = 0; band != sim_params.bands; ++band) {
        //  Find the waveguide sampling rate required.
        const auto sample_rate = band_params.edges[band + 1] /
                                 (0.25 * sim_params.usable_portion);

        //  Generate a mesh using the largest possible grid spacing.
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

        if (auto rendered_band = detail::canonical_impl(cc,
                                                        mesh,
                                                        sample_rate,
                                                        simulation_time,
                                                        params,
                                                        keep_going,
                                                        callback)) {
            ret.bands.emplace_back(*rendered_band);
        } else {
            return std::experimental::nullopt;
        }
    }

    return ret;
}

}  // namespace waveguide
