#pragma once

//  please only include in .cpp files

#include <string>

namespace cl_sources {
const std::string voxel(R"(

int3 get_starting_index(float3 position,
                        AABB global_aabb,
                        float3 voxel_dimensions);
int3 get_starting_index(float3 position,
                        AABB global_aabb,
                        float3 voxel_dimensions) {
    return convert_int3(floor((position - global_aabb.c0) / voxel_dimensions));
}

uint get_voxel_index(const global uint* voxel_index, int3 i, int side);
uint get_voxel_index(const global uint* voxel_index, int3 i, int side) {
    size_t offset = i.x * side * side + i.y * side + i.z;
    return voxel_index[offset];
}

Intersection voxel_traversal(const global uint* voxel_index,
                             Ray ray,
                             AABB global_aabb,
                             float3 voxel_dimensions,
                             int side,
                             const global Triangle* triangles,
                             const global float3* vertices);
Intersection voxel_traversal(const global uint* voxel_index,
                             Ray ray,
                             AABB global_aabb,
                             float3 voxel_dimensions,
                             int side,
                             const global Triangle* triangles,
                             const global float3* vertices) {
    int3 ind = get_starting_index(ray.position, global_aabb, voxel_dimensions);

    float3 c0 = convert_float3(ind + (int3)(0)) * voxel_dimensions;
    float3 c1 = convert_float3(ind + (int3)(1)) * voxel_dimensions;

    AABB voxel_bounds = {global_aabb.c0 + c0, global_aabb.c0 + c1};

    int3 gt         = signbit(ray.direction);
    int3 s          = select((int3)(1), (int3)(-1), gt);
    int3 just_out   = select((int3)(side), (int3)(-1), gt);
    float3 boundary = select(voxel_bounds.c1, voxel_bounds.c0, gt);

    float3 t_max   = fabs((boundary - ray.position) / ray.direction);
    float3 t_delta = fabs(voxel_dimensions / ray.direction);

    for (;;) {
        int min_i = 0;
        for (int i = 1; i != 3; ++i) {
            if (t_max[i] < t_max[min_i]) {
                min_i = i;
            }
        }

        uint voxel_offset  = get_voxel_index(voxel_index, ind, side);
        uint num_triangles = voxel_index[voxel_offset];

        Intersection ret =
                ray_triangle_group_intersection(ray,
                                                triangles,
                                                voxel_index + voxel_offset + 1,
                                                num_triangles,
                                                vertices);

        if (ret.intersects && EPSILON < ret.distance &&
            ret.distance < t_max[min_i]) {
            return ret;
        }

        ind[min_i] += s[min_i];
        if (ind[min_i] == just_out[min_i])
            return (Intersection){};
        t_max[min_i] += t_delta[min_i];
    }
    return (Intersection){};
}

bool voxel_point_intersection(float3 begin,
                              float3 point,
                              const global uint* voxel_index,
                              AABB global_aabb,
                              float3 voxel_dimensions,
                              int side,
                              const global Triangle* triangles,
                              const global float3* vertices);
bool voxel_point_intersection(float3 begin,
                              float3 point,
                              const global uint* voxel_index,
                              AABB global_aabb,
                              float3 voxel_dimensions,
                              int side,
                              const global Triangle* triangles,
                              const global float3* vertices) {
    const float3 begin_to_point = point - begin;
    const float mag             = length(begin_to_point);
    const float3 direction      = normalize(begin_to_point);

    Ray to_point = {begin, direction};

    Intersection inter = voxel_traversal(voxel_index,
                                         to_point,
                                         global_aabb,
                                         voxel_dimensions,
                                         side,
                                         triangles,
                                         vertices);

    return (!inter.intersects) || inter.distance > mag;
}

)");
}  // namespace cl_sources
