#include "combined/waveguide_base.h"

#include "waveguide/canonical.h"

namespace wayverb {
namespace combined {

template <typename T>
class concrete_waveguide final : public waveguide_base {
public:
    concrete_waveguide(const T& t)
            : sim_params_{t} {}

    std::unique_ptr<waveguide_base> clone() const override {
        return std::make_unique<concrete_waveguide>(*this);
    }

    double compute_sampling_frequency() const override {
        return waveguide::compute_sampling_frequency(sim_params_);
    }

    std::experimental::optional<util::aligned::vector<waveguide::bandpass_band>>
    run(const core::compute_context& cc,
        const waveguide::voxels_and_mesh& voxelised,
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

std::unique_ptr<waveguide_base> make_waveguide_ptr(
        const waveguide::single_band_parameters& t) {
    return std::make_unique<
            concrete_waveguide<waveguide::single_band_parameters>>(
            std::move(t));
}

std::unique_ptr<waveguide_base> make_waveguide_ptr(
        const waveguide::multiple_band_constant_spacing_parameters& t) {
    return std::make_unique<concrete_waveguide<
            waveguide::multiple_band_constant_spacing_parameters>>(
            std::move(t));
}

}  // namespace wayverb
}  // namespace combined
