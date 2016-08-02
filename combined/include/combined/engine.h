#pragma once

#include "combined/engine_state.h"

#include "common/aligned/vector.h"

#include "glm/glm.hpp"

#include <functional>

//  forward declarations  ----------------------------------------------------//

namespace model {
struct ReceiverSettings;
}  // namespace model

class compute_context;
class CopyableSceneData;

//  engine  ------------------------------------------------------------------//

namespace wayverb {

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

    aligned::vector<glm::vec3> get_node_positions() const;

    void swap(engine&) noexcept;

private:
    class impl;
    std::unique_ptr<impl> pimpl;
};

void swap(engine&, engine&) noexcept;

}  // namespace wayverb
