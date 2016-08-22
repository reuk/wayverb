#pragma once

#include "combined/engine_state.h"

#include "raytracer/cl/structs.h"

#include "common/aligned/vector.h"

#include "glm/glm.hpp"

#include <functional>

//  forward declarations  ----------------------------------------------------//

namespace model {
struct ReceiverSettings;
}  // namespace model

struct compute_context;
class copyable_scene_data;

//  engine  ------------------------------------------------------------------//

namespace wayverb {

class intermediate {
public:
    using state_callback = std::function<void(state, double)>;

    intermediate() = default;
    intermediate(const intermediate&) = default;
    intermediate& operator=(const intermediate&) = default;
    intermediate(intermediate&&) noexcept = default;
    intermediate& operator=(intermediate&&) noexcept = default;
    virtual ~intermediate() noexcept = default;

    virtual aligned::vector<aligned::vector<float>> attenuate(
            const compute_context& cc,
            const model::ReceiverSettings& receiver,
            double output_sample_rate,
            const state_callback&) const = 0;
};

class engine final {
public:
    engine(const compute_context& compute_context,
           const copyable_scene_data& scene_data,
           const glm::vec3& source,
           const glm::vec3& receiver,
           double waveguide_sample_rate,
           size_t rays,
           size_t impulses);

    engine(const compute_context& compute_context,
           const copyable_scene_data& scene_data,
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
                                      const state_callback&);

    using raytracer_visual_callback_t =
            std::function<void(const aligned::vector<aligned::vector<impulse>>&,
                               const glm::vec3&,
                               const glm::vec3&)>;
    void register_raytracer_visual_callback(
            raytracer_visual_callback_t callback);
    void unregister_raytracer_visual_callback();

    using waveguide_visual_callback_t =
            std::function<void(const aligned::vector<cl_float>&, double)>;
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
