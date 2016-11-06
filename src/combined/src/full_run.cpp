#include "combined/full_run.h"

namespace wayverb {
namespace combined {

engine_state_changed::scoped_connector
postprocessing_engine::add_scoped_engine_state_changed_callback(
        engine_state_changed::callback_type callback) {
    return engine_state_changed_.add_scoped(std::move(callback));
}

postprocessing_engine::waveguide_node_positions_changed::scoped_connector
postprocessing_engine::add_scoped_waveguide_node_positions_changed_callback(
        waveguide_node_positions_changed::callback_type callback) {
    return waveguide_node_positions_changed_.add_scoped(std::move(callback));
}

postprocessing_engine::waveguide_node_pressures_changed::scoped_connector
postprocessing_engine::add_scoped_waveguide_node_pressures_changed_callback(
        waveguide_node_pressures_changed::callback_type callback) {
    return waveguide_node_pressures_changed_.add_scoped(std::move(callback));
}

postprocessing_engine::raytracer_reflections_generated::scoped_connector
postprocessing_engine::add_scoped_raytracer_reflections_generated_callback(
        raytracer_reflections_generated::callback_type callback) {
    return raytracer_reflections_generated_.add_scoped(std::move(callback));
}

}  // namespace combined
}  // namespace wayverb
