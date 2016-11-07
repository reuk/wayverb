#include "combined/threaded_engine.h"

namespace wayverb {
namespace combined {

engine_state_changed::connection
complete_engine::add_engine_state_changed_callback(
        engine_state_changed::callback_type callback) {
    return engine_state_changed_.connect(std::move(callback));
}

waveguide_node_positions_changed::connection
complete_engine::add_waveguide_node_positions_changed_callback(
        waveguide_node_positions_changed::callback_type callback) {
    return waveguide_node_positions_changed_.connect(std::move(callback));
}

waveguide_node_pressures_changed::connection
complete_engine::add_waveguide_node_pressures_changed_callback(
        waveguide_node_pressures_changed::callback_type callback) {
    return waveguide_node_pressures_changed_.connect(std::move(callback));
}

raytracer_reflections_generated::connection
complete_engine::add_raytracer_reflections_generated_callback(
        raytracer_reflections_generated::callback_type callback) {
    return raytracer_reflections_generated_.connect(std::move(callback));
}

complete_engine::encountered_error::connection
complete_engine::add_encountered_error_callback(
        encountered_error::callback_type callback) {
    return encountered_error_.connect(std::move(callback));
}

complete_engine::finished::connection
complete_engine::add_finished_callback(
        finished::callback_type callback) {
    return finished_.connect(std::move(callback));
}

}  // namespace combined
}  // namespace wayverb
