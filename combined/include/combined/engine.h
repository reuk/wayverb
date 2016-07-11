#pragma once

#include "common/boundaries.h"
#include "common/cl_common.h"

#include "raytracer/raytracer.h"
#include "waveguide/rectangular_waveguide.h"

namespace engine {

enum class State {
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

inline constexpr auto to_string(State s) {
    switch (s) {
        case State::idle:
            return "idle";
        case State::initialising:
            return "initialising";
        case State::starting_raytracer:
            return "starting raytracer";
        case State::running_raytracer:
            return "running raytracer";
        case State::finishing_raytracer:
            return "finishing raytracer";
        case State::starting_waveguide:
            return "starting waveguide";
        case State::running_waveguide:
            return "running waveguide";
        case State::finishing_waveguide:
            return "finishing waveguide";
        case State::postprocessing:
            return "postprocessing";
    }
}

/// The Wayverb engine
template <BufferType buffer_type>
class WayverbEngine {
public:
    WayverbEngine(ComputeContext& compute_context,
                  const CopyableSceneData& scene_data,
                  const glm::vec3& source,
                  const glm::vec3& mic,
                  float waveguide_sample_rate,
                  int rays,
                  int impulses,
                  float output_sample_rate);

    bool get_source_position_is_valid() const;
    bool get_mic_position_is_valid() const;

    struct Intermediate {};

    using StateCallback = std::function<void(State, double)>;
    using VisualiserCallback = std::function<void(std::vector<float>)>;

    Intermediate run(std::atomic_bool& keep_going,
                     const StateCallback& callback = StateCallback());

    Intermediate run_visualised(
            std::atomic_bool& keep_going,
            const StateCallback& state_callback = StateCallback(),
            const VisualiserCallback& visualiser_callback =
                    VisualiserCallback());

    std::vector<std::vector<float>> attenuate(
            const Intermediate& i,
            //  other args or whatever
            const StateCallback& callback = StateCallback());

    std::vector<cl_float3> get_node_positions() const;

private:
    void check_source_mic_positions() const;

    template <typename Callback>
    auto run_basic(std::atomic_bool& keep_going,
                   const StateCallback& state_callback,
                   const Callback& waveguide_callback);

    CopyableSceneData scene_data;
    Raytracer raytracer;
    RectangularWaveguide<buffer_type> waveguide;

    glm::vec3 source;
    glm::vec3 mic;
    float waveguide_sample_rate;
    int rays;
    int impulses;
    // float output_sample_rate;

    size_t source_index;
    size_t mic_index;
};

}  // namespace engine
