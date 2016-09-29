#include "raytracer/program.h"

#include "raytracer/cl/brdf.h"
#include "raytracer/cl/speed_of_sound_declaration.h"
#include "raytracer/cl/structs.h"

#include "common/cl/geometry.h"
#include "common/cl/geometry_structs.h"
#include "common/cl/scene_structs.h"
#include "common/cl/voxel.h"
#include "common/cl/voxel_structs.h"

namespace raytracer {

constexpr auto source{R"(
#define PRINT_INT3(VAR) printf("%v3hld\n", (VAR));
#define PRINT_FLOAT3(VAR) printf("%2.2v3hlf\n", (VAR));

#define PRINT_ULONG(VAR) printf("%ld\n", (VAR));
#define PRINT_FLOAT(VAR) printf("%2.2f\n", (VAR));

float3 mirror_point(float3 p, triangle_verts t);
float3 mirror_point(float3 p, triangle_verts t) {
    const float3 n = triangle_verts_normal(t);
    return p - n * dot(n, p - t.v0) * 2;
}

triangle_verts mirror_verts(triangle_verts in, triangle_verts t);
triangle_verts mirror_verts(triangle_verts in, triangle_verts t) {
    return (triangle_verts){mirror_point(in.v0, t),
                            mirror_point(in.v1, t),
                            mirror_point(in.v2, t)};
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

kernel void reflections(global ray* rays,  //  ray

                        float3 receiver,  //  receiver

                        const global uint* voxel_index,  //  voxel
                        aabb global_aabb,
                        uint side,

                        const global triangle* triangles,  //  scene
                        const global float3* vertices,
                        const global surface* surfaces,

                        const global float* rng,  //  random numbers

                        global reflection* reflections) {  //  output
    //  get thread index
    const size_t thread = get_global_id(0);

    const bool keep_going = reflections[thread].keep_going;
    const uint previous_triangle = reflections[thread].triangle;

    //  zero out result reflection
    reflections[thread] = (reflection){};

    //  if this thread should stop, then stop
    if (!keep_going) {
        return;
    }

    //  find the ray to intersect
    const ray this_ray = rays[thread];

    //  find the intersection between scene geometry and this ray
    const intersection closest_intersection = voxel_traversal(
            this_ray, voxel_index, global_aabb, side, triangles, vertices, previous_triangle);

    //  didn't find an intersection, should halt this thread
    if (!closest_intersection.inter.t) {
        return;
    }

    //  find where the ray intersects with the scene geometry
    const float3 intersection_pt =
            this_ray.position +
            this_ray.direction * closest_intersection.inter.t;

    //  get the normal at the intersection
    const triangle closest_triangle = triangles[closest_intersection.index];
    float3 tnorm = triangle_normal(closest_triangle, vertices);

    //  calculate the new specular direction from this point
    const float3 specular = reflect(tnorm, this_ray.direction);

    //  make sure the normal faces the right direction
    tnorm *= signbit(dot(tnorm, specular));

    //  see whether the receiver is visible from this point
    const bool is_intersection = voxel_point_intersection(intersection_pt,
                                                          receiver,
                                                          voxel_index,
                                                          global_aabb,
                                                          side,
                                                          triangles,
                                                          vertices,
                                                          closest_intersection.index);

    //  now we can populate the output
    reflections[thread] = (reflection){intersection_pt,
                                       specular,
                                       closest_intersection.index,
                                       true,
                                       is_intersection};

    //  we also need to find the next ray to trace

    //  find the scattering
    //  get random values to influence direction of reflected ray
    const float z = rng[2 * thread + 0];
    const float theta = rng[2 * thread + 1];
    const float3 random_unit_vector = sphere_point(z, theta);
    //  scattering coefficient is the average of the diffuse coefficients
    const surface s = surfaces[closest_triangle.surface];
    const float scatter = mean(s.diffuse_coefficient);
    const float3 scattering = lambert_scattering(
            specular, tnorm, random_unit_vector, scatter);

    //  find the next ray to trace
    rays[thread] = (ray){intersection_pt, scattering};
}

)"};

program::program(const compute_context& cc, double speed_of_sound)
        : program_wrapper(cc,
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

}  // namespace raytracer
