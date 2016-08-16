#include "raytracer/program.h"

#include "raytracer/cl/brdf.h"
#include "raytracer/cl/structs.h"

#include "common/cl/scene_structs.h"
#include "common/cl/geometry.h"
#include "common/cl/voxel.h"
#include "common/config.h"

namespace raytracer {

program::program(const cl::Context& context, const cl::Device& device)
        : program_wrapper(context,
                          device,
                          std::vector<std::string>{
                                  cl_representation_v<volume_type>,
                                  cl_representation_v<surface>,
                                  cl_representation_v<triangle>,
                                  cl_representation_v<triangle_verts>,
                                  cl_representation_v<reflection>,
                                  cl_representation_v<diffuse_path_info>,
                                  cl_representation_v<impulse>,
                                  cl_representation_v<microphone>,
                                  ::cl_sources::geometry,
                                  ::cl_sources::voxel,
                                  ::cl_sources::brdf,
                                  source}) {}

static_assert(speed_of_sound != 0, "SPEED_OF_SOUND");

const std::string program::source("constant const float SPEED_OF_SOUND = " +
                                  std::to_string(speed_of_sound) +
                                  ";\n"
                                  R"(
#define NULL (0)

#define PRINT_INT3(VAR) printf("%v3hld\n", (VAR));
#define PRINT_FLOAT3(VAR) printf("%2.2v3hlf\n", (VAR));

#define PRINT_ULONG(VAR) printf("%ld\n", (VAR));
#define PRINT_FLOAT(VAR) printf("%2.2f\n", (VAR));

constant const float SECONDS_PER_METER = 1.0f / SPEED_OF_SOUND;

volume_type air_attenuation_for_distance(float distance,
                                        volume_type air_coefficient);
volume_type air_attenuation_for_distance(float distance,
                                        volume_type air_coefficient) {
    return pow(M_E, distance * air_coefficient);
}

float power_attenuation_for_distance(float distance);
float power_attenuation_for_distance(float distance) {
#if 1
    return 1 / (4 * M_PI * distance * distance);
#else
    return 1;
#endif
}

volume_type attenuation_for_distance(float distance, volume_type air_coefficient);
volume_type attenuation_for_distance(float distance,
                                    volume_type air_coefficient) {
    return (air_attenuation_for_distance(distance, air_coefficient) *
            power_attenuation_for_distance(distance));
}

float3 mirror_point(float3 p, triangle_verts t);
float3 mirror_point(float3 p, triangle_verts t) {
    float3 n = triangle_verts_normal(t);
    return p - n * dot(n, p - t.v0) * 2;
}

triangle_verts mirror_verts(triangle_verts in, triangle_verts t);
triangle_verts mirror_verts(triangle_verts in, triangle_verts t) {
    return (triangle_verts){
        mirror_point(in.v0, t), mirror_point(in.v1, t), mirror_point(in.v2, t)};
}

void reflect_and_add_triangle_to_history(triangle_verts current,
                                         global triangle_verts* history,
                                         size_t iteration);
void reflect_and_add_triangle_to_history(triangle_verts current,
                                         global triangle_verts* history,
                                         size_t iteration) {
    //  repeatedly reflect the intersected triangle in
    //  previously-intersected triangles
    for (size_t k = 0; k != iteration; ++k) {
        current = mirror_verts(current, history[k]);
    }

    //  add the reflected triangle to the prev_primitives array
    history[iteration] = current;
}

kernel void reflections(global ray * rays,                 //  ray

                        float3 receiver,                   //  receiver

                        const global uint * voxel_index,   //  voxel
                        aabb global_aabb,
                        ulong side,

                        const global triangle * triangles, //  scene
                        const global float3 * vertices,
                        const global surface * surfaces,

                        const global float * rng,          //  random numbers

                        global reflection * reflections) {  //  output
    //  get thread index
    const size_t thread = get_global_id(0);

    const bool keep_going = reflections[thread].keep_going;

    //  zero out result reflection
    reflections[thread] = (reflection) {};

    //  if this thread should stop, then stop
    if (! keep_going) {
        return;
    }

    //  find the ray to intersect
    const ray this_ray = rays[thread];

    //  find the intersection between scene geometry and this ray
    const intersection closest = voxel_traversal(
            this_ray, voxel_index, global_aabb, side, triangles, vertices);

    //  didn't find an intersection, should halt this thread
    if (! closest.inter.t) {
        return;
    }

    //  find where the ray intersects with the scene geometry
    const float3 intersection =
            this_ray.position + this_ray.direction * closest.inter.t;

    //  get the normal at the intersection
    float3 tnorm = triangle_normal(triangles[closest.index], vertices);

    //  calculate the new specular direction from this point
    const float3 specular = reflect(tnorm, this_ray.direction);

    //  make sure the normal faces the right direction
    tnorm *= signbit(dot(tnorm, specular));

    //  see whether the receiver is visible from this point
    const bool is_intersection = voxel_point_intersection(intersection,
                                                          receiver,
                                                          voxel_index,
                                                          global_aabb,
                                                          side,
                                                          triangles,
                                                          vertices);

    //  now we can populate the output
    reflections[thread] = (reflection) {intersection,
                                       specular,
                                       closest.index,
                                       true,
                                       is_intersection};

    //  we also need to find the next ray to trace

    //  find the scattering
    //  get random values to influence direction of reflected ray
    const float z           = rng[2 * thread + 0];
    const float theta       = rng[2 * thread + 1];
    //  scattering coefficient is the average of the diffuse coefficients
    const surface s         = surfaces[triangles[closest.index].surface];
    const float scatter     = mean(s.diffuse);
    const float3 scattering = lambert_scattering(
            specular, tnorm, sphere_point(z, theta), scatter);

    //  find the next ray to trace
    rays[thread] = (ray){intersection, scattering};
}

kernel void diffuse(const global reflection* reflections,  //  input
                    float3 receiver,
                    volume_type air_coefficient,

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
    //  TODO change specular output depending on angle
    //  TODO remove diffuse energy from ongoing specular energy
    const surface s = surfaces[triangles[reflections[thread].triangle].surface];
    const volume_type new_volume = -diffuse_path[thread].volume * s.specular;

    //  find the new total distance
    const float new_distance = diffuse_path[thread].distance +
                               distance(diffuse_path[thread].position,
                                        reflections[thread].position);

    //  set accumulator
    diffuse_path[thread] = (diffuse_path_info){
            new_volume, reflections[thread].position, new_distance};

    //  compute output

    const float total_distance = new_distance + distance(reflections[thread].position, receiver);

    //  find output volume
    volume_type output_volume = (volume_type)(0, 0, 0, 0, 0, 0, 0, 0);
    if (reflections[thread].receiver_visible) {
        const float3 to_receiver =
                normalize(receiver - reflections[thread].position);
        const volume_type diffuse_brdf = brdf_mags_for_outgoing(
                reflections[thread].direction, to_receiver, s.diffuse);
        output_volume = new_volume * diffuse_brdf * attenuation_for_distance(total_distance, air_coefficient);
    }

    //  find output time
    const float output_time = total_distance / SPEED_OF_SOUND;

    //  set output
    diffuse_output[thread] =
            (impulse){output_volume, reflections[thread].position, output_time};
}
)");

}  // namespace raytracer
