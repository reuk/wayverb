#pragma once

#include "waveguide/bandpass_band.h"
#include "waveguide/calibration.h"
#include "waveguide/fitted_boundary.h"
#include "waveguide/postprocessor/directional_receiver.h"
#include "waveguide/preprocessor/hard_source.h"
#include "waveguide/simulation_parameters.h"
#include "waveguide/waveguide.h"

#include "core/callback_accumulator.h"
#include "core/environment.h"
#include "core/reverb_time.h"

#include "hrtf/multiband.h"

#include <cmath>

/// \file canonical.h
/// The waveguide algorithm in waveguide.h is modular, in that
/// you can supply different combinations of sources and receivers.
/// The method below drives the combination deemed to be most approriate for
/// single-run simulation.

namespace wayverb {
namespace waveguide {
namespace detail {

template <typename Callback>
std::experimental::optional<band> canonical_impl(
        const core::compute_context& cc,
        const mesh& mesh,
        double simulation_time,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const core::environment& environment,
        const std::atomic_bool& keep_going,
        Callback&& callback) {
    const auto sample_rate = compute_sample_rate(mesh.get_descriptor(),
                                                 environment.speed_of_sound);

    const auto compute_mesh_index = [&](const auto& pt) {
        const auto ret = compute_index(mesh.get_descriptor(), pt);
        if (!waveguide::is_inside(
                    mesh.get_structure().get_condensed_nodes()[ret])) {
            throw std::runtime_error{
                    "Source/receiver node position appears to be outside "
                    "mesh."};
        }
        return ret;
    };

    const auto ideal_steps = std::ceil(sample_rate * simulation_time);

    const auto input = [&] {
        auto raw = util::aligned::vector<float>(ideal_steps, 0.0f);
        if (!raw.empty()) {
            raw.front() = rectilinear_calibration_factor(
                    mesh.get_descriptor().spacing,
                    environment.acoustic_impedance);
        }
        return raw;
    }();

    auto output_accumulator =
            core::callback_accumulator<postprocessor::directional_receiver>{
                    mesh.get_descriptor(),
                    sample_rate,
                    get_ambient_density(environment),
                    compute_mesh_index(receiver)};

    const auto steps =
            run(cc,
                mesh,
                preprocessor::make_hard_source(
                        compute_mesh_index(source), begin(input), end(input)),
                [&](auto& queue, const auto& buffer, auto step) {
                    output_accumulator(queue, buffer, step);
                    callback(queue, buffer, step, ideal_steps);
                },
                keep_going);

    if (steps != ideal_steps) {
        return std::experimental::nullopt;
    }

    return band{std::move(output_accumulator.get_output()), sample_rate};
}

}  // namespace detail

////////////////////////////////////////////////////////////////////////////////

/// Run a waveguide using:
///     specified sample rate
///     receiver at specified location
///     source at closest available location
///     single hard source
///     single directional receiver
template <typename PressureCallback>
std::experimental::optional<util::aligned::vector<bandpass_band>> canonical(
        const core::compute_context& cc,
        voxels_and_mesh voxelised,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const core::environment& environment,
        const single_band_parameters& sim_params,
        double simulation_time,
        const std::atomic_bool& keep_going,
        PressureCallback&& pressure_callback) {
    if (auto ret = detail::canonical_impl(cc,
                                          voxelised.mesh,
                                          simulation_time,
                                          source,
                                          receiver,
                                          environment,
                                          keep_going,
                                          pressure_callback)) {
        return util::aligned::vector<bandpass_band>{bandpass_band{
                std::move(*ret), util::make_range(0.0, sim_params.cutoff)}};
    }

    return std::experimental::nullopt;
}

////////////////////////////////////////////////////////////////////////////////

inline auto set_flat_coefficients_for_band(voxels_and_mesh& voxels_and_mesh,
                                           size_t band) {
    voxels_and_mesh.mesh.set_coefficients(util::map_to_vector(
            begin(voxels_and_mesh.voxels.get_scene_data().get_surfaces()),
            end(voxels_and_mesh.voxels.get_scene_data().get_surfaces()),
            [&](const auto& surface) {
                return to_flat_coefficients(surface.absorption.s[band]);
            }));
}

/// This is a sort of middle ground - more accurate boundary modelling, but
/// really unbelievably slow.
template <typename PressureCallback>
std::experimental::optional<util::aligned::vector<bandpass_band>> canonical(
        const core::compute_context& cc,
        voxels_and_mesh voxelised,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const core::environment& environment,
        const multiple_band_constant_spacing_parameters& sim_params,
        double simulation_time,
        const std::atomic_bool& keep_going,
        PressureCallback&& pressure_callback) {
    const auto band_params = hrtf_data::hrtf_band_params_hz();

    util::aligned::vector<bandpass_band> ret{};

    //  For each band, up to the maximum band specified.
    for (auto band = 0; band != sim_params.bands; ++band) {
        set_flat_coefficients_for_band(voxelised, band);

        if (auto rendered_band = detail::canonical_impl(cc,
                                                        voxelised.mesh,
                                                        simulation_time,
                                                        source,
                                                        receiver,
                                                        environment,
                                                        keep_going,
                                                        pressure_callback)) {
            ret.emplace_back(bandpass_band{
                    std::move(*rendered_band),
                    util::make_range(band_params.edges[band],
                                     band_params.edges[band + 1])});
        } else {
            return std::experimental::nullopt;
        }
    }

    return ret;
}

}  // namespace waveguide
}  // namespace wayverb
