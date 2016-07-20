#pragma once

#include "raytracer/raytracer.h"
#include "waveguide/waveguide.h"

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

inline constexpr auto to_string(state s) {
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

private:
    friend void swap(engine&, engine&);

    class impl;
    std::unique_ptr<impl> pimpl;
};

void swap(engine&, engine&);

}  // namespace wayverb
