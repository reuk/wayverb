#pragma once

#include "waveguide/buffer_type.h"

#include "raytracer/raytracer.h"
#include "waveguide/waveguide.h"

#include "glm/fwd.hpp"

#include <functional>

//  forward declarations  ----------------------------------------------------//

namespace model {
struct ReceiverSettings;
}  // namespace model

class ComputeContext;
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
        case state::idle:
            return "idle";
        case state::initialising:
            return "initialising";
        case state::starting_raytracer:
            return "starting raytracer";
        case state::running_raytracer:
            return "running raytracer";
        case state::finishing_raytracer:
            return "finishing raytracer";
        case state::starting_waveguide:
            return "starting waveguide";
        case state::running_waveguide:
            return "running waveguide";
        case state::finishing_waveguide:
            return "finishing waveguide";
        case state::postprocessing:
            return "postprocessing";
    }
}

template <BufferType buffer_type>
class engine final {
public:
    engine(const ComputeContext& compute_context,
           const CopyableSceneData& scene_data,
           const glm::vec3& source,
           const glm::vec3& receiver,
           float waveguide_sample_rate,
           int rays,
           int impulses,
           float output_sample_rate);

    ~engine() noexcept;

    bool get_source_position_is_valid() const;
    bool get_receiver_position_is_valid() const;

    /// Stores intermediate state of the engine.
    /// Allows the 'run' step to be run once, but the 'attenuate' step to be
    /// run multiple times on the output.
    struct intermediate {
        raytracer::RaytracerResults raytracer_results;
        std::vector<RunStepResult> waveguide_results;
        double waveguide_sample_rate;
    };

    using state_callback = std::function<void(state, double)>;
    using visualiser_callback = std::function<void(std::vector<float>)>;

    std::unique_ptr<intermediate> run(std::atomic_bool& keep_going,
                                      const state_callback&);

    std::unique_ptr<intermediate> run_visualised(std::atomic_bool& keep_going,
                                                 const state_callback&,
                                                 const visualiser_callback&);

    std::vector<std::vector<float>> attenuate(
            const intermediate& i,
            const model::ReceiverSettings& receiver,
            const state_callback&);

    std::vector<cl_float3> get_node_positions() const;

private:
    class impl;
    std::unique_ptr<impl> pimpl;
};

}  // namespace wayverb
