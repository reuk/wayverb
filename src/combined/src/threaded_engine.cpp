#include "combined/threaded_engine.h"

namespace wayverb {
namespace combined {

engine_state_changed::scoped_connector
complete_engine::add_scoped_engine_state_changed_callback(
        engine_state_changed::callback_type callback) {
    return engine_state_changed_.add_scoped(std::move(callback));
}

waveguide_node_positions_changed::scoped_connector
complete_engine::add_scoped_waveguide_node_positions_changed_callback(
        waveguide_node_positions_changed::callback_type callback) {
    return waveguide_node_positions_changed_.add_scoped(std::move(callback));
}

waveguide_node_pressures_changed::scoped_connector
complete_engine::add_scoped_waveguide_node_pressures_changed_callback(
        waveguide_node_pressures_changed::callback_type callback) {
    return waveguide_node_pressures_changed_.add_scoped(std::move(callback));
}

raytracer_reflections_generated::scoped_connector
complete_engine::add_scoped_raytracer_reflections_generated_callback(
        raytracer_reflections_generated::callback_type callback) {
    return raytracer_reflections_generated_.add_scoped(std::move(callback));
}

complete_engine::encountered_error::scoped_connector
complete_engine::add_scoped_encountered_error_callback(
        encountered_error::callback_type callback) {
    return encountered_error_.add_scoped(std::move(callback));
}

}  // namespace combined
}  // namespace wayverb
