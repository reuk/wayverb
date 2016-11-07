#include "combined/engine.h"

namespace wayverb {
namespace combined {

engine::engine(const core::compute_context& compute_context,
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
        , room_volume_{estimate_room_volume(scene_data)}
        , source_{source}
        , receiver_{receiver}
        , environment_{environment}
        , raytracer_{raytracer}
        , waveguide_{std::move(waveguide)} {}

std::unique_ptr<intermediate> engine::run(
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

engine_state_changed::connection engine::add_engine_state_changed_callback(
        engine_state_changed::callback_type callback) {
    return engine_state_changed_.connect(std::move(callback));
}

waveguide_node_pressures_changed::connection
engine::add_waveguide_node_pressures_changed_callback(
        waveguide_node_pressures_changed::callback_type callback) {
    return waveguide_node_pressures_changed_.connect(std::move(callback));
}

raytracer_reflections_generated::connection
engine::add_raytracer_reflections_generated_callback(
        raytracer_reflections_generated::callback_type callback) {
    return raytracer_reflections_generated_.connect(std::move(callback));
}

//  cached data  ///////////////////////////////////////////////////////////

const waveguide::voxels_and_mesh& engine::get_voxels_and_mesh() const {
    return voxels_and_mesh_;
}
}  // namespace combined
}  // namespace wayverb
