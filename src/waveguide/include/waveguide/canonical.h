#pragma once

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

/// The waveguide algorithm in waveguide.h is modular, in that
/// you can supply different combinations of sources and receivers.
/// The method below drives the combination deemed to be most approriate for
/// single-run simulation.

namespace wayverb {
namespace waveguide {

struct band final {
    util::aligned::vector<postprocessor::directional_receiver::output>
            directional;
    double sample_rate;
};

struct bandpass_band final {
    band band;
    util::range<double> valid_hz;
};

namespace detail {

template <typename Callback>
std::experimental::optional<band> canonical_impl(
        const core::compute_context& cc,
        const mesh& mesh,
        double sample_rate,
        double simulation_time,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const core::environment& environment,
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
        auto raw = util::aligned::vector<float>(ideal_steps, 0.0f);
        raw.front() = rectilinear_calibration_factor(
                mesh.get_descriptor().spacing, environment.acoustic_impedance);
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

struct simulation_results final {
    util::aligned::vector<bandpass_band> bands;
};

/// Run a waveguide using:
///     specified sample rate
///     receiver at specified location
///     source at closest available location
///     single hard source
///     single directional receiver
template <typename Callback>
std::experimental::optional<simulation_results> canonical(
        const core::compute_context& cc,
        const core::generic_scene_data<cl_float3,
                                       core::surface<core::simulation_bands>>&
                scene,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const core::environment& environment,
        const single_band_parameters& sim_params,
        double simulation_time,
        const std::atomic_bool& keep_going,
        Callback&& callback) {
    auto voxelised = compute_voxels_and_mesh(cc,
                                             scene,
                                             receiver,
                                             sim_params.sample_rate,
                                             environment.speed_of_sound);

    if (auto ret = detail::canonical_impl(cc,
                                          voxelised.mesh,
                                          sim_params.sample_rate,
                                          simulation_time,
                                          source,
                                          receiver,
                                          environment,
                                          keep_going,
                                          std::forward<Callback>(callback))) {
        return simulation_results{{bandpass_band{
                std::move(*ret),
                util::make_range(
                        0.0,
                        compute_cutoff_frequency(sim_params.sample_rate,
                                                 sim_params.usable_portion))}}};
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

/// This method shows a different approach which more accurately simulates
/// frequency-dependent boundaries, but which runs several times slower.
template <typename Callback>
[
        [deprecated("really slow and inaccurate compared to the single-band "
                    "version")]] std::experimental::optional<simulation_results>
canonical(const core::compute_context& cc,
          const core::generic_scene_data<cl_float3,
                                         core::surface<core::simulation_bands>>&
                  scene,
          const glm::vec3& source,
          const glm::vec3& receiver,
          const core::environment& environment,
          const multiple_band_variable_spacing_parameters& sim_params,
          double simulation_time,
          const std::atomic_bool& keep_going,
          Callback&& callback) {
    const auto band_params = hrtf_data::hrtf_band_params_hz();

    simulation_results ret{};

    //  For each band, up to the maximum band specified.
    for (auto band = 0; band != sim_params.bands; ++band) {
        //  Find the waveguide sampling rate required.
        const auto sample_rate = compute_sampling_frequency(
                band_params.edges[band + 1], sim_params.usable_portion);

        //  Generate a mesh using the largest possible grid spacing.
        const auto mesh = [&] {
            auto ret = compute_voxels_and_mesh(cc,
                                               scene,
                                               receiver,
                                               sample_rate,
                                               environment.speed_of_sound);

            set_flat_coefficients_for_band(ret, band);

            return ret.mesh;
        }();
        if (auto rendered_band = detail::canonical_impl(cc,
                                                        mesh,
                                                        sample_rate,
                                                        simulation_time,
                                                        source,
                                                        receiver,
                                                        environment,
                                                        keep_going,
                                                        callback)) {
            ret.bands.emplace_back(bandpass_band{
                    std::move(*rendered_band),
                    util::make_range(band_params.edges[band],
                                     band_params.edges[band + 1])});
        } else {
            return std::experimental::nullopt;
        }
    }

    return ret;
}

/// This is a sort of middle ground - more accurate boundary modelling, but
/// really unbelievably slow.
template <typename Callback>
std::experimental::optional<simulation_results> canonical(
        const core::compute_context& cc,
        const core::generic_scene_data<cl_float3,
                                       core::surface<core::simulation_bands>>&
                scene,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const core::environment& environment,
        const multiple_band_constant_spacing_parameters& sim_params,
        double simulation_time,
        const std::atomic_bool& keep_going,
        Callback&& callback) {
    const auto band_params = hrtf_data::hrtf_band_params_hz();

    //  Find the waveguide sampling rate required.
    auto voxels_and_mesh = compute_voxels_and_mesh(cc,
                                                   scene,
                                                   receiver,
                                                   sim_params.sample_rate,
                                                   environment.speed_of_sound);

    simulation_results ret{};

    //  For each band, up to the maximum band specified.
    for (auto band = 0; band != sim_params.bands; ++band) {
        set_flat_coefficients_for_band(voxels_and_mesh, band);

        if (auto rendered_band = detail::canonical_impl(cc,
                                                        voxels_and_mesh.mesh,
                                                        sim_params.sample_rate,
                                                        simulation_time,
                                                        source,
                                                        receiver,
                                                        environment,
                                                        keep_going,
                                                        callback)) {
            ret.bands.emplace_back(bandpass_band{
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
