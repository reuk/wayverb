#pragma once

#include "combined/full_run.h"
#include "combined/model/scene.h"

#include "audio_file/audio_file.h"

namespace wayverb {
namespace combined {

////////////////////////////////////////////////////////////////////////////////

struct channel_info final {
    util::aligned::vector<float> data;
    std::string source_name;
    std::string receiver_name;
    std::string capsule_name;
};

////////////////////////////////////////////////////////////////////////////////

using waveguide_node_positions_changed =
        util::event<waveguide::mesh_descriptor>;

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
    void run(const core::compute_context& compute_context,
             const core::gpu_scene_data& scene_data,
             const model::scene& scene_model);

    bool is_running() const {
        return is_running_;
    }

    void cancel() {
        keep_going_ = false;
    }

    engine_state_changed::connection add_engine_state_changed_callback(
            engine_state_changed::callback_type);

    waveguide_node_positions_changed::connection
            add_waveguide_node_positions_changed_callback(
                    waveguide_node_positions_changed::callback_type);

    waveguide_node_pressures_changed::connection
            add_waveguide_node_pressures_changed_callback(
                    waveguide_node_pressures_changed::callback_type);

    raytracer_reflections_generated::connection
            add_raytracer_reflections_generated_callback(
                    raytracer_reflections_generated::callback_type);

    using encountered_error = util::event<std::string>;

    encountered_error::connection add_encountered_error_callback(
            encountered_error::callback_type);

    using finished = util::event<>;

    finished::connection add_finished_callback(finished::callback_type);

private:
    engine_state_changed engine_state_changed_;
    waveguide_node_positions_changed waveguide_node_positions_changed_;
    waveguide_node_pressures_changed waveguide_node_pressures_changed_;
    raytracer_reflections_generated raytracer_reflections_generated_;
    encountered_error encountered_error_;
    finished finished_;

    std::atomic_bool is_running_{false};
    std::atomic_bool keep_going_{true};
};

}  // namespace combined
}  // namespace wayverb
