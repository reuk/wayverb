#pragma once

#include "combined/engine.h"

#include "model/model.h"

#include <string>

/// Runs the wayverb engine on all combinations of sources and receivers in a
/// scene, sending notifications to a listener about the current state of the
/// engine.
class EngineFunctor final {
public:
    class Listener {
    public:
        Listener() = default;
        Listener(const Listener&) = default;
        Listener& operator=(const Listener&) = default;
        Listener(Listener&&) noexcept = default;
        Listener& operator=(Listener&&) noexcept = default;

        virtual void engine_encountered_error(const std::string& str) = 0;
        virtual void engine_state_changed(wayverb::combined::state state,
                                          double progress) = 0;
        virtual void engine_nodes_changed(
                const util::aligned::vector<glm::vec3>& positions) = 0;
        virtual void engine_waveguide_visuals_changed(
                const util::aligned::vector<float>& pressures,
                double current_time) = 0;
        virtual void engine_raytracer_visuals_changed(
                const util::aligned::vector<
                        util::aligned::vector<wayverb::raytracer::impulse<
                                wayverb::core::simulation_bands>>>& impulses,
                const glm::vec3& source,
                const glm::vec3& receiver) = 0;
        virtual void engine_finished() = 0;

    protected:
        ~Listener() noexcept = default;
    };

    EngineFunctor(Listener& listener,
                  std::atomic_bool& keep_going,
                  std::string file_name,
                  model::Persistent persistent,
                  wayverb::combined::engine::scene_data scene_data,
                  bool visualise);

    /// Call this to start the simulation.
    void operator()() const;

private:
    void single_pair(
            Listener& listener,
            const std::string& file_name,
            const model::SingleShot& single_shot,
            const wayverb::combined::engine::scene_data& scene_data,
            bool visualise,
            const wayverb::core::compute_context& compute_context) const;

    /// Receives notifications.
    Listener& listener;

    /// Calling scope may flip this to get the simulation to quit early.
    std::atomic_bool& keep_going;

    /// State.
    std::string file_name;
    model::Persistent persistent;
    wayverb::combined::engine::scene_data scene;
    bool visualise;
};
