#pragma once

#include "combined/model/app.h"

#include "UtilityComponents/async_work_queue.h"

namespace detail {
template <typename T>
class queue_forwarding_call final {
public:
    constexpr queue_forwarding_call(T& t, async_work_queue& queue)
            : t_{t}
            , queue_{queue} {}

    template <typename... Ts>
    void operator()(Ts... ts) const {
        this->queue_.push([ t = &t_, ts... ] { (*t)(ts...); });
    }

private:
    T& t_;
    async_work_queue& queue_;
};

template <typename T>
auto make_queue_forwarding_call(T& t, async_work_queue& queue) {
    return queue_forwarding_call<T>{t, queue};
}

}  // namespace detail

////////////////////////////////////////////////////////////////////////////////

/// Will queue up messages from the model (which might be generated from any
/// thread) and distribute them on the JUCE message thread.
class engine_message_queue final {
public:
    engine_message_queue(wayverb::combined::model::app& model);

    using begun = wayverb::combined::model::app::begun;
    begun::connection connect_begun(begun::callback_type t);

    using engine_state_changed = wayverb::combined::engine_state_changed;
    engine_state_changed::connection connect_engine_state(
            engine_state_changed::callback_type t);

    using node_positions_changed =
            wayverb::combined::waveguide_node_positions_changed;
    node_positions_changed::connection connect_node_positions(
            node_positions_changed::callback_type t);

    using node_pressures_changed =
            wayverb::combined::waveguide_node_pressures_changed;
    node_pressures_changed::connection connect_node_pressures(
            node_pressures_changed::callback_type t);

    using reflections_generated =
            wayverb::combined::raytracer_reflections_generated;
    reflections_generated::connection connect_reflections(
            reflections_generated::callback_type t);

    using encountered_error = wayverb::combined::model::app::encountered_error;
    encountered_error::connection connect_error_handler(
            encountered_error::callback_type t);

    using finished = wayverb::combined::model::app::finished;
    finished::connection connect_finished(finished::callback_type t);

private:
    template <typename T>
    auto make_queue_forwarding_call(T& t) {
        return detail::make_queue_forwarding_call(t, queue_);
    }

    begun::scoped_connection begun_connection_;
    engine_state_changed::scoped_connection engine_state_changed_connection_;
    node_positions_changed::scoped_connection
            node_positions_changed_connection_;
    node_pressures_changed::scoped_connection
            node_pressures_changed_connection_;
    reflections_generated::scoped_connection reflections_generated_connection_;
    encountered_error::scoped_connection encountered_error_connection_;
    finished::scoped_connection finished_connection_;

    begun begun_;
    engine_state_changed engine_state_changed_;
    node_positions_changed node_positions_changed_;
    node_pressures_changed node_pressures_changed_;
    reflections_generated reflections_generated_;
    encountered_error encountered_error_;
    finished finished_;

    async_work_queue queue_;
};
