#pragma once

#include "combined/capsule_base.h"
#include "combined/forwarding_call.h"
#include "combined/engine.h"

#include <experimental/optional>

namespace wayverb {
namespace combined {

/// Similar to `engine` but immediately runs the postprocessing step.

class postprocessing_engine final {
public:
    postprocessing_engine(const core::compute_context& compute_context,
                          const core::gpu_scene_data& scene_data,
                          const glm::vec3& source,
                          const glm::vec3& receiver,
                          const core::environment& environment,
                          const raytracer::simulation_parameters& raytracer,
                          std::unique_ptr<waveguide_base> waveguide);

    postprocessing_engine(const postprocessing_engine&) = delete;
    postprocessing_engine(postprocessing_engine&&) noexcept = delete;

    postprocessing_engine& operator=(const postprocessing_engine&) = delete;
    postprocessing_engine& operator=(postprocessing_engine&&) noexcept = delete;

    template <typename It>
    std::experimental::optional<
            util::aligned::vector<util::aligned::vector<float>>>
    run(It b_capsules,
        It e_capsules,
        double sample_rate,
        const std::atomic_bool& keep_going) {
        //  Only add engine listeners if things are listening to this object.

        engine_state_changed::scoped_connection state;
        if (!engine_state_changed_.empty()) {
            state = engine_state_changed::scoped_connection{
                    engine_.add_engine_state_changed_callback(
                            make_forwarding_call(engine_state_changed_))};
        }

        waveguide_node_pressures_changed::scoped_connection pressures;
        if (!waveguide_node_pressures_changed_.empty()) {
            pressures = waveguide_node_pressures_changed::scoped_connection{
                    engine_.add_waveguide_node_pressures_changed_callback(
                            make_forwarding_call(
                                    waveguide_node_pressures_changed_))};
        }

        raytracer_reflections_generated::scoped_connection reflections;
        if (!raytracer_reflections_generated_.empty()) {
            reflections = raytracer_reflections_generated::scoped_connection{
                    engine_.add_raytracer_reflections_generated_callback(
                            make_forwarding_call(
                                    raytracer_reflections_generated_))};
        }

        //  Start running.

        const auto intermediate = engine_.run(keep_going);

        if (intermediate == nullptr) {
            return std::experimental::nullopt;
        }

        engine_state_changed_(state::postprocessing, 1.0);

        util::aligned::vector<util::aligned::vector<float>> channels;
        for (auto it = b_capsules; it != e_capsules && keep_going; ++it) {
            channels.emplace_back(
                    (*it)->postprocess(*intermediate, sample_rate));
        }

        if (!keep_going) {
            return std::experimental::nullopt;
        }

        return channels;
    }

    //  notifications

    engine_state_changed::connection add_engine_state_changed_callback(
            engine_state_changed::callback_type callback);

    waveguide_node_pressures_changed::connection
    add_waveguide_node_pressures_changed_callback(
            waveguide_node_pressures_changed::callback_type callback);

    raytracer_reflections_generated::connection
    add_raytracer_reflections_generated_callback(
            raytracer_reflections_generated::callback_type callback);

    //  get contents

    const waveguide::voxels_and_mesh& get_voxels_and_mesh() const;

private:
    engine engine_;

    engine_state_changed engine_state_changed_;
    waveguide_node_pressures_changed waveguide_node_pressures_changed_;
    raytracer_reflections_generated raytracer_reflections_generated_;
};

}  // namespace combined
}  // namespace wayverb
