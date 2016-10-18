#include "raytracer/diffuse/program.h"
#include "raytracer/cl/brdf.h"
#include "raytracer/cl/structs.h"

#include "common/cl/geometry.h"
#include "common/cl/geometry_structs.h"
#include "common/cl/scene_structs.h"
#include "common/cl/voxel.h"
#include "common/cl/voxel_structs.h"

namespace raytracer {
namespace diffuse {

constexpr auto source{R"(

//  These functions replicate functionality from common/surfaces.h

volume_type absorption_to_energy_reflectance(volume_type t);
volume_type absorption_to_energy_reflectance(volume_type t) { return 1 - t; }

volume_type absorption_to_pressure_reflectance(volume_type t);
volume_type absorption_to_pressure_reflectance(volume_type t) {
    return sqrt(absorption_to_energy_reflectance(t));
}

volume_type pressure_reflectance_to_average_wall_impedance(volume_type t);
volume_type pressure_reflectance_to_average_wall_impedance(volume_type t) {
    return (1 + t) / (1 - t);
}

volume_type average_wall_impedance_to_pressure_reflectance(volume_type t,
                                                           float cos_angle);
volume_type average_wall_impedance_to_pressure_reflectance(volume_type t,
                                                           float cos_angle) {
    const volume_type tmp = t * cos_angle;
    const volume_type ret = (tmp - 1) / (tmp + 1);
    return ret;
}

volume_type scattered(volume_type total_reflected, volume_type scattering);
volume_type scattered(volume_type total_reflected, volume_type scattering) {
    return total_reflected * scattering;
}

volume_type specular(volume_type total_reflected, volume_type scattering);
volume_type specular(volume_type total_reflected, volume_type scattering) {
    return total_reflected * (1 - scattering);
}

kernel void init_diffuse_path_info(global diffuse_path_info* info,
                                   volume_type volume,
                                   float3 position) {
    const size_t thread = get_global_id(0);
    info[thread] = (diffuse_path_info){volume, position, 0};
}

kernel void diffuse(const global reflection* reflections,
                    float3 receiver,
                    float receiver_radius,
                    uint iteration,

                    const global triangle* triangles,
                    const global float3* vertices,
                    const global surface* surfaces,

                    global diffuse_path_info* diffuse_path,

                    global impulse* diffuse_output,
                    global impulse* intersected_output) {
    const size_t thread = get_global_id(0);

    //  zero out output
    diffuse_output[thread] = (impulse){};
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

    const volume_type reflectance =
            absorption_to_energy_reflectance(reflective_surface.absorption);

    const volume_type last_volume = diffuse_path[thread].volume;
    const volume_type outgoing = last_volume * reflectance;

    const volume_type specular_accumulator =
            specular(outgoing, reflective_surface.scattering);

    const float3 last_position = diffuse_path[thread].position;
    const float3 this_position = reflections[thread].position;

    //  find the new distance to this reflection
    const float last_distance = diffuse_path[thread].distance;
    const float this_distance =
            last_distance + distance(last_position, this_position);

    //  set accumulator
    diffuse_path[thread] = (diffuse_path_info){
            specular_accumulator, this_position, this_distance};

    //  compute output
    
    //  specular output
    if (line_segment_sphere_intersection(
                last_position, this_position, receiver, receiver_radius)) {
        const float3 to_receiver = receiver - last_position;
        const float to_receiver_distance = length(to_receiver);
        const float total_distance = last_distance + to_receiver_distance;

        const volume_type output_volume = last_volume;

        intersected_output[thread] =
                (impulse){output_volume, last_position, total_distance};
    }

    //  diffuse output
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

        const volume_type output_volume =
                angle_correction * 2 * cos_angle *
                scattered(outgoing, reflective_surface.scattering);

        //  set output
        diffuse_output[thread] =
                (impulse){output_volume, this_position, total_distance};
    }
}

)"};

program::program(const compute_context& cc)
        : program_wrapper_(cc,
                           std::vector<std::string>{
                                   cl_representation_v<volume_type>,
                                   cl_representation_v<surface>,
                                   cl_representation_v<triangle>,
                                   cl_representation_v<triangle_verts>,
                                   cl_representation_v<reflection>,
                                   cl_representation_v<diffuse_path_info>,
                                   cl_representation_v<impulse<8>>,
                                   cl_representation_v<aabb>,
                                   cl_representation_v<ray>,
                                   cl_representation_v<triangle_inter>,
                                   cl_representation_v<intersection>,
                                   ::cl_sources::geometry,
                                   ::cl_sources::voxel,
                                   ::cl_sources::brdf,
                                   source}) {}

}  // namespace diffuse
}  // namespace raytracer
