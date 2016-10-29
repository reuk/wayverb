#pragma once

#include "raytracer/cl/structs.h"

#include "common/scene_data.h"

#include "utilities/aligned/vector.h"

#include "glm/glm.hpp"

#include <functional>

//  forward declarations  //////////////////////////////////////////////////////

namespace model {
struct receiver;
}  // namespace model

class compute_context;

namespace wayverb {

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

//  postprocessing  ////////////////////////////////////////////////////////////

class intermediate {
public:
    intermediate() = default;
    intermediate(const intermediate&) = default;
    intermediate& operator=(const intermediate&) = default;
    intermediate(intermediate&&) noexcept = default;
    intermediate& operator=(intermediate&&) noexcept = default;
    virtual ~intermediate() noexcept = default;

    virtual aligned::vector<aligned::vector<float>> attenuate(
            const model::receiver& receiver,
            double output_sample_rate) const = 0;
};

//  engine  ////////////////////////////////////////////////////////////////////

class engine final {
public:
    using scene_data = generic_scene_data<cl_float3, surface<simulation_bands>>;

    engine(const compute_context& compute_context,
           const scene_data& scene_data,
           const glm::vec3& source,
           const glm::vec3& receiver,
           double waveguide_sample_rate,
           size_t rays);

    engine(const engine& rhs) = delete;
    engine& operator=(const engine& rhs) = delete;

    engine(engine&& rhs) noexcept = default;
    engine& operator=(engine&& rhs) noexcept = default;

    ~engine() noexcept;

    using state_callback = std::function<void(state, double)>;
    std::unique_ptr<intermediate> run(const std::atomic_bool& keep_going,
                                      const state_callback&) const;

    using raytracer_visual_callback_t =
            std::function<void(aligned::vector<aligned::vector<reflection>>,
                               const glm::vec3&,
                               const glm::vec3&)>;
    void register_raytracer_visual_callback(
            raytracer_visual_callback_t callback);
    void unregister_raytracer_visual_callback();

    using waveguide_visual_callback_t =
            std::function<void(aligned::vector<cl_float>, double)>;
    void register_waveguide_visual_callback(
            waveguide_visual_callback_t callback);
    void unregister_waveguide_visual_callback();

    aligned::vector<glm::vec3> get_node_positions() const;

    void swap(engine&) noexcept;

private:
    class impl;
    std::unique_ptr<impl> pimpl;
};

void swap(engine&, engine&) noexcept;

}  // namespace wayverb
