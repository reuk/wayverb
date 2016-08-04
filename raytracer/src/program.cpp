#include "raytracer/program.h"

#include "cl/brdf.h"
#include "cl/geometry.h"
#include "cl/structs.h"
#include "cl/voxel.h"

namespace raytracer {

program::program(const cl::Context& context, const cl::Device& device)
        : program_wrapper(context,
                          device,
                          std::vector<std::string>{cl_sources::structs,
                                                   cl_sources::geometry,
                                                   cl_sources::voxel,
                                                   cl_sources::brdf,
                                                   source}) {}

static_assert(speed_of_sound != 0, "SPEED_OF_SOUND");

const std::string program::source("constant float SPEED_OF_SOUND = " +
                                  std::to_string(speed_of_sound) +
                                  ";\n"
                                  R"(

#define NULL (0)

#define PRINT_INT3(VAR) printf("%v3hld\n", (VAR));
#define PRINT_FLOAT3(VAR) printf("%2.2v3hlf\n", (VAR));

#define PRINT_ULONG(VAR) printf("%ld\n", (VAR));
#define PRINT_FLOAT(VAR) printf("%2.2f\n", (VAR));

constant float SECONDS_PER_METER = 1.0f / SPEED_OF_SOUND;

VolumeType air_attenuation_for_distance(float distance,
                                        VolumeType air_coefficient);
VolumeType air_attenuation_for_distance(float distance,
                                        VolumeType air_coefficient) {
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

VolumeType attenuation_for_distance(float distance, VolumeType air_coefficient);
VolumeType attenuation_for_distance(float distance,
                                    VolumeType air_coefficient) {
    return (air_attenuation_for_distance(distance, air_coefficient) *
            power_attenuation_for_distance(distance));
}

float3 mirror_point(float3 p, TriangleVerts t);
float3 mirror_point(float3 p, TriangleVerts t) {
    float3 n = triangle_verts_normal(t);
    return p - n * dot(n, p - t.v0) * 2;
}

TriangleVerts mirror_verts(TriangleVerts in, TriangleVerts t);
TriangleVerts mirror_verts(TriangleVerts in, TriangleVerts t) {
    return (TriangleVerts){
        mirror_point(in.v0, t), mirror_point(in.v1, t), mirror_point(in.v2, t)};
}

bool point_intersection(float3 begin,
                        float3 point,
                        const global Triangle * triangles,
                        ulong numtriangles,
                        const global float3 * vertices);
bool point_intersection(float3 begin,
                        float3 point,
                        const global Triangle * triangles,
                        ulong numtriangles,
                        const global float3 * vertices) {
    const float3 begin_to_point = point - begin;
    const float mag = length(begin_to_point);
    const float3 direction = normalize(begin_to_point);

    Ray to_point = {begin, direction};

    Intersection inter =
        ray_triangle_intersection(to_point, triangles, numtriangles, vertices);

    return (!inter.intersects) || inter.distance > mag;
}

void reflect_and_add_triangle_to_history(TriangleVerts current,
                                         global TriangleVerts* history,
                                         size_t iteration);
void reflect_and_add_triangle_to_history(TriangleVerts current,
                                         global TriangleVerts* history,
                                         size_t iteration) {
    //  repeatedly reflect the intersected triangle in
    //  previously-intersected triangles
    for (size_t k = 0; k != iteration; ++k) {
        current = mirror_verts(current, history[k]);
    }

    //  add the reflected triangle to the prev_primitives array
    history[iteration] = current;
}

kernel void reflections(global Ray * ray,                  //  ray

                        float3 receiver,                   //  receiver

                        const global uint * voxel_index,   //  voxel
                        AABB global_aabb,
                        ulong side,

                        const global Triangle * triangles, //  scene
                        const global float3 * vertices,
                        const global Surface * surfaces,

                        const global float * rng,          //  random numbers

                        global Reflection * reflection) {  //  output
    //  get thread index
    const size_t thread = get_global_id(0);

    const bool keep_going = reflection[thread].keep_going;

    //  zero out result reflection
    reflection[thread] = (Reflection) {};

    //  if this thread should stop, then stop
    if (! keep_going) {
        return;
    }

    //  find the ray to intersect
    const Ray this_ray = ray[thread];

    //  find the intersection between scene geometry and this ray
    const Intersection closest = voxel_traversal(
            voxel_index, this_ray, global_aabb, side, triangles, vertices);

    //  didn't find an intersection, should halt this thread
    if (! closest.intersects) {
        return;
    }

    //  find where the ray intersects with the scene geometry
    const float3 intersection =
            this_ray.position + this_ray.direction * closest.distance;

    //  get the normal at the intersection
    float3 tnorm = triangle_normal(triangles + closest.primitive, vertices);

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
    reflection[thread] = (Reflection) {intersection,
                                       specular,
                                       closest.primitive,
                                       true,
                                       is_intersection};

    //  we also need to find the next ray to trace

    //  find the scattering
    //  get random values to influence direction of reflected ray
    const float z           = rng[2 * thread + 0];
    const float theta       = rng[2 * thread + 1];
    //  scattering coefficient is the average of the diffuse coefficients
    const Surface surface   = surfaces[triangles[closest.primitive].surface];
    const float scatter     = mean(surface.diffuse);
    const float3 scattering = lambert_scattering(
            specular, tnorm, sphere_point(z, theta), scatter);

    //  find the next ray to trace
    ray[thread] = (Ray){intersection, scattering};
}

kernel void diffuse(const global Reflection* reflections,  //  input
                    float3 receiver,
                    VolumeType air_coefficient,

                    const global Triangle* triangles,  //  scene
                    const global float3* vertices,
                    const global Surface* surfaces,

                    global DiffusePathInfo* diffuse_path_info,  //  accumulator

                    global Impulse* diffuse_output) {  //  output
    const size_t thread = get_global_id(0);

    //  zero out output
    diffuse_output[thread] = (Impulse){};

    //  if this thread doesn't have anything to do, stop now
    if (!reflections[thread].keep_going) {
        return;
    }

    //  find the new volume
    //  TODO change specular output depending on angle
    //  TODO remove diffuse energy from ongoing specular energy
    const Surface surface =
            surfaces[triangles[reflections[thread].triangle].surface];
    const VolumeType new_volume =
            -diffuse_path_info[thread].volume * surface.specular;

    //  find the new total distance
    const float new_distance = diffuse_path_info[thread].distance +
                               distance(diffuse_path_info[thread].position,
                                        reflections[thread].position);

    //  set accumulator
    diffuse_path_info[thread] = (DiffusePathInfo){
            new_volume, reflections[thread].position, new_distance};

    //  compute output

    const float total_distance = new_distance + distance(reflections[thread].position, receiver);

    //  find output volume
    VolumeType output_volume = (VolumeType)(0, 0, 0, 0, 0, 0, 0, 0);
    if (reflections[thread].receiver_visible) {
        const float3 to_receiver =
                normalize(receiver - reflections[thread].position);
        const VolumeType diffuse_brdf = brdf_mags_for_outgoing(
                reflections[thread].direction, to_receiver, surface.diffuse);
        output_volume = new_volume * diffuse_brdf * attenuation_for_distance(total_distance, air_coefficient);
    }

    //  find output time
    const float output_time = total_distance / SPEED_OF_SOUND;

    //  set output
    diffuse_output[thread] =
            (Impulse){output_volume, reflections[thread].position, output_time};
}
)");

}  // namespace raytracer
