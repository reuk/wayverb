#pragma once

#include "raytracer/cl/structs.h"

#include "common/aligned/vector.h"
#include "common/spatial_division/voxelised_scene_data.h"

#include "glm/fwd.hpp"

#include <complex>

/// The image source finder is designed just to ascertain whether or not each
/// 'possible' image-source path is an *actual* image-source path.
/// Once it's worked this out, it delegates to one of these secondary functions
/// to actually compute the pressure or intensity contributed by that particular
/// path.

namespace raytracer {
namespace image_source {

struct reflection_metadata final {
    cl_uint index;  /// The index of the triangle that was intersected.
    float angle;    /// The angle against the triangle normal.
};

/// All image-source postprocessors should be convertible to this type.
using postprocessor = std::function<void(
        const glm::vec3&, const aligned::vector<reflection_metadata>&)>;

//----------------------------------------------------------------------------//

class intensity_calculator final {
public:
    intensity_calculator(const glm::vec3& source,
                         const glm::vec3& receiver,
                         const voxelised_scene_data& voxelised,
                         float speed_of_sound,
                         float acoustic_impedance);

    impulse operator()(
            const glm::vec3& image_source,
            const aligned::vector<reflection_metadata>& intersections) const;

private:
    const glm::vec3& receiver_;
    const voxelised_scene_data& voxelised_;
    float speed_of_sound_;
};

//----------------------------------------------------------------------------//

class fast_pressure_calculator final {
public:
    fast_pressure_calculator(const glm::vec3& source,
                             const glm::vec3& receiver,
                             const voxelised_scene_data& voxelised,
                             float speed_of_sound,
                             float acoustic_impedance);

    impulse operator()(
            const glm::vec3& image_source,
            const aligned::vector<reflection_metadata>& intersections) const;

private:
    const glm::vec3& receiver_;
    const voxelised_scene_data& voxelised_;
    float speed_of_sound_;
    float acoustic_impedance_;

    aligned::vector<volume_type> surface_impedances_;
};

//----------------------------------------------------------------------------//

class comparison_calculator final {
public:
    comparison_calculator(const glm::vec3& source,
                          const glm::vec3& receiver,
                          const voxelised_scene_data& voxelised,
                          float speed_of_sound,
                          float acoustic_impedance);

    impulse operator()(
            const glm::vec3& image_source,
            const aligned::vector<reflection_metadata>& intersections) const;

private:
    intensity_calculator intensity_;
    fast_pressure_calculator fast_pressure_;
};

//----------------------------------------------------------------------------//

#if 0
class dumb_slow_fft_pressure_calculator final {
public:
    dumb_slow_fft_pressure_calculator(const glm::vec3& source,
                                      const glm::vec3& receiver,
                                      const voxelised_scene_data& voxelised,
                                      float speed_of_sound);

    constexpr static auto output_spectrum_size{1 << 9};

    /// Returns a full FFT spectrum for this reflection which should be summed
    /// with the 'output spectrum'.
    std::array<std::complex<float>, output_spectrum_size> operator()(
            const glm::vec3& image_source,
            const aligned::vector<reflection_metadata>& intersections) const;

private:
    const glm::vec3& receiver_;
    const voxelised_scene_data& voxelised_;
    float speed_of_sound_;
};
#endif

}  // namespace image_source
}  // namespace raytracer
