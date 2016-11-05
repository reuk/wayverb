#pragma once

#include "raytracer/cl/structs.h"
#include "raytracer/simulation_parameters.h"

#include "waveguide/simulation_parameters.h"

#include "core/attenuator/hrtf.h"
#include "core/attenuator/microphone.h"
#include "core/attenuator/null.h"
#include "core/cl/common.h"
#include "core/environment.h"
#include "core/scene_data.h"

#include "utilities/aligned/vector.h"
#include "utilities/event.h"

#include "glm/glm.hpp"

#include <functional>

namespace wayverb {
namespace combined {

//  state information  /////////////////////////////////////////////////////////

enum class state {
    idle,
    initialising,
    starting_raytracer,
    running_raytracer,
    finishing_raytracer,
    starting_waveguide,
    running_waveguide,
    finishing_waveguide,
};

constexpr auto to_string(state s) {
    switch (s) {
        case state::idle: return "idle";
        case state::initialising: return "initialising";
        case state::starting_raytracer: return "starting raytracer";
        case state::running_raytracer: return "running raytracer";
        case state::finishing_raytracer: return "finishing raytracer";
        case state::starting_waveguide: return "starting waveguide";
        case state::running_waveguide: return "running waveguide";
        case state::finishing_waveguide: return "finishing waveguide";
    }
}

//  postprocessing  ////////////////////////////////////////////////////////////

class intermediate {
public:
    intermediate() = default;

    intermediate(const intermediate&) = default;
    intermediate(intermediate&&) noexcept = default;

    intermediate& operator=(const intermediate&) = default;
    intermediate& operator=(intermediate&&) noexcept = default;

    virtual ~intermediate() noexcept = default;

    virtual util::aligned::vector<float> postprocess(
            const core::attenuator::hrtf& attenuator,
            double sample_rate) const = 0;

    virtual util::aligned::vector<float> postprocess(
            const core::attenuator::microphone& attenuator,
            double sample_rate) const = 0;

    virtual util::aligned::vector<float> postprocess(
            const core::attenuator::null& attenuator,
            double sample_rate) const = 0;
};

//  engine  ////////////////////////////////////////////////////////////////////

class engine final {
public:
    using scene_data =
            core::generic_scene_data<cl_float3,
                                     core::surface<core::simulation_bands>>;

    //  Note: Passing scene_data by value is deliberate, as we need to own
    //  the scene, and users can std::move if they don't need their own copy.

    engine(const core::compute_context& compute_context,
           scene_data scene_data,
           const glm::vec3& source,
           const glm::vec3& receiver,
           const core::environment& environment,
           const raytracer::simulation_parameters& raytracer,
           const waveguide::single_band_parameters& waveguide);

    engine(const core::compute_context& compute_context,
           scene_data scene_data,
           const glm::vec3& source,
           const glm::vec3& receiver,
           const core::environment& environment,
           const raytracer::simulation_parameters& raytracer,
           const waveguide::multiple_band_constant_spacing_parameters&
                   waveguide);

    engine(const engine& rhs) = delete;
    engine(engine&& rhs) noexcept;

    engine& operator=(const engine& rhs) = delete;
    engine& operator=(engine&& rhs) noexcept;

    ~engine() noexcept;

    std::unique_ptr<intermediate> run(const std::atomic_bool& keep_going) const;

    //  notifications  /////////////////////////////////////////////////////////

    using engine_state_changed = util::event<state, double>;

    engine_state_changed::scoped_connector
            add_scoped_engine_state_changed_callback(
                    engine_state_changed::callback_type);

    using waveguide_node_positions_changed =
            util::event<util::aligned::vector<glm::vec3>>;

    waveguide_node_positions_changed::scoped_connector
            add_scoped_waveguide_node_positions_changed_callback(
                    waveguide_node_positions_changed::callback_type);

    using waveguide_node_pressures_changed =
            util::event<util::aligned::vector<float>>;

    waveguide_node_pressures_changed::scoped_connector
            add_scoped_waveguide_node_pressures_changed_callback(
                    waveguide_node_pressures_changed::callback_type);

    using raytracer_reflections_generated = util::event<util::aligned::vector<
            util::aligned::vector<raytracer::reflection>>>;

    raytracer_reflections_generated::scoped_connector
            add_scoped_raytracer_reflections_generated_callback(
                    raytracer_reflections_generated::callback_type);

    //  utilities  /////////////////////////////////////////////////////////////

    void swap(engine&) noexcept;

    class impl;

private:
    std::unique_ptr<impl> pimpl_;
};

void swap(engine&, engine&) noexcept;

}  // namespace combined
}  // namespace wayverb
