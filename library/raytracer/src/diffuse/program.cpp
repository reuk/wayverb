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

volume_type scattered_pressure(volume_type total_reflected,
                               volume_type scattering);
volume_type scattered_pressure(volume_type total_reflected,
                               volume_type scattering) {
    return total_reflected * scattering;
}

volume_type specular_pressure(volume_type total_reflected,
                              volume_type scattering);
volume_type specular_pressure(volume_type total_reflected,
                              volume_type scattering) {
    return total_reflected * (1 - scattering);
}

kernel void diffuse(const global reflection* reflections,  //  input
                    float3 receiver,

                    const global triangle* triangles,  //  scene
                    const global float3* vertices,
                    const global surface* surfaces,

                    global diffuse_path_info* diffuse_path,  //  accumulator

                    global impulse* diffuse_output,
                    char flip_phase) {  //  output
    const size_t thread = get_global_id(0);

    //  zero out output
    diffuse_output[thread] = (impulse){};

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
            absorption_to_pressure_reflectance(reflective_surface.absorption);

    const volume_type outgoing_pressure =
            diffuse_path[thread].volume * reflectance;

    const volume_type specular_accumulator =
            specular_pressure(outgoing_pressure, reflective_surface.scattering);

    const float3 reflective_position = reflections[thread].position;

    //  find the new distance to this reflection
    const float new_distance = diffuse_path[thread].distance +
                               distance(diffuse_path[thread].position,
                                        reflective_position);

    //  set accumulator
    diffuse_path[thread] = (diffuse_path_info){
            specular_accumulator, reflective_position, new_distance};

    //  compute output

    const float3 to_receiver = receiver - reflective_position;
    const float to_receiver_distance = length(to_receiver);
    const float total_distance = new_distance + to_receiver_distance;

    //  find output volume
    volume_type output_volume = (volume_type)(0, 0, 0, 0, 0, 0, 0, 0);
    if (reflections[thread].receiver_visible) {
        //  This implements diffusion according to Lambert's cosine law.
        //  i.e. The intensity is proportional to the cosine of the angle
        //  between the surface normal and the outgoing vector.
        const float3 tnorm = triangle_normal(reflective_triangle, vertices);
        const float cos_angle = fabs(dot(tnorm, normalize(to_receiver)));
        output_volume = cos_angle * scattered_pressure(outgoing_pressure,
                                                reflective_surface.scattering);

        if (flip_phase) {
            output_volume *= -1;
        }
    }

    //  set output
    diffuse_output[thread] = (impulse){
            output_volume, reflective_position, total_distance};
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
