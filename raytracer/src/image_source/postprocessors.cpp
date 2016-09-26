#include "raytracer/image_source/postprocessors.h"

#include "common/conversions.h"
#include "common/map_to_vector.h"
#include "common/pressure_intensity.h"
#include "common/surfaces.h"

namespace raytracer {
namespace image_source {

intensity_calculator::intensity_calculator(
        const glm::vec3&,
        const glm::vec3& receiver,
        const voxelised_scene_data<cl_float3, surface>& voxelised,
        float,
        bool flip_phase)
        : receiver_{receiver}
        , voxelised_{voxelised}
        , flip_phase_{flip_phase} {}

//----------------------------------------------------------------------------//

//  A simple pressure calculator which accumulates pressure responses in
//  the time domain.
//  Uses equation 9.22 from the kuttruff book, assuming single-sample
//  reflection/convolution kernels.
fast_pressure_calculator::fast_pressure_calculator(
        const glm::vec3& source,
        const glm::vec3& receiver,
        const voxelised_scene_data<cl_float3, surface>& voxelised,
        float acoustic_impedance,
        bool flip_phase)
        : receiver_{receiver}
        , voxelised_{voxelised}
        , flip_phase_{flip_phase}
        , surface_impedances_{map_to_vector(
                  voxelised.get_scene_data().get_surfaces(), [](auto material) {
                      const auto absorption{get_specular_absorption(material)};
                      const auto reflectance{
                              absorption_to_pressure_reflectance(absorption)};
                      return pressure_reflectance_to_average_wall_impedance(
                              reflectance);
                  })} {}

}  // namespace image_source
}  // namespace raytracer
