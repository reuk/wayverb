#pragma once

namespace wayverb {

enum class state {
    idle,
    initialising,
    starting_raytracer,
    running_raytracer,
    finishing_raytracer,
    starting_waveguide,
    running_waveguide,
    finishing_waveguide,
    postprocessing,
};

namespace detail {

template <state>
struct state_trait;
template <>
struct state_trait<state::idle> final {
    static constexpr auto description = "idle";
};
template <>
struct state_trait<state::initialising> final {
    static constexpr auto description = "initialising";
};
template <>
struct state_trait<state::starting_raytracer> final {
    static constexpr auto description = "starting raytracer";
};
template <>
struct state_trait<state::running_raytracer> final {
    static constexpr auto description = "running raytracer";
};
template <>
struct state_trait<state::finishing_raytracer> final {
    static constexpr auto description = "finishing raytracer";
};
template <>
struct state_trait<state::starting_waveguide> final {
    static constexpr auto description = "starting waveguide";
};
template <>
struct state_trait<state::running_waveguide> final {
    static constexpr auto description = "running waveguide";
};
template <>
struct state_trait<state::finishing_waveguide> final {
    static constexpr auto description = "finishing waveguide";
};
template <>
struct state_trait<state::postprocessing> final {
    static constexpr auto description = "postprocessing";
};

template <state state>
constexpr auto description_v = state_trait<state>::description;

}  // namespace detail

constexpr auto to_string(state s) {
    switch (s) {
        //  TODO I wonder whether there's a way of removing the repetition here
        case state::idle: return detail::description_v<state::idle>;
        case state::initialising:
            return detail::description_v<state::initialising>;
        case state::starting_raytracer:
            return detail::description_v<state::starting_raytracer>;
        case state::running_raytracer:
            return detail::description_v<state::running_raytracer>;
        case state::finishing_raytracer:
            return detail::description_v<state::finishing_raytracer>;
        case state::starting_waveguide:
            return detail::description_v<state::starting_waveguide>;
        case state::running_waveguide:
            return detail::description_v<state::running_waveguide>;
        case state::finishing_waveguide:
            return detail::description_v<state::finishing_waveguide>;
        case state::postprocessing:
            return detail::description_v<state::postprocessing>;
    }
}

}  // namespace wayverb
