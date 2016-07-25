#pragma once

#include "raytracer/attenuator_program.h"
#include "raytracer/cl_structs.h"
#include "raytracer/results.h"

#include "common/cl_include.h"
#include "common/hrtf_utils.h"

namespace raytracer {
namespace attenuator {

/// Class for parallel HRTF attenuation of raytrace results.
class hrtf final {
public:
    hrtf(const cl::Context&, const cl::Device&);

    /// Attenuate some raytrace results.
    /// The outer vector corresponds to separate channels, the inner vector
    /// contains the impulses, each of which has a time and an 8-band volume.
    aligned::vector<AttenuatedImpulse> process(
            const aligned::vector<Impulse>& results,
            const glm::vec3& direction,
            const glm::vec3& up,
            const glm::vec3& position,
            HrtfChannel channel);

    const std::array<std::array<std::array<cl_float8, 180>, 360>, 2>&
    get_hrtf_data() const;

private:
    using kernel_type =
            decltype(std::declval<attenuator_program>().get_hrtf_kernel());

    cl::CommandQueue queue;
    kernel_type kernel;

    cl::Buffer cl_hrtf;
};

/// Class for parallel Speaker attenuation of raytrace results.
class microphone final {
public:
    microphone(const cl::Context&, const cl::Device&);

    /// Attenuate some raytrace results.
    /// The outer vector corresponds to separate channels, the inner vector
    /// contains the impulses, each of which has a time and an 8-band volume.
    aligned::vector<AttenuatedImpulse> process(
            const aligned::vector<Impulse>& results,
            const glm::vec3& pointing,
            float shape,
            const glm::vec3& position);

private:
    using kernel_type = decltype(
            std::declval<attenuator_program>().get_microphone_kernel());

    cl::CommandQueue queue;
    kernel_type kernel;
};

}  // namespace attenuator
}  // namespace raytracer
