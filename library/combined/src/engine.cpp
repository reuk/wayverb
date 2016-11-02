#include "combined/engine.h"
#include "combined/postprocess.h"

#include "raytracer/canonical.h"
#include "raytracer/postprocess.h"
#include "raytracer/reflector.h"

#include "waveguide/canonical.h"

#include "common/attenuator/hrtf.h"
#include "common/attenuator/microphone.h"
#include "common/azimuth_elevation.h"
#include "common/callback_accumulator.h"
#include "common/cl/common.h"
#include "common/conversions.h"
#include "common/dc_blocker.h"
#include "common/filters_common.h"
#include "common/kernel.h"
#include "common/model/receiver.h"
#include "common/pressure_intensity.h"
#include "common/spatial_division/voxelised_scene_data.h"
#include "common/surfaces.h"

#include "utilities/event.h"

#include "hrtf/multiband.h"

#include <cmath>

namespace {

template <typename Histogram>
class intermediate_impl : public wayverb::intermediate {
public:
    intermediate_impl(wayverb::combined_results<Histogram> to_process,
                      const glm::vec3& receiver_position,
                      double room_volume,
                      double acoustic_impedance,
                      double speed_of_sound)
            : to_process_{std::move(to_process)}
            , receiver_position_{receiver_position}
            , room_volume_{room_volume}
            , acoustic_impedance_{acoustic_impedance}
            , speed_of_sound_{speed_of_sound} {}

    util::aligned::vector<float> postprocess(
            const attenuator::hrtf& attenuator,
            double sample_rate) const override {
        return postprocess_impl(attenuator, sample_rate);
    }

    util::aligned::vector<float> postprocess(
            const attenuator::microphone& attenuator,
            double sample_rate) const override {
        return postprocess_impl(attenuator, sample_rate);
    }

    util::aligned::vector<float> postprocess(
            const attenuator::null& attenuator,
            double sample_rate) const override {
        return postprocess_impl(attenuator, sample_rate);
    }

private:
    template <typename Attenuator>
    auto postprocess_impl(const Attenuator& attenuator,
                          double output_sample_rate) const {
        return wayverb::postprocess(to_process_,
                                    attenuator,
                                    receiver_position_,
                                    room_volume_,
                                    acoustic_impedance_,
                                    speed_of_sound_,
                                    output_sample_rate);
    }

    wayverb::combined_results<Histogram> to_process_;
    glm::vec3 receiver_position_;
    double room_volume_;
    double acoustic_impedance_;
    double speed_of_sound_;
};

template <typename Histogram>
auto make_intermediate_impl_ptr(wayverb::combined_results<Histogram> to_process,
                                const glm::vec3& receiver_position,
                                double room_volume,
                                double acoustic_impedance,
                                double speed_of_sound) {
    return std::make_unique<intermediate_impl<Histogram>>(std::move(to_process),
                                                          receiver_position,
                                                          room_volume,
                                                          acoustic_impedance,
                                                          speed_of_sound);
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////

namespace wayverb {

class engine::impl {
public:
    impl() = default;

    impl(const impl&) = default;
    impl(impl&&) noexcept = default;

    impl& operator=(const impl&) = default;
    impl& operator=(impl&&) noexcept = default;

    virtual ~impl() noexcept = default;

    virtual std::unique_ptr<intermediate> run(
            const std::atomic_bool& keep_going,
            const state_callback& callback) const = 0;

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
    concrete_impl(const compute_context& cc,
                  engine::scene_data scene_data,
                  const model::parameters& parameters,
                  const raytracer::simulation_parameters& raytracer,
                  const WaveguideParams& waveguide)
            : compute_context_{cc}
            , scene_data_{std::move(scene_data)}
            , room_volume_{estimate_room_volume(scene_data_)}
            , parameters_{parameters}
            , raytracer_{raytracer}
            , waveguide_{waveguide} {}

    std::unique_ptr<intermediate> run(
            const std::atomic_bool& keep_going,
            const engine::state_callback& callback) const override {
        //  RAYTRACER  /////////////////////////////////////////////////////////

        const auto rays_to_visualise = std::min(1000ul, raytracer_.rays);

        callback(state::starting_raytracer, 1.0);

        auto raytracer_output =
                raytracer::canonical(compute_context_,
                                     scene_data_,
                                     parameters_,
                                     raytracer_,
                                     rays_to_visualise,
                                     keep_going,
                                     [&](auto step, auto total_steps) {
                                         callback(state::running_raytracer,
                                                  step / (total_steps - 1.0));
                                     });

        if (!(keep_going && raytracer_output)) {
            return nullptr;
        }

        callback(state::finishing_raytracer, 1.0);

        raytracer_reflections_generated_(std::move(raytracer_output->visual));

        //  look for the max time of an impulse
        const auto max_stochastic_time =
                max_time(raytracer_output->aural.stochastic);

        //  WAVEGUIDE  /////////////////////////////////////////////////////////
        callback(state::starting_waveguide, 1.0);

        auto waveguide_output = waveguide::canonical(
                compute_context_,
                scene_data_,
                parameters_,
                waveguide_,
                max_stochastic_time,
                keep_going,
                [&](auto step, auto steps) {
                    callback(state::running_waveguide, step / (steps - 1.0));
                });

        if (!(keep_going && waveguide_output)) {
            return nullptr;
        }

        callback(state::finishing_waveguide, 1.0);

        return make_intermediate_impl_ptr(
                make_combined_results(std::move(raytracer_output->aural),
                                      std::move(*waveguide_output)),
                parameters_.receiver,
                room_volume_,
                parameters_.acoustic_impedance,
                parameters_.speed_of_sound);
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
    compute_context compute_context_;
    engine::scene_data scene_data_;
    double room_volume_;
    model::parameters parameters_;
    raytracer::simulation_parameters raytracer_;
    WaveguideParams waveguide_;

    engine::waveguide_node_positions_changed waveguide_node_positions_changed_;
    engine::waveguide_node_pressures_changed waveguide_node_pressures_changed_;
    engine::raytracer_reflections_generated raytracer_reflections_generated_;
};

////////////////////////////////////////////////////////////////////////////////

engine::engine(const compute_context& compute_context,
               scene_data scene_data,
               const model::parameters& parameters,
               const raytracer::simulation_parameters& raytracer,
               const waveguide::single_band_parameters& waveguide)
        : pimpl_{std::make_unique<
                  concrete_impl<waveguide::single_band_parameters>>(
                  compute_context,
                  std::move(scene_data),
                  parameters,
                  raytracer,
                  waveguide)} {}

engine::engine(
        const compute_context& compute_context,
        scene_data scene_data,
        const model::parameters& parameters,
        const raytracer::simulation_parameters& raytracer,
        const waveguide::multiple_band_constant_spacing_parameters& waveguide)
        : pimpl_{std::make_unique<concrete_impl<
                  waveguide::multiple_band_constant_spacing_parameters>>(
                  compute_context,
                  std::move(scene_data),
                  parameters,
                  raytracer,
                  waveguide)} {}

engine::engine(engine&& rhs) noexcept = default;

engine& engine::operator=(engine&& rhs) noexcept = default;

engine::~engine() noexcept = default;

std::unique_ptr<intermediate> engine::run(
        const std::atomic_bool& keep_going,
        const engine::state_callback& callback) const {
    return pimpl_->run(keep_going, callback);
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

}  // namespace wayverb
