#pragma once

#include "combined/full_run.h"

#include "audio_file/audio_file.h"

namespace wayverb {
namespace combined {

struct source_info final {
    std::string name;
    glm::vec3 position;
};

struct capsule_info final {
    std::string name;
    enum class capsule_mode { microphone, hrtf };
    capsule_mode mode;
    core::attenuator::microphone microphone;
    core::attenuator::hrtf hrtf;
};

struct receiver_info final {
    std::string name;
    glm::vec3 position;
    core::orientable orientation;
    util::aligned::vector<capsule_info> capsules;
};

auto get_absolute_capsule_pointing_vectors(const receiver_info& i) {
    //  TODO
    //  implement and use in complete_engine where appropriate
}

struct output_info final {
    std::string output_folder;
    std::string name;
    double sample_rate;
    enum class bit_depth { bd16 = 16, bd24 = 24 };
    bit_depth bit_depth;
};

constexpr auto convert_bit_depth(enum output_info::bit_depth bd) {
    return static_cast<int>(bd);
}

struct waveguide_info final {
    enum class waveguide_mode {single_band, multiple_band};
    waveguide_mode mode;
    waveguide::single_band_parameters single_band;
    waveguide::multiple_band_constant_spacing_parameters multiple_band;
};

struct scene_parameters final {
    util::aligned::vector<source_info> sources;
    util::aligned::vector<receiver_info> receivers;
    core::environment environment;
    raytracer::simulation_parameters raytracer;
    waveguide_info waveguide;
    output_info output;
};

////////////////////////////////////////////////////////////////////////////////

struct channel_info final {
    util::aligned::vector<float> data;
    std::string source_name;
    std::string receiver_name;
    std::string capsule_name;
};

////////////////////////////////////////////////////////////////////////////////

struct max_mag_functor final {
    template <typename T>
    auto operator()(const T& t) const {
        return core::max_mag(t.data);
    }
};

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
             const scene_parameters& scene_parameters);

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
