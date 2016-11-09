#pragma once

#include "waveguide/bandpass_band.h"

#include "glm/fwd.hpp"

#include <experimental/optional>
#include <functional>

//  forward declarations  //////////////////////////////////////////////////////

namespace cl {
class Buffer;
class CommandQueue;
}  // namespace cl

namespace wayverb {

namespace waveguide {
struct voxels_and_mesh;
struct single_band_parameters;
struct multiple_band_constant_spacing_parameters;
}  // namespace waveguide

namespace core {
class compute_context;
struct environment;
}  // namespace core

namespace combined {

//  interesting stuff  /////////////////////////////////////////////////////////

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

    virtual std::experimental::optional<
            util::aligned::vector<waveguide::bandpass_band>>
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
                           size_t steps)> pressure_callback) = 0;
};

std::unique_ptr<waveguide_base> make_waveguide_ptr(
        const waveguide::single_band_parameters& t);
std::unique_ptr<waveguide_base> make_waveguide_ptr(
        const waveguide::multiple_band_constant_spacing_parameters& t);

}  // namespace combined
}  // namespace wayverb
