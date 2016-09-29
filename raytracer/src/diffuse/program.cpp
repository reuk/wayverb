#include "raytracer/diffuse/program.h"
#include "raytracer/cl/brdf.h"
#include "raytracer/cl/speed_of_sound_declaration.h"
#include "raytracer/cl/structs.h"

#include "common/cl/geometry.h"
#include "common/cl/geometry_structs.h"
#include "common/cl/scene_structs.h"
#include "common/cl/voxel.h"
#include "common/cl/voxel_structs.h"

namespace raytracer {
namespace diffuse {

constexpr auto source{R"(

float power_attenuation_for_distance(float distance);
float power_attenuation_for_distance(float distance) {
    return 1 / (4 * M_PI * distance * distance);
}

volume_type absorption_to_energy_reflectance(volume_type t);
volume_type absorption_to_energy_reflectance(volume_type t) {
    return 1 - t;
}

kernel void diffuse(const global reflection* reflections,  //  input
                    float3 receiver,

                    const global triangle* triangles,  //  scene
                    const global float3* vertices,
                    const global surface* surfaces,

                    global diffuse_path_info* diffuse_path,  //  accumulator

                    global impulse* diffuse_output) {  //  output
    const size_t thread = get_global_id(0);

    //  zero out output
    diffuse_output[thread] = (impulse){};

    //  if this thread doesn't have anything to do, stop now
    if (!reflections[thread].keep_going) {
        return;
    }

    //  find the new volume
    const size_t triangle_index = reflections[thread].triangle;
    const size_t surface_index = triangles[triangle_index].surface;
    const surface s = surfaces[surface_index];
    const volume_type new_volume = diffuse_path[thread].volume * absorption_to_energy_reflectance(s.specular_absorption);

    //  find the new distance to this reflection
    const float new_distance = diffuse_path[thread].distance +
                               distance(diffuse_path[thread].position,
                                        reflections[thread].position);

    //  set accumulator
    diffuse_path[thread] = (diffuse_path_info){
            new_volume, reflections[thread].position, new_distance};

    //  compute output

    const float3 to_receiver = receiver - reflections[thread].position;
    const float to_receiver_distance = length(to_receiver);
    const float total_distance = new_distance + to_receiver_distance;

    //  find output volume
    volume_type output_volume = (volume_type)(0, 0, 0, 0, 0, 0, 0, 0);
    if (reflections[thread].receiver_visible) {
        const volume_type diffuse_brdf = brdf_mags_for_outgoing(
                reflections[thread].direction, to_receiver, s.diffuse_coefficient);
        output_volume = new_volume * diffuse_brdf * power_attenuation_for_distance(total_distance);
    }

    //  find output time
    const float output_time = total_distance / SPEED_OF_SOUND;

    //  set output
    diffuse_output[thread] =
            (impulse){output_volume, reflections[thread].position, output_time};
}

)"};

program::program(const compute_context& cc, double speed_of_sound)
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
                                   ::cl_sources::speed_of_sound_declaration(
                                           speed_of_sound),
                                   source}) {}

}  // namespace diffuse
}  // namespace raytracer
