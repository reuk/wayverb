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

#define PRINT_INT3(var) printf("##var: %v3hld\n", var);
#define PRINT_FLOAT3(var) printf("##var: %2.2v3hlf\n", var);

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

void add_image_improved(float3 mic_position,
                        float3 mic_reflection,
                        float3 source,
                        global Impulse * image_source,
                        global ulong * image_source_index,
                        VolumeType volume,
                        ulong object_index,
                        VolumeType air_coefficient);
void add_image_improved(float3 mic_position,
                        float3 mic_reflection,
                        float3 source,
                        global Impulse * image_source,
                        global ulong * image_source_index,
                        VolumeType volume,
                        ulong object_index,
                        VolumeType air_coefficient) {
    const float3 init_diff = source - mic_reflection;
    const float init_dist = length(init_diff);
    *image_source = (Impulse){
        -volume * attenuation_for_distance(init_dist, air_coefficient),
        mic_position + init_diff,
        SECONDS_PER_METER * init_dist};
    *image_source_index = object_index;
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

    //  get info about this thread
    global RayInfo * info = ray_info + thread;

    if (!info->keep_going) {
        return;
    }

    Ray ray = info->ray;

    float3 voxel_dimensions = (global_aabb.c1 - global_aabb.c0) / side;
    Intersection closest = voxel_traversal(voxel_index,
                                           ray,
                                           global_aabb,
                                           voxel_dimensions,
                                           side,
                                           triangles,
                                           vertices);

    if (!closest.intersects) {
        info->keep_going = false;
        return;
    }

    const global Triangle * triangle = triangles + closest.primitive;

    VolumeType new_vol = -info->volume * surfaces[triangle->surface].specular;

    if (iteration < num_image_source) {
        TriangleVerts current = {vertices[triangle->v0],
                                 vertices[triangle->v1],
                                 vertices[triangle->v2]};

        global TriangleVerts * prev_primitives_base =
                 prev_primitives + (thread * num_image_source);

        for (uint k = 0; k != iteration; ++k) {
            current = mirror_verts(current, prev_primitives_base[k]);
        }

        prev_primitives_base[iteration] = current;
        info->mic_reflection = mirror_point(info->mic_reflection, current);
        const float3 DIR = get_direction(source, info->mic_reflection);

        Ray to_mic = {source, DIR};
        bool intersects = true;
        float3 prev_intersection = source;
        for (ulong k = 0; k != iteration + 1 && intersects; ++k) {
            TriangleVerts to_test = prev_primitives_base[k];

            float TO_INTERSECTION = triangle_vert_intersection(to_test, to_mic);
            if (TO_INTERSECTION <= EPSILON) {
                intersects = false;
                break;
            }

            float3 intersection_point = source + DIR * TO_INTERSECTION;
            for (long l = k - 1; l != -1; --l) {
                intersection_point =
                    mirror_point(intersection_point, prev_primitives_base[l]);
            }

            Ray intermediate = {
                prev_intersection,
                get_direction(prev_intersection, intersection_point)};

            Intersection inter = voxel_traversal(voxel_index,
                                                 intermediate,
                                                 global_aabb,
                                                 voxel_dimensions,
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

        if (intersects) {
            intersects = voxel_point_intersection(prev_intersection,
                                                  mic,
                                                  voxel_index,
                                                  global_aabb,
                                                  voxel_dimensions,
                                                  side,
                                                  triangles,
                                                  vertices);
        }

        if (intersects) {
            add_image_improved(mic,
                               info->mic_reflection,
                               source,
                               image_source + thread,
                               image_source_index + thread,
                               -new_vol,
                               closest.primitive + 1,
                               air_coefficient);
        }
    }

    float3 intersection = ray.position + ray.direction * closest.distance;
    float new_dist = info->distance + closest.distance;

    const bool is_intersection = voxel_point_intersection(intersection,
                                                          mic,
                                                          voxel_index,
                                                          global_aabb,
                                                          voxel_dimensions,
                                                          side,
                                                          triangles,
                                                          vertices);

    float dist = is_intersection ? new_dist + length(mic - intersection) : 0;
    float diff = fabs(dot(triangle_normal(triangle, vertices), ray.direction));
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
