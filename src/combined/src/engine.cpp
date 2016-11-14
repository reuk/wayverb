#include "combined/engine.h"
#include "combined/postprocess.h"
#include "combined/waveguide_base.h"

#include "waveguide/mesh.h"

#include "raytracer/canonical.h"

#include "core/cl/common.h"
#include "core/environment.h"
#include "core/reverb_time.h"
#include "core/scene_data.h"

#include "glm/glm.hpp"

namespace wayverb {
namespace combined {

namespace {
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

}  // namespace

class engine::impl final {
public:
    impl(const core::compute_context& compute_context,
         const core::gpu_scene_data& scene_data,
         const glm::vec3& source,
         const glm::vec3& receiver,
         const core::environment& environment,
         const raytracer::simulation_parameters& raytracer,
         std::unique_ptr<waveguide_base> waveguide)
            : compute_context_{compute_context}
            , voxels_and_mesh_{waveguide::compute_voxels_and_mesh(
                      compute_context,
                      scene_data,
                      receiver,
                      waveguide->compute_sampling_frequency(),
                      environment.speed_of_sound)}
            , room_volume_{estimate_volume(voxels_and_mesh_.mesh)}
            , source_{source}
            , receiver_{receiver}
            , environment_{environment}
            , raytracer_{raytracer}
            , waveguide_{std::move(waveguide)} {}

    std::unique_ptr<intermediate> run(
            const std::atomic_bool& keep_going) const {
        //  RAYTRACER  /////////////////////////////////////////////////////////

        const auto rays_to_visualise = std::min(32ul, raytracer_.rays);

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

        raytracer_reflections_generated_(std::move(raytracer_output->visual),
                                         source_);

        //  look for the max time of an impulse
        const auto max_stochastic_time =
                max_time(raytracer_output->aural.stochastic);

        //  WAVEGUIDE  /////////////////////////////////////////////////////////
        engine_state_changed_(state::starting_waveguide, 1.0);

        auto waveguide_output = waveguide_->run(
                compute_context_,
                voxels_and_mesh_,
                source_,
                receiver_,
                environment_,
                max_stochastic_time,
                keep_going,
                [&](auto& queue, const auto& buffer, auto step, auto steps) {
                    //  If there are node pressure listeners.
                    if (!waveguide_node_pressures_changed_.empty()) {
                        auto pressures =
                                core::read_from_buffer<float>(queue, buffer);
                        const auto time =
                                step / waveguide_->compute_sampling_frequency();
                        const auto distance =
                                time * environment_.speed_of_sound;
                        waveguide_node_pressures_changed_(std::move(pressures),
                                                          distance);
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

    engine_state_changed::connection add_engine_state_changed_callback(
            engine_state_changed::callback_type callback) {
        return engine_state_changed_.connect(std::move(callback));
    }

    waveguide_node_pressures_changed::connection
    add_waveguide_node_pressures_changed_callback(
            waveguide_node_pressures_changed::callback_type callback) {
        return waveguide_node_pressures_changed_.connect(std::move(callback));
    }

    raytracer_reflections_generated::connection
    add_raytracer_reflections_generated_callback(
            raytracer_reflections_generated::callback_type callback) {
        return raytracer_reflections_generated_.connect(std::move(callback));
    }

    //  cached data  ///////////////////////////////////////////////////////////

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
    std::unique_ptr<waveguide_base> waveguide_;

    engine_state_changed engine_state_changed_;
    waveguide_node_pressures_changed waveguide_node_pressures_changed_;
    raytracer_reflections_generated raytracer_reflections_generated_;
};

////////////////////////////////////////////////////////////////////////////////

engine::engine(const core::compute_context& compute_context,
               const core::gpu_scene_data& scene_data,
               const glm::vec3& source,
               const glm::vec3& receiver,
               const core::environment& environment,
               const raytracer::simulation_parameters& raytracer,
               std::unique_ptr<waveguide_base> waveguide)
        : pimpl_{std::make_unique<impl>(compute_context,
                                        scene_data,
                                        source,
                                        receiver,
                                        environment,
                                        raytracer,
                                        std::move(waveguide))} {}

engine::~engine() noexcept = default;

std::unique_ptr<intermediate> engine::run(
        const std::atomic_bool& keep_going) const {
    return pimpl_->run(keep_going);
}

engine_state_changed::connection engine::add_engine_state_changed_callback(
        engine_state_changed::callback_type callback) {
    return pimpl_->add_engine_state_changed_callback(std::move(callback));
}

waveguide_node_pressures_changed::connection
engine::add_waveguide_node_pressures_changed_callback(
        waveguide_node_pressures_changed::callback_type callback) {
    return pimpl_->add_waveguide_node_pressures_changed_callback(
            std::move(callback));
}

raytracer_reflections_generated::connection
engine::add_raytracer_reflections_generated_callback(
        raytracer_reflections_generated::callback_type callback) {
    return pimpl_->add_raytracer_reflections_generated_callback(
            std::move(callback));
}

const waveguide::voxels_and_mesh& engine::get_voxels_and_mesh() const {
    return pimpl_->get_voxels_and_mesh();
}

}  // namespace combined
}  // namespace wayverb
