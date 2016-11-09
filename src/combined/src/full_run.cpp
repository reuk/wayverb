#include "combined/full_run.h"
#include "combined/forwarding_call.h"
#include "combined/waveguide_base.h"

namespace wayverb {
namespace combined {

postprocessing_engine::postprocessing_engine(
        const core::compute_context& compute_context,
        const core::gpu_scene_data& scene_data,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const core::environment& environment,
        const raytracer::simulation_parameters& raytracer,
        std::unique_ptr<waveguide_base> waveguide)
        : engine_{compute_context,
                  scene_data,
                  source,
                  receiver,
                  environment,
                  raytracer,
                  std::move(waveguide)}
        , state_connector_{engine_.add_engine_state_changed_callback(
                  make_forwarding_call(engine_state_changed_))}
        , pressure_connector_{engine_.add_waveguide_node_pressures_changed_callback(
                  make_forwarding_call(waveguide_node_pressures_changed_))}
        , reflection_connector_{
                  engine_.add_raytracer_reflections_generated_callback(
                          make_forwarding_call(
                                  raytracer_reflections_generated_))} {}

engine_state_changed::connection
postprocessing_engine::add_engine_state_changed_callback(
        engine_state_changed::callback_type callback) {
    return engine_state_changed_.connect(std::move(callback));
}

waveguide_node_pressures_changed::connection
postprocessing_engine::add_waveguide_node_pressures_changed_callback(
        waveguide_node_pressures_changed::callback_type callback) {
    return waveguide_node_pressures_changed_.connect(std::move(callback));
}

raytracer_reflections_generated::connection
postprocessing_engine::add_raytracer_reflections_generated_callback(
        raytracer_reflections_generated::callback_type callback) {
    return raytracer_reflections_generated_.connect(std::move(callback));
}

//  get contents

const waveguide::voxels_and_mesh& postprocessing_engine::get_voxels_and_mesh()
        const {
    return engine_.get_voxels_and_mesh();
}

}  // namespace combined
}  // namespace wayverb
