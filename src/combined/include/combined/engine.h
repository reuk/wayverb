#pragma once

#include "combined/postprocess.h"

#include "raytracer/canonical.h"

#include "waveguide/canonical.h"

#include "core/attenuator/hrtf.h"
#include "core/attenuator/microphone.h"
#include "core/attenuator/null.h"
#include "core/cl/common.h"
#include "core/environment.h"
#include "core/scene_data.h"

#include "utilities/aligned/vector.h"
#include "utilities/event.h"

#include "glm/glm.hpp"

#include <functional>

namespace wayverb {
namespace combined {

//  state information  /////////////////////////////////////////////////////////

enum class state {
    idle,
    initialising,
    starting_raytracer,
    running_raytracer,
    finishing_raytracer,
    starting_waveguide,
    running_waveguide,
    finishing_waveguide,
    postprocessing,
};

constexpr auto to_string(state s) {
    switch (s) {
        case state::idle: return "idle";
        case state::initialising: return "initialising";
        case state::starting_raytracer: return "starting raytracer";
        case state::running_raytracer: return "running raytracer";
        case state::finishing_raytracer: return "finishing raytracer";
        case state::starting_waveguide: return "starting waveguide";
        case state::running_waveguide: return "running waveguide";
        case state::finishing_waveguide: return "finishing waveguide";
        case state::postprocessing: return "postprocessing";
    }
}

using engine_state_changed = util::event<state, double>;
using waveguide_node_positions_changed =
        util::event<util::aligned::vector<glm::vec3>>;
using waveguide_node_pressures_changed =
        util::event<util::aligned::vector<float>, double>;
using raytracer_reflections_generated = util::event<
        util::aligned::vector<util::aligned::vector<raytracer::reflection>>>;

//  postprocessing  ////////////////////////////////////////////////////////////

class intermediate {
public:
    intermediate() = default;
    intermediate(const intermediate&) = default;
    intermediate(intermediate&&) noexcept = default;
    intermediate& operator=(const intermediate&) = default;
    intermediate& operator=(intermediate&&) noexcept = default;
    virtual ~intermediate() noexcept = default;

    virtual util::aligned::vector<float> postprocess(
            const core::attenuator::null&, double) const = 0;
    virtual util::aligned::vector<float> postprocess(
            const core::attenuator::hrtf&, double) const = 0;
    virtual util::aligned::vector<float> postprocess(
            const core::attenuator::microphone&, double) const = 0;
};

template <typename Histogram>
class intermediate_impl final : public intermediate {
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
            const core::attenuator::null& a,
            double sample_rate) const override {
        return postprocess_impl(a, sample_rate);
    }

    util::aligned::vector<float> postprocess(
            const core::attenuator::hrtf& a,
            double sample_rate) const override {
        return postprocess_impl(a, sample_rate);
    }

    util::aligned::vector<float> postprocess(
            const core::attenuator::microphone& a,
            double sample_rate) const override {
        return postprocess_impl(a, sample_rate);
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

//  engine  ////////////////////////////////////////////////////////////////////

template <typename WaveguideParameters>
class engine final {
public:
    engine(const core::compute_context& compute_context,
           const core::gpu_scene_data& scene_data,
           const glm::vec3& source,
           const glm::vec3& receiver,
           const core::environment& environment,
           const raytracer::simulation_parameters& raytracer,
           const WaveguideParameters& waveguide)
            : compute_context_{compute_context}
            , voxels_and_mesh_{waveguide::compute_voxels_and_mesh(
                      compute_context,
                      scene_data,
                      receiver,
                      waveguide.sample_rate,
                      environment.speed_of_sound)}
            , room_volume_{estimate_room_volume(scene_data)}
            , source_{source}
            , receiver_{receiver}
            , environment_{environment}
            , raytracer_{raytracer}
            , waveguide_{waveguide} {}

    //  Only valid when WaveguideParameters is
    //  waveguide::single_band_parameters
    //  waveguide::multiple_band_constant_spacing_parameters

    std::unique_ptr<intermediate> run(
            const std::atomic_bool& keep_going) const {
        //  RAYTRACER  /////////////////////////////////////////////////////////

        const auto rays_to_visualise = std::min(1000ul, raytracer_.rays);

        engine_state_changed_(state::starting_raytracer, 1.0);

        auto raytracer_output = raytracer::canonical(
                compute_context_,
                voxels_and_mesh_.voxels,
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
                voxels_and_mesh_,
                source_,
                receiver_,
                environment_,
                waveguide_,
                max_stochastic_time,
                keep_going,
                [&](auto& queue, const auto& buffer, auto step, auto steps) {
                    //  If there are node pressure listeners.
                    if (!waveguide_node_pressures_changed_.empty()) {
                        auto pressures =
                                core::read_from_buffer<float>(queue, buffer);
                        const auto time = step / waveguide_.sample_rate;
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
                receiver_,
                room_volume_,
                environment_);
    }

    //  notifications  /////////////////////////////////////////////////////////

    engine_state_changed::scoped_connector
    add_scoped_engine_state_changed_callback(
            engine_state_changed::callback_type callback) {
        return engine_state_changed_.add_scoped(std::move(callback));
    }

    waveguide_node_positions_changed::scoped_connector
    add_scoped_waveguide_node_positions_changed_callback(
            waveguide_node_positions_changed::callback_type callback) {
        return waveguide_node_positions_changed_.add_scoped(
                std::move(callback));
    }

    waveguide_node_pressures_changed::scoped_connector
    add_scoped_waveguide_node_pressures_changed_callback(
            waveguide_node_pressures_changed::callback_type callback) {
        return waveguide_node_pressures_changed_.add_scoped(
                std::move(callback));
    }

    raytracer_reflections_generated::scoped_connector
    add_scoped_raytracer_reflections_generated_callback(
            raytracer_reflections_generated::callback_type callback) {
        return raytracer_reflections_generated_.add_scoped(std::move(callback));
    }

    const waveguide::voxels_and_mesh& get_voxels_and_mesh() const {
        return voxels_and_mesh_;
    }

private:
    core::compute_context compute_context_;
    waveguide::voxels_and_mesh voxels_and_mesh_;
    double room_volume_;
    glm::vec3 source_;
    glm::vec3 receiver_;
    core::environment environment_;
    raytracer::simulation_parameters raytracer_;
    WaveguideParameters waveguide_;

    engine_state_changed engine_state_changed_;
    waveguide_node_positions_changed waveguide_node_positions_changed_;
    waveguide_node_pressures_changed waveguide_node_pressures_changed_;
    raytracer_reflections_generated raytracer_reflections_generated_;
};

template <typename WaveguideParameters>
auto make_engine(const core::compute_context& compute_context,
                 const core::gpu_scene_data& scene_data,
                 const glm::vec3& source,
                 const glm::vec3& receiver,
                 const core::environment& environment,
                 const raytracer::simulation_parameters& raytracer,
                 const WaveguideParameters& waveguide) {
    return engine<WaveguideParameters>{compute_context,
                                       scene_data,
                                       source,
                                       receiver,
                                       environment,
                                       raytracer,
                                       waveguide};
}

}  // namespace combined
}  // namespace wayverb
