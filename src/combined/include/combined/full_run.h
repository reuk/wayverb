#pragma once

#include "combined/capsules.h"

#include <experimental/optional>

namespace wayverb {
namespace combined {

template <typename T>
class forwarding_call final {
public:
    constexpr explicit forwarding_call(T& t)
            : t_{&t} {}

    template <typename... Ts>
    constexpr auto operator()(Ts&&... ts) const {
        (*t_)(std::forward<Ts>(ts)...);
    }

private:
    T* t_;
};

template <typename T>
constexpr auto make_forwarding_call(T& t) {
    return forwarding_call<T>{t};
}

////////////////////////////////////////////////////////////////////////////////

/// Similar to `engine` but immediately runs the postprocessing step.

template <typename WaveguideParameters>
class postprocessing_engine final {
public:
    using engine_type = engine<WaveguideParameters>;

    postprocessing_engine(const core::compute_context& compute_context,
                          const core::gpu_scene_data& scene_data,
                          const glm::vec3& source,
                          const glm::vec3& receiver,
                          const core::environment& environment,
                          const raytracer::simulation_parameters& raytracer,
                          const WaveguideParameters& waveguide)
            : engine_{compute_context,
                      scene_data,
                      source,
                      receiver,
                      environment,
                      raytracer,
                      waveguide}
            , state_connector_{engine_.add_scoped_engine_state_changed_callback(
                      make_forwarding_call(engine_state_changed_))}
            , position_connector_{engine_.add_scoped_waveguide_node_positions_changed_callback(
                      make_forwarding_call(waveguide_node_positions_changed_))}
            , pressure_connector_{engine_.add_scoped_waveguide_node_pressures_changed_callback(
                      make_forwarding_call(waveguide_node_pressures_changed_))}
            , reflection_connector_{
                      engine_.add_scoped_raytracer_reflections_generated_callback(
                              make_forwarding_call(
                                      raytracer_reflections_generated_))} {}

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
        const std::atomic_bool& keep_going) const {
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

    engine_state_changed::scoped_connector
    add_scoped_engine_state_changed_callback(
            engine_state_changed::callback_type callback) {
        return engine_state_changed_.add_scoped(std::move(callback));
    }

    waveguide_node_positions_changed::scoped_connector
    add_scoped_waveguide_node_positions_changed_callback(
            waveguide_node_positions_changed::callback_type callback) {
        return waveguide_node_positions_changed_.add_scoped(
                std::move(callback));
    }

    waveguide_node_pressures_changed::scoped_connector
    add_scoped_waveguide_node_pressures_changed_callback(
            waveguide_node_pressures_changed::callback_type callback) {
        return waveguide_node_pressures_changed_.add_scoped(
                std::move(callback));
    }

    raytracer_reflections_generated::scoped_connector
    add_scoped_raytracer_reflections_generated_callback(
            raytracer_reflections_generated::callback_type callback) {
        return raytracer_reflections_generated_.add_scoped(std::move(callback));
    }

private:
    engine_type engine_;

    engine_state_changed engine_state_changed_;
    waveguide_node_positions_changed waveguide_node_positions_changed_;
    waveguide_node_pressures_changed waveguide_node_pressures_changed_;
    raytracer_reflections_generated raytracer_reflections_generated_;

    engine_state_changed::scoped_connector state_connector_;
    waveguide_node_positions_changed::scoped_connector position_connector_;
    waveguide_node_pressures_changed::scoped_connector pressure_connector_;
    raytracer_reflections_generated::scoped_connector reflection_connector_;
};

template <typename WaveguideParameters>
auto make_postprocessing_engine_ptr(
        const core::compute_context& compute_context,
        const core::gpu_scene_data& scene_data,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const core::environment& environment,
        const raytracer::simulation_parameters& raytracer,
        const WaveguideParameters& waveguide) {
    return std::make_unique<postprocessing_engine<WaveguideParameters>>(
            compute_context,
            scene_data,
            source,
            receiver,
            environment,
            raytracer,
            waveguide);
}

}  // namespace combined
}  // namespace wayverb
