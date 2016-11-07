#pragma once

#include "waveguide/canonical.h"

namespace wayverb {
namespace combined {

class waveguide_base {
public:
    waveguide_base() = default;
    waveguide_base(const waveguide_base&) = default;
    waveguide_base(waveguide_base&&) noexcept = default;
    waveguide_base& operator=(const waveguide_base&) = default;
    waveguide_base& operator=(waveguide_base&&) noexcept = default;
    virtual ~waveguide_base() noexcept = default;

    virtual std::unique_ptr<waveguide_base> clone() const = 0;

    virtual double compute_sampling_frequency() const = 0;

    virtual std::experimental::optional<waveguide::simulation_results> run(
            const core::compute_context& cc,
            waveguide::voxels_and_mesh voxelised,
            const glm::vec3& source,
            const glm::vec3& receiver,
            const core::environment& environment,
            double simulation_time,
            const std::atomic_bool& keep_going,
            std::function<void(cl::CommandQueue& queue,
                               const cl::Buffer& buffer,
                               size_t step,
                               size_t steps)> pressure_callback) = 0;
};

template <typename T>
class concrete_waveguide final : public waveguide_base {
public:
    concrete_waveguide(T t)
            : sim_params_{std::move(t)} {}

    std::unique_ptr<waveguide_base> clone() const override {
        return std::make_unique<concrete_waveguide>(*this);
    }

    double compute_sampling_frequency() const override {
        return waveguide::compute_sampling_frequency(sim_params_);
    }

    std::experimental::optional<waveguide::simulation_results> run(
            const core::compute_context& cc,
            waveguide::voxels_and_mesh voxelised,
            const glm::vec3& source,
            const glm::vec3& receiver,
            const core::environment& environment,
            double simulation_time,
            const std::atomic_bool& keep_going,
            std::function<void(cl::CommandQueue& queue,
                               const cl::Buffer& buffer,
                               size_t step,
                               size_t steps)> pressure_callback) override {
        return waveguide::canonical(cc,
                                    std::move(voxelised),
                                    source,
                                    receiver,
                                    environment,
                                    sim_params_,
                                    simulation_time,
                                    keep_going,
                                    std::move(pressure_callback));
    }

private:
    T sim_params_;
};

template <typename T>
auto make_waveguide_ptr(T t) {
    return std::make_unique<concrete_waveguide<T>>(std::move(t));
}

}  // namespace wayverb
}  // namespace combined
