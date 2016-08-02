#pragma once

#include "waveguide/rectangular_waveguide.h"

#include "glm/fwd.hpp"

#include <functional>

//  forward declarations  ----------------------------------------------------//

namespace model {
struct ReceiverSettings;
}  // namespace model

class compute_context;
class CopyableSceneData;

//  engine  ------------------------------------------------------------------//

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

template<state> struct state_trait;
template<> struct state_trait<state::idle>                  final { static constexpr auto description = "idle"; };
template<> struct state_trait<state::initialising>          final { static constexpr auto description = "initialising"; };
template<> struct state_trait<state::starting_raytracer>    final { static constexpr auto description = "starting raytracer"; };
template<> struct state_trait<state::running_raytracer>     final { static constexpr auto description = "running raytracer"; };
template<> struct state_trait<state::finishing_raytracer>   final { static constexpr auto description = "finishing raytracer"; };
template<> struct state_trait<state::starting_waveguide>    final { static constexpr auto description = "starting waveguide"; };
template<> struct state_trait<state::running_waveguide>     final { static constexpr auto description = "running waveguide"; };
template<> struct state_trait<state::finishing_waveguide>   final { static constexpr auto description = "finishing waveguide"; };
template<> struct state_trait<state::postprocessing>        final { static constexpr auto description = "postprocessing"; };

template<state state>
constexpr auto description_v = state_trait<state>::description;

} // namespace detail

constexpr auto to_string(state s) {
    switch (s) {
        //  TODO I wonder whether there's a way of removing the repetition here
        case state::idle:                       return detail::description_v<state::idle>;
        case state::initialising:               return detail::description_v<state::initialising>;
        case state::starting_raytracer:         return detail::description_v<state::starting_raytracer>;
        case state::running_raytracer:          return detail::description_v<state::running_raytracer>;
        case state::finishing_raytracer:        return detail::description_v<state::finishing_raytracer>;
        case state::starting_waveguide:         return detail::description_v<state::starting_waveguide>;
        case state::running_waveguide:          return detail::description_v<state::running_waveguide>;
        case state::finishing_waveguide:        return detail::description_v<state::finishing_waveguide>;
        case state::postprocessing:             return detail::description_v<state::postprocessing>;
    }
}

class intermediate {
public:
    using state_callback = std::function<void(state, double)>;

    intermediate()                    = default;
    intermediate(const intermediate&) = default;
    intermediate& operator=(const intermediate&) = default;
    intermediate(intermediate&&) noexcept        = default;
    intermediate& operator=(intermediate&&) noexcept = default;
    virtual ~intermediate() noexcept                 = default;

    virtual aligned::vector<aligned::vector<float>> attenuate(
            const compute_context& cc,
            const model::ReceiverSettings& receiver,
            double output_sample_rate,
            const state_callback&) const = 0;
};

class engine final {
public:
    engine(const compute_context& compute_context,
           const CopyableSceneData& scene_data,
           const glm::vec3& source,
           const glm::vec3& receiver,
           double waveguide_sample_rate,
           size_t rays,
           size_t impulses);

    engine(const compute_context& compute_context,
           const CopyableSceneData& scene_data,
           const glm::vec3& source,
           const glm::vec3& receiver,
           double waveguide_sample_rate,
           size_t rays);

    engine(const engine& rhs) = delete;
    engine& operator=(const engine& rhs) = delete;

    engine(engine&& rhs) noexcept = default;
    engine& operator=(engine&& rhs) noexcept = default;

    ~engine() noexcept;

    using state_callback      = std::function<void(state, double)>;
    using visualiser_callback = std::function<void(aligned::vector<float>)>;

    std::unique_ptr<intermediate> run(std::atomic_bool& keep_going,
                                      const state_callback&);

    std::unique_ptr<intermediate> run_visualised(std::atomic_bool& keep_going,
                                                 const state_callback&,
                                                 const visualiser_callback&);

    aligned::vector<cl_float3> get_node_positions() const;

    void swap(engine&) noexcept;

private:
    class impl;
    std::unique_ptr<impl> pimpl;
};

void swap(engine&, engine&) noexcept;

}  // namespace wayverb
