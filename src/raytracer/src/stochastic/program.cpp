#include "raytracer/stochastic/program.h"
#include "raytracer/cl/brdf.h"
#include "raytracer/cl/structs.h"

#include "core/cl/geometry.h"
#include "core/cl/geometry_structs.h"
#include "core/cl/scene_structs.h"
#include "core/cl/voxel.h"
#include "core/cl/voxel_structs.h"

namespace raytracer {
namespace stochastic {

constexpr auto source = R"(

//  These functions replicate functionality from common/surfaces.h

bands_type absorption_to_energy_reflectance(bands_type t);
bands_type absorption_to_energy_reflectance(bands_type t) { return 1 - t; }

bands_type absorption_to_pressure_reflectance(bands_type t);
bands_type absorption_to_pressure_reflectance(bands_type t) {
    return sqrt(absorption_to_energy_reflectance(t));
}

bands_type pressure_reflectance_to_average_wall_impedance(bands_type t);
bands_type pressure_reflectance_to_average_wall_impedance(bands_type t) {
    return (1 + t) / (1 - t);
}

bands_type average_wall_impedance_to_pressure_reflectance(bands_type t,
                                                           float cos_angle);
bands_type average_wall_impedance_to_pressure_reflectance(bands_type t,
                                                           float cos_angle) {
    const bands_type tmp = t * cos_angle;
    const bands_type ret = (tmp - 1) / (tmp + 1);
    return ret;
}

bands_type scattered(bands_type total_reflected, bands_type scattering);
bands_type scattered(bands_type total_reflected, bands_type scattering) {
    return total_reflected * scattering;
}

bands_type specular(bands_type total_reflected, bands_type scattering);
bands_type specular(bands_type total_reflected, bands_type scattering) {
    return total_reflected * (1 - scattering);
}

kernel void init_stochastic_path_info(global stochastic_path_info* info,
                                   bands_type volume,
                                   float3 position) {
    const size_t thread = get_global_id(0);
    info[thread] = (stochastic_path_info){volume, position, 0};
}

kernel void stochastic(const global reflection* reflections,
                    float3 receiver,
                    float receiver_radius,

                    const global triangle* triangles,
                    const global float3* vertices,
                    const global surface* surfaces,

                    global stochastic_path_info* stochastic_path,

                    global impulse* stochastic_output,
                    global impulse* intersected_output) {
    const size_t thread = get_global_id(0);

    //  zero out output
    stochastic_output[thread] = (impulse){};
    intersected_output[thread] = (impulse){};

    //  if this thread doesn't have anything to do, stop now
    if (!reflections[thread].keep_going) {
        return;
    }

    //  find the new volume
    const size_t triangle_index = reflections[thread].triangle;
    const triangle reflective_triangle = triangles[triangle_index];
    const size_t surface_index = reflective_triangle.surface;
    const surface reflective_surface = surfaces[surface_index];

    const bands_type reflectance =
            absorption_to_energy_reflectance(reflective_surface.absorption);

    const bands_type last_volume = stochastic_path[thread].volume;
    const bands_type outgoing = last_volume * reflectance;

    const float3 last_position = stochastic_path[thread].position;
    const float3 this_position = reflections[thread].position;

    //  find the new distance to this reflection
    const float last_distance = stochastic_path[thread].distance;
    const float this_distance =
            last_distance + distance(last_position, this_position);

    //  set accumulator
    stochastic_path[thread] = (stochastic_path_info){
            outgoing, this_position, this_distance};

    //  compute output
    
    //  specular output
    if (line_segment_sphere_intersection(
                last_position, this_position, receiver, receiver_radius)) {
        const float3 to_receiver = receiver - last_position;
        const float to_receiver_distance = length(to_receiver);
        const float total_distance = last_distance + to_receiver_distance;

        const bands_type output_volume = last_volume;

        intersected_output[thread] =
                (impulse){output_volume, last_position, total_distance};
    }

    //  stochastic output
    if (reflections[thread].receiver_visible) {
        const float3 to_receiver = receiver - this_position;
        const float to_receiver_distance = length(to_receiver);
        const float total_distance = this_distance + to_receiver_distance;

        //  This implements diffusion according to Lambert's cosine law.
        //  i.e. The intensity is proportional to the cosine of the angle
        //  between the surface normal and the outgoing vector.
        const float3 tnorm = triangle_normal(reflective_triangle, vertices);
        const float cos_angle = fabs(dot(tnorm, normalize(to_receiver)));

        //  Scattered energy equation:
        //  schroder2011 5.20
        //  detected scattered energy =
        //      incident energy * (1 - a) * s * (1 - cos(y/2)) * 2 * cos(theta)
        //  where
        //      y = opening angle
        //      theta = angle between receiver centre and surface normal

        const float sin_y =
                receiver_radius / max(receiver_radius, to_receiver_distance);
        const float angle_correction = 1 - sqrt(1 - sin_y * sin_y);

        const bands_type output_volume =
                angle_correction * 2 * cos_angle *
                scattered(outgoing, reflective_surface.scattering);

        //  set output
        stochastic_output[thread] =
                (impulse){output_volume, this_position, total_distance};
    }
}

)";

program::program(const core::compute_context& cc)
        : program_wrapper_(
                  cc,
                  std::vector<std::string>{
                          core::cl_representation_v<core::bands_type>,
                          core::cl_representation_v<
                                  core::surface<core::simulation_bands>>,
                          core::cl_representation_v<core::triangle>,
                          core::cl_representation_v<core::triangle_verts>,
                          core::cl_representation_v<core::aabb>,
                          core::cl_representation_v<core::ray>,
                          core::cl_representation_v<core::triangle_inter>,
                          core::cl_representation_v<core::intersection>,
                          core::cl_representation_v<
                                  impulse<core::simulation_bands>>,
                          core::cl_representation_v<reflection>,
                          core::cl_representation_v<stochastic_path_info>,
                          core::cl_sources::geometry,
                          core::cl_sources::voxel,
                          ::cl_sources::brdf,
                          source}) {}

}  // namespace stochastic
}  // namespace raytracer
