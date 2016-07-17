#include "raytracer/raytracer_program.h"

#include "cl/structs.h"
#include "cl/voxel.h"
#include "cl/geometry.h"

raytracer_program::raytracer_program(const cl::Context& context,
                                     const cl::Device& device)
        : custom_program_base(context,
                              device,
                              std::vector<std::string>{cl_sources::structs,
                                                       cl_sources::geometry,
                                                       cl_sources::voxel,
                                                       source}) {
}

static_assert(SPEED_OF_SOUND != 0, "SPEED_OF_SOUND");

const std::string raytracer_program::source("#define SPEED_OF_SOUND " +
                                            std::to_string(SPEED_OF_SOUND) +
                                            "\n"
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


void image_source_contributions(Intersection closest,
                                VolumeType new_vol,

                                global RayInfo * info,

                                const global uint * voxel_index,
                                AABB global_aabb,
                                int side,

                                const global Triangle * triangles,
                                ulong numtriangles,
                                const global float3 * vertices,
                                const global Surface * surfaces,

                                float3 source,
                                float3 mic,
                                VolumeType air_coefficient,
                                ulong iteration,
                                ulong num_image_source,

                                global Impulse * image_source,
                                global TriangleVerts * history,
                                global ulong * image_source_index);
void image_source_contributions(Intersection closest,
                                VolumeType new_vol,

                                global RayInfo * info,

                                const global uint * voxel_index,
                                AABB global_aabb,
                                int side,

                                const global Triangle * triangles,
                                ulong numtriangles,
                                const global float3 * vertices,
                                const global Surface * surfaces,

                                float3 source,
                                float3 mic,
                                VolumeType air_coefficient,
                                ulong iteration,
                                ulong num_image_source,

                                global Impulse * image_source,
                                global TriangleVerts * history,
                                global ulong * image_source_index) {
    Triangle triangle = triangles[closest.primitive];

    //  if should check for image source contribution
    if (iteration < num_image_source) {
        //  get vertices of the intersected triangle
        TriangleVerts current = {vertices[triangle.v0],
                                 vertices[triangle.v1],
                                 vertices[triangle.v2]};

        reflect_and_add_triangle_to_history(current, history, iteration);

        info->image = mirror_point(info->image, history[iteration]);

        const Ray to_image = {source, get_direction(source, info->image)};
        bool intersects = true;
        float3 prev_intersection = source;
        for (ulong k = 0; k != iteration + 1 && intersects; ++k) {
            TriangleVerts to_test = history[k];
            const float TO_INTERSECTION = triangle_vert_intersection(to_test, to_image);
            if (TO_INTERSECTION <= EPSILON) {
                intersects = false;
            } else {
                float3 intersection_point = source + to_image.direction * TO_INTERSECTION;
                for (long l = k - 1; l != -1; --l) {
                    intersection_point =
                        mirror_point(intersection_point, history[l]);
                }

                Ray intermediate = {
                    prev_intersection,
                    get_direction(prev_intersection, intersection_point)};

                Intersection inter = voxel_traversal(voxel_index,
                                                     intermediate,
                                                     global_aabb,
                                                     side,
                                                     triangles,
                                                     vertices);

                float3 new_intersection_point =
                    (intermediate.position +
                     intermediate.direction * inter.distance);
                intersects =
                    (inter.intersects &&
                     all(new_intersection_point - EPSILON < intersection_point) &&
                     all(intersection_point < new_intersection_point + EPSILON));

                prev_intersection = intersection_point;
            }
        }

        if (intersects) {
            intersects = voxel_point_intersection(prev_intersection,
                                                  mic,
                                                  voxel_index,
                                                  global_aabb,
                                                  side,
                                                  triangles,
                                                  vertices);
        }

        if (intersects) {
            const float3 init_diff = source - info->image;
            const float init_dist = length(init_diff);
            *image_source = (Impulse){
                new_vol * attenuation_for_distance(init_dist, air_coefficient),
                info->image + init_diff,
                SECONDS_PER_METER * init_dist};
            *image_source_index = closest.primitive + 1;
        }
    }
}

kernel void raytrace(global RayInfo * ray_info,         //  ray

                     const global uint * voxel_index,   //  voxel
                     AABB global_aabb,
                     int side,

                     const global Triangle * triangles, //  scene
                     ulong numtriangles,
                     const global float3 * vertices,
                     const global Surface * surfaces,

                     float3 source,                     //  misc
                     float3 mic,
                     VolumeType air_coefficient,
                     ulong iteration,
                     ulong num_image_source,

                     global Impulse * impulses,         //  output
                     global Impulse * image_source,
                     global TriangleVerts * prev_primitives,
                     global ulong * image_source_index) {
    size_t thread = get_global_id(0);
    //  zero stuff
    impulses[thread] = (Impulse){};
    image_source[thread] = (Impulse){};
    image_source_index[thread] = 0;

    //  get info about this thread
    global RayInfo * info = ray_info + thread;

    if (!info->keep_going) {
        return;
    }

    Ray ray = info->ray;

    Intersection closest = voxel_traversal(voxel_index,
                                           ray,
                                           global_aabb,
                                           side,
                                           triangles,
                                           vertices);

    if (!closest.intersects) {
        info->keep_going = false;
        return;
    }

    const global Triangle * triangle = triangles + closest.primitive;
    const VolumeType new_vol = -info->volume * surfaces[triangle->surface].specular;

    image_source_contributions(closest,
                               new_vol,
                               info,
                               voxel_index,
                               global_aabb,
                               side,
                               triangles,
                               numtriangles,
                               vertices,
                               surfaces,
                               source,
                               mic,
                               air_coefficient,
                               iteration,
                               num_image_source,
                               image_source + thread,
                               prev_primitives + (thread * num_image_source),
                               image_source_index + thread);

    const float3 intersection = ray.position + ray.direction * closest.distance;
    const float new_dist = info->distance + closest.distance;

    const bool is_intersection = voxel_point_intersection(intersection,
                                                          mic,
                                                          voxel_index,
                                                          global_aabb,
                                                          side,
                                                          triangles,
                                                          vertices);

    const float dist = is_intersection ? new_dist + length(mic - intersection) : 0;
    const float diff = fabs(dot(triangle_normal(triangle, vertices), ray.direction));
    impulses[thread] = (Impulse){
        (is_intersection
             ? (new_vol * attenuation_for_distance(dist, air_coefficient) *
                surfaces[triangle->surface].diffuse * diff)
             : 0),
        intersection,
        SECONDS_PER_METER * dist};

    Ray new_ray = triangle_reflectAt(triangle, vertices, ray, intersection);

    info->ray = new_ray;
    info->volume = new_vol;
    info->distance = new_dist;
}

)");
