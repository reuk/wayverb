#pragma once

#include "raytracer/cl/reflection.h"

#include "core/scene_data.h"

#include "utilities/aligned/vector.h"
#include "utilities/event.h"

#include "glm/fwd.hpp"

#include <memory>

namespace wayverb {

//  forward declarations  //////////////////////////////////////////////////////

namespace core {
class compute_context;
struct environment;
namespace attenuator {
class null;
class hrtf;
class microphone;
}  // namespace attenuator
}  // namespace core
namespace raytracer {
struct simulation_parameters;
}  // namespace raytracer
namespace waveguide {
struct voxels_and_mesh;
}  // namespace waveguide
namespace combined {
class waveguide_base;

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
    postprocessing,
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
        case state::postprocessing: return "postprocessing";
    }
}

using engine_state_changed = util::event<state, double>;
using waveguide_node_pressures_changed =
        util::event<util::aligned::vector<float>, double>;
using raytracer_reflections_generated = util::event<
        util::aligned::vector<util::aligned::vector<raytracer::reflection>>>;

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
            const core::attenuator::null&,
            double) const = 0;
    virtual util::aligned::vector<float> postprocess(
            const core::attenuator::hrtf&,
            double) const = 0;
    virtual util::aligned::vector<float> postprocess(
            const core::attenuator::microphone&,
            double) const = 0;
};

//  engine  ////////////////////////////////////////////////////////////////////

class engine final {
public:
    engine(const core::compute_context& compute_context,
           const core::gpu_scene_data& scene_data,
           const glm::vec3& source,
           const glm::vec3& receiver,
           const core::environment& environment,
           const raytracer::simulation_parameters& raytracer,
           std::unique_ptr<waveguide_base> waveguide);

    ~engine() noexcept;

    std::unique_ptr<intermediate> run(const std::atomic_bool& keep_going) const;

    //  notifications  /////////////////////////////////////////////////////////

    engine_state_changed::connection add_engine_state_changed_callback(
            engine_state_changed::callback_type callback);

    waveguide_node_pressures_changed::connection
    add_waveguide_node_pressures_changed_callback(
            waveguide_node_pressures_changed::callback_type callback);

    raytracer_reflections_generated::connection
    add_raytracer_reflections_generated_callback(
            raytracer_reflections_generated::callback_type callback);

    //  cached data  ///////////////////////////////////////////////////////////

    const waveguide::voxels_and_mesh& get_voxels_and_mesh() const;

private:
    class impl;
    std::unique_ptr<impl> pimpl_;
};

}  // namespace combined
}  // namespace wayverb
