#include "combined/engine.h"
#include "combined/postprocess.h"

#include "raytracer/canonical.h"
#include "raytracer/postprocess.h"
#include "raytracer/reflector.h"

#include "waveguide/canonical.h"

#include "core/attenuator/hrtf.h"
#include "core/attenuator/microphone.h"
#include "core/azimuth_elevation.h"
#include "core/callback_accumulator.h"
#include "core/cl/common.h"
#include "core/conversions.h"
#include "core/dc_blocker.h"
#include "core/filters_common.h"
#include "core/kernel.h"
#include "core/pressure_intensity.h"
#include "core/spatial_division/voxelised_scene_data.h"
#include "core/surfaces.h"

#include "utilities/event.h"

#include "hrtf/multiband.h"

#include <cmath>

namespace wayverb {
namespace combined {

template <typename Histogram>
class intermediate_impl : public intermediate {
public:
    intermediate_impl(combined_results<Histogram> to_process,
                      const glm::vec3& receiver_position,
                      double room_volume,
                      const core::environment& environment)
            : to_process_{std::move(to_process)}
            , receiver_position_{receiver_position}
            , room_volume_{room_volume}
            , environment_{environment} {}

    util::aligned::vector<float> postprocess(
            const core::attenuator::hrtf& attenuator,
            double sample_rate) const override {
        return postprocess_impl(attenuator, sample_rate);
    }

    util::aligned::vector<float> postprocess(
            const core::attenuator::microphone& attenuator,
            double sample_rate) const override {
        return postprocess_impl(attenuator, sample_rate);
    }

    util::aligned::vector<float> postprocess(
            const core::attenuator::null& attenuator,
            double sample_rate) const override {
        return postprocess_impl(attenuator, sample_rate);
    }

    engine_state_changed::scoped_connector
    add_scoped_engine_state_changed_callback(
            engine_state_changed::callback_type callback) override {
        return engine_state_changed_.add_scoped(std::move(callback));
    }

private:
    template <typename Attenuator>
    auto postprocess_impl(const Attenuator& attenuator,
                          double output_sample_rate) const {
        return wayverb::combined::postprocess(to_process_,
                                              attenuator,
                                              receiver_position_,
                                              room_volume_,
                                              environment_,
                                              output_sample_rate);
    }

    combined_results<Histogram> to_process_;
    glm::vec3 receiver_position_;
    double room_volume_;
    core::environment environment_;
    engine_state_changed engine_state_changed_;
};

template <typename Histogram>
auto make_intermediate_impl_ptr(combined_results<Histogram> to_process,
                                const glm::vec3& receiver_position,
                                double room_volume,
                                const core::environment& environment) {
    return std::make_unique<intermediate_impl<Histogram>>(
            std::move(to_process), receiver_position, room_volume, environment);
}

////////////////////////////////////////////////////////////////////////////////

template <typename WaveguideParameters>
std::unique_ptr<intermediate> engine::run(
        const core::compute_context& compute_context,
        const scene_data& scene_data,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const core::environment& environment,
        const raytracer::simulation_parameters& raytracer,
        const WaveguideParameters& waveguide,
        const std::atomic_bool& keep_going) const {
    //  RAYTRACER  /////////////////////////////////////////////////////////

    const auto rays_to_visualise = std::min(1000ul, raytracer.rays);

    engine_state_changed_(state::starting_raytracer, 1.0);

    auto raytracer_output = raytracer::canonical(
            compute_context,
            scene_data,
            source,
            receiver,
            environment,
            raytracer,
            rays_to_visualise,
            keep_going,
            [&](auto step, auto total_steps) {
                engine_state_changed_(state::running_raytracer,
                                      step / (total_steps - 1.0));
            });

    if (!(keep_going && raytracer_output)) {
        return nullptr;
    }

    engine_state_changed_(state::finishing_raytracer, 1.0);

    raytracer_reflections_generated_(std::move(raytracer_output->visual));

    //  look for the max time of an impulse
    const auto max_stochastic_time =
            max_time(raytracer_output->aural.stochastic);

    //  WAVEGUIDE  /////////////////////////////////////////////////////////
    engine_state_changed_(state::starting_waveguide, 1.0);

    auto waveguide_output = waveguide::canonical(
            compute_context,
            scene_data,
            source,
            receiver,
            environment,
            waveguide,
            max_stochastic_time,
            keep_going,
            [&](auto mesh_descriptor) {
                if (! waveguide_node_positions_changed_.empty()) {
                    waveguide_node_positions_changed_(
                            waveguide::compute_node_positions(mesh_descriptor));
                }
            },
            [&](auto& queue, const auto& buffer, auto step, auto steps) {
                //  If there are node pressure listeners.
                if (!waveguide_node_pressures_changed_.empty()) {
                    auto pressures =
                            core::read_from_buffer<float>(queue, buffer);
                    const auto time = step / waveguide.sample_rate;
                    waveguide_node_pressures_changed_(std::move(pressures),
                                                      time);
                }

                engine_state_changed_(state::running_waveguide,
                                      step / (steps - 1.0));
            });

    if (!(keep_going && waveguide_output)) {
        return nullptr;
    }

    engine_state_changed_(state::finishing_waveguide, 1.0);

    return make_intermediate_impl_ptr(
            make_combined_results(std::move(raytracer_output->aural),
                                  std::move(*waveguide_output)),
            receiver,
            estimate_room_volume(scene_data),
            environment);
}

/// Explicitly define for certain WaveguideParams.
template std::unique_ptr<intermediate> engine::run(
        const core::compute_context& compute_context,
        const scene_data& scene_data,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const core::environment& environment,
        const raytracer::simulation_parameters& raytracer,
        const waveguide::single_band_parameters& waveguide,
        const std::atomic_bool& keep_going) const;

template std::unique_ptr<intermediate> engine::run(
        const core::compute_context& compute_context,
        const scene_data& scene_data,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const core::environment& environment,
        const raytracer::simulation_parameters& raytracer,
        const waveguide::multiple_band_constant_spacing_parameters& waveguide,
        const std::atomic_bool& keep_going) const;

engine_state_changed::scoped_connector
engine::add_scoped_engine_state_changed_callback(
        engine_state_changed::callback_type callback) {
    return engine_state_changed_.add_scoped(std::move(callback));
}

engine::waveguide_node_positions_changed::scoped_connector
engine::add_scoped_waveguide_node_positions_changed_callback(
        waveguide_node_positions_changed::callback_type callback) {
    return waveguide_node_positions_changed_.add_scoped(std::move(callback));
}

engine::waveguide_node_pressures_changed::scoped_connector
engine::add_scoped_waveguide_node_pressures_changed_callback(
        waveguide_node_pressures_changed::callback_type callback) {
    return waveguide_node_pressures_changed_.add_scoped(std::move(callback));
}

engine::raytracer_reflections_generated::scoped_connector
engine::add_scoped_raytracer_reflections_generated_callback(
        raytracer_reflections_generated::callback_type callback) {
    return raytracer_reflections_generated_.add_scoped(std::move(callback));
}

}  // namespace combined
}  // namespace wayverb
