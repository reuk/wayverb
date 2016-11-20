#pragma once

#include "combined/full_run.h"
#include "combined/model/persistent.h"

#include "waveguide/mesh_descriptor.h"

#include <future>

namespace wayverb {
namespace combined {

/// Given a scene, and a collection of sources and receivers,
/// For each source-receiver pair:
///     Simulate the scene.
///     Do microphone post-processing according to the receiver's capsules.
///     Cache the results.
/// Once all outputs have been calculated:
///     Do global normalization.
///     Write files out.
/// If the engine encounters an error:
///     Signal using a callback, and quit.
/// If the user cancels early:
///     Signal using a callback, and quit.

class complete_engine final {
public:
    ~complete_engine() noexcept;

    void run(core::compute_context compute_context,
             core::gpu_scene_data scene_data,
             model::persistent persistent,
             model::output output);

    bool is_running() const;

    void cancel();

    using engine_state_changed = util::event<size_t, size_t, state, double>;
    using waveguide_node_positions_changed =
            util::event<waveguide::mesh_descriptor>;
    using waveguide_node_pressures_changed =
            postprocessing_engine::waveguide_node_pressures_changed;
    using raytracer_reflections_generated =
            postprocessing_engine::raytracer_reflections_generated;
    using encountered_error = util::event<std::string>;
    using begun = util::event<>;
    using finished = util::event<>;

    engine_state_changed::connection connect_engine_state_changed(
            engine_state_changed::callback_type);

    waveguide_node_positions_changed::connection
            connect_waveguide_node_positions_changed(
                    waveguide_node_positions_changed::callback_type);

    waveguide_node_pressures_changed::connection
            connect_waveguide_node_pressures_changed(
                    waveguide_node_pressures_changed::callback_type);

    raytracer_reflections_generated::connection
            connect_raytracer_reflections_generated(
                    raytracer_reflections_generated::callback_type);

    encountered_error::connection connect_encountered_error(
            encountered_error::callback_type);

    begun::connection connect_begun(begun::callback_type);
    finished::connection connect_finished(finished::callback_type);

private:
    void do_run(core::compute_context compute_context,
                core::gpu_scene_data scene_data,
                model::persistent persistent,
                model::output output);

    engine_state_changed engine_state_changed_;
    waveguide_node_positions_changed waveguide_node_positions_changed_;
    waveguide_node_pressures_changed waveguide_node_pressures_changed_;
    raytracer_reflections_generated raytracer_reflections_generated_;
    encountered_error encountered_error_;
    begun begun_;
    finished finished_;

    std::atomic_bool is_running_{false};
    std::atomic_bool keep_going_{true};

    std::future<void> future_;
};

}  // namespace combined
}  // namespace wayverb
