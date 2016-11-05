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

class engine::impl {
public:
    impl() = default;

    impl(const impl&) = default;
    impl(impl&&) noexcept = default;

    impl& operator=(const impl&) = default;
    impl& operator=(impl&&) noexcept = default;

    virtual ~impl() noexcept = default;

    virtual std::unique_ptr<intermediate> run(
            const std::atomic_bool& keep_going) const = 0;

    virtual engine_state_changed::scoped_connector
    add_scoped_engine_state_changed_callback(
            engine_state_changed::callback_type callback) = 0;

    virtual waveguide_node_positions_changed::scoped_connector
    add_scoped_waveguide_node_positions_changed_callback(
            waveguide_node_positions_changed::callback_type callback) = 0;

    virtual waveguide_node_pressures_changed::scoped_connector
    add_scoped_waveguide_node_pressures_changed_callback(
            waveguide_node_pressures_changed::callback_type callback) = 0;

    virtual raytracer_reflections_generated::scoped_connector
    add_scoped_raytracer_reflections_generated_callback(
            raytracer_reflections_generated::callback_type callback) = 0;
};

template <typename WaveguideParams>
class concrete_impl : public engine::impl {
public:
    concrete_impl(const core::compute_context& cc,
                  engine::scene_data scene_data,
                  const glm::vec3& source,
                  const glm::vec3& receiver,
                  const core::environment& environment,
                  const raytracer::simulation_parameters& raytracer,
                  const WaveguideParams& waveguide)
            : compute_context_{cc}
            , scene_data_{std::move(scene_data)}
            , room_volume_{estimate_room_volume(scene_data_)}
            , source_{source}
            , receiver_{receiver}
            , environment_{environment}
            , raytracer_{raytracer}
            , waveguide_{waveguide} {}

    std::unique_ptr<intermediate> run(
            const std::atomic_bool& keep_going) const override {
        //  RAYTRACER  /////////////////////////////////////////////////////////

        const auto rays_to_visualise = std::min(1000ul, raytracer_.rays);

        engine_state_changed_(state::starting_raytracer, 1.0);

        auto raytracer_output = raytracer::canonical(
                compute_context_,
                scene_data_,
                source_,
                receiver_,
                environment_,
                raytracer_,
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
                compute_context_,
                scene_data_,
                source_,
                receiver_,
                environment_,
                waveguide_,
                max_stochastic_time,
                keep_going,
                [&](auto step, auto steps) {
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
                receiver_,
                room_volume_,
                environment_);
    }

    engine::engine_state_changed::scoped_connector
    add_scoped_engine_state_changed_callback(
            engine::engine_state_changed::callback_type callback) override {
        return engine_state_changed_.add_scoped(std::move(callback));
    }

    engine::waveguide_node_positions_changed::scoped_connector
    add_scoped_waveguide_node_positions_changed_callback(
            engine::waveguide_node_positions_changed::callback_type callback)
            override {
        return waveguide_node_positions_changed_.add_scoped(
                std::move(callback));
    }

    engine::waveguide_node_pressures_changed::scoped_connector
    add_scoped_waveguide_node_pressures_changed_callback(
            engine::waveguide_node_pressures_changed::callback_type callback)
            override {
        return waveguide_node_pressures_changed_.add_scoped(
                std::move(callback));
    }

    engine::raytracer_reflections_generated::scoped_connector
    add_scoped_raytracer_reflections_generated_callback(
            engine::raytracer_reflections_generated::callback_type callback)
            override {
        return raytracer_reflections_generated_.add_scoped(std::move(callback));
    }

private:
    core::compute_context compute_context_;
    engine::scene_data scene_data_;
    double room_volume_;
    glm::vec3 source_;
    glm::vec3 receiver_;
    core::environment environment_;
    raytracer::simulation_parameters raytracer_;
    WaveguideParams waveguide_;

    engine::engine_state_changed engine_state_changed_;
    engine::waveguide_node_positions_changed waveguide_node_positions_changed_;
    engine::waveguide_node_pressures_changed waveguide_node_pressures_changed_;
    engine::raytracer_reflections_generated raytracer_reflections_generated_;
};

////////////////////////////////////////////////////////////////////////////////

engine::engine(const core::compute_context& compute_context,
               scene_data scene_data,
               const glm::vec3& source,
               const glm::vec3& receiver,
               const core::environment& environment,
               const raytracer::simulation_parameters& raytracer,
               const waveguide::single_band_parameters& waveguide)
        : pimpl_{std::make_unique<
                  concrete_impl<waveguide::single_band_parameters>>(
                  compute_context,
                  std::move(scene_data),
                  source,
                  receiver,
                  environment,
                  raytracer,
                  waveguide)} {}

engine::engine(
        const core::compute_context& compute_context,
        scene_data scene_data,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const core::environment& environment,
        const raytracer::simulation_parameters& raytracer,
        const waveguide::multiple_band_constant_spacing_parameters& waveguide)
        : pimpl_{std::make_unique<concrete_impl<
                  waveguide::multiple_band_constant_spacing_parameters>>(
                  compute_context,
                  std::move(scene_data),
                  source,
                  receiver,
                  environment,
                  raytracer,
                  waveguide)} {}

engine::engine(engine&& rhs) noexcept = default;

engine& engine::operator=(engine&& rhs) noexcept = default;

engine::~engine() noexcept = default;

std::unique_ptr<intermediate> engine::run(
        const std::atomic_bool& keep_going) const {
    return pimpl_->run(keep_going);
}

engine::engine_state_changed::scoped_connector
engine::add_scoped_engine_state_changed_callback(
        engine_state_changed::callback_type callback) {
    return pimpl_->add_scoped_engine_state_changed_callback(
            std::move(callback));
}

engine::waveguide_node_positions_changed::scoped_connector
engine::add_scoped_waveguide_node_positions_changed_callback(
        waveguide_node_positions_changed::callback_type callback) {
    return pimpl_->add_scoped_waveguide_node_positions_changed_callback(
            std::move(callback));
}

engine::waveguide_node_pressures_changed::scoped_connector
engine::add_scoped_waveguide_node_pressures_changed_callback(
        waveguide_node_pressures_changed::callback_type callback) {
    return pimpl_->add_scoped_waveguide_node_pressures_changed_callback(
            std::move(callback));
}

engine::raytracer_reflections_generated::scoped_connector
engine::add_scoped_raytracer_reflections_generated_callback(
        raytracer_reflections_generated::callback_type callback) {
    return pimpl_->add_scoped_raytracer_reflections_generated_callback(
            std::move(callback));
}

void engine::swap(engine& rhs) noexcept {
    using std::swap;
    swap(pimpl_, rhs.pimpl_);
}

void swap(engine& a, engine& b) noexcept { a.swap(b); }

}  // namespace combined
}  // namespace wayverb
