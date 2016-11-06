#pragma once

#include "combined/capsules.h"

#include <experimental/optional>

namespace wayverb {
namespace combined {

template <typename T>
class forwarding_call final {
public:
    constexpr explicit forwarding_call(T& t)
            : t_{t} {}

    template <typename... Ts>
    constexpr auto operator()(Ts&&... ts) const {
        t_(std::forward<Ts>(ts)...);
    }

private:
    T& t_;
};

template <typename T>
constexpr auto make_forwarding_call(T& t) {
    return forwarding_call<T>{t};
}

////////////////////////////////////////////////////////////////////////////////

/// Similar to `engine` but immediately runs the postprocessing step.
class postprocessing_engine final {
public:
    template <typename WaveguideParameters, typename It>
    std::experimental::optional<
            util::aligned::vector<util::aligned::vector<float>>>
    run(const core::compute_context& compute_context,
        const engine::scene_data& scene_data,
        const glm::vec3& source,
        const glm::vec3& receiver,
        It b_capsules,
        It e_capsules,
        const core::environment& environment,
        const raytracer::simulation_parameters& raytracer,
        const WaveguideParameters& waveguide,
        double sample_rate,
        const std::atomic_bool& keep_going) const {
        //  Create engine.
        auto eng = engine{};

        //  Register callbacks.
        const auto engine_state_change_connector =
                eng.add_scoped_engine_state_changed_callback(
                        make_forwarding_call(engine_state_changed_));

        const auto node_position_connector =
                eng.add_scoped_waveguide_node_positions_changed_callback(
                        make_forwarding_call(
                                waveguide_node_positions_changed_));

        const auto node_pressure_connector =
                eng.add_scoped_waveguide_node_pressures_changed_callback(
                        make_forwarding_call(
                                waveguide_node_pressures_changed_));

        const auto raytracer_reflection_connector =
                eng.add_scoped_raytracer_reflections_generated_callback(
                        make_forwarding_call(raytracer_reflections_generated_));

        auto intermediate = eng.run(compute_context,
                                    scene_data,
                                    source,
                                    receiver,
                                    environment,
                                    raytracer,
                                    waveguide,
                                    keep_going);

        if (intermediate == nullptr) {
            return std::experimental::nullopt;
        }

        const auto intermediate_state_change_connector =
                intermediate.add_scoped_engine_state_changed_callback(
                        make_forwarding_call(engine_state_changed_));

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

    engine_state_changed::scoped_connector
            add_scoped_engine_state_changed_callback(
                    engine_state_changed::callback_type);

    using waveguide_node_positions_changed =
            engine::waveguide_node_positions_changed;

    waveguide_node_positions_changed::scoped_connector
            add_scoped_waveguide_node_positions_changed_callback(
                    waveguide_node_positions_changed::callback_type);

    using waveguide_node_pressures_changed =
            engine::waveguide_node_pressures_changed;

    waveguide_node_pressures_changed::scoped_connector
            add_scoped_waveguide_node_pressures_changed_callback(
                    waveguide_node_pressures_changed::callback_type);

    using raytracer_reflections_generated =
            engine::raytracer_reflections_generated;

    raytracer_reflections_generated::scoped_connector
            add_scoped_raytracer_reflections_generated_callback(
                    raytracer_reflections_generated::callback_type);

private:
    engine_state_changed engine_state_changed_;
    waveguide_node_positions_changed waveguide_node_positions_changed_;
    waveguide_node_pressures_changed waveguide_node_pressures_changed_;
    raytracer_reflections_generated raytracer_reflections_generated_;
};

}  // namespace combined
}  // namespace wayverb
