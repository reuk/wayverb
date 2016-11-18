#include "engine_message_queue.h"

engine_message_queue::engine_message_queue(wayverb::combined::model::app& model)
        : begun_connection_{model.connect_begun(
                  make_queue_forwarding_call(begun_))}
        , engine_state_changed_connection_{model.connect_engine_state(
                  make_queue_forwarding_call(engine_state_changed_))}
        , node_positions_changed_connection_{model.connect_node_positions(
                  make_queue_forwarding_call(node_positions_changed_))}
        , node_pressures_changed_connection_{model.connect_node_pressures(
                  make_queue_forwarding_call(node_pressures_changed_))}
        , reflections_generated_connection_{model.connect_reflections(
                  make_queue_forwarding_call(reflections_generated_))}
        , encountered_error_connection_{model.connect_error_handler(
                  make_queue_forwarding_call(encountered_error_))}
        , finished_connection_{model.connect_finished(
                  make_queue_forwarding_call(finished_))} {}

engine_message_queue::begun::connection engine_message_queue::connect_begun(
        begun::callback_type t) {
    return begun_.connect(std::move(t));
}

engine_message_queue::engine_state_changed::connection
engine_message_queue::connect_engine_state(
        engine_state_changed::callback_type t) {
    return engine_state_changed_.connect(std::move(t));
}

engine_message_queue::node_positions_changed::connection
engine_message_queue::connect_node_positions(
        node_positions_changed::callback_type t) {
    return node_positions_changed_.connect(std::move(t));
}

engine_message_queue::node_pressures_changed::connection
engine_message_queue::connect_node_pressures(
        node_pressures_changed::callback_type t) {
    return node_pressures_changed_.connect(std::move(t));
}

engine_message_queue::reflections_generated::connection
engine_message_queue::connect_reflections(
        reflections_generated::callback_type t) {
    return reflections_generated_.connect(std::move(t));
}

engine_message_queue::encountered_error::connection
engine_message_queue::connect_error_handler(
        encountered_error::callback_type t) {
    return encountered_error_.connect(std::move(t));
}

engine_message_queue::finished::connection
engine_message_queue::connect_finished(finished::callback_type t) {
    return finished_.connect(std::move(t));
}
