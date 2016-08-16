#pragma once

//  please only include in .cpp files

#include "common/cl/voxel_structs.h"

#include <string>

namespace cl_sources {
const std::string voxel(std::string{} + cl_representation_v<aabb> + R"(
#ifndef VOXEL_HEADER__
#define VOXEL_HEADER__

int3 get_starting_index(float3 position,
                        aabb global_aabb,
                        float3 voxel_dimensions);
int3 get_starting_index(float3 position,
                        aabb global_aabb,
                        float3 voxel_dimensions) {
    return convert_int3(floor((position - global_aabb.c0) / voxel_dimensions));
}

uint get_voxel_index(const global uint* voxel_index, int3 i, ulong side);
uint get_voxel_index(const global uint* voxel_index, int3 i, ulong side) {
    const size_t offset = i.x * side * side + i.y * side + i.z;
    return voxel_index[offset];
}

#define VOXEL_TRAVERSAL_ALGORITHM(TO_INJECT)                                   \
    const float3 voxel_dimensions = (global_aabb.c1 - global_aabb.c0) / side;  \
                                                                               \
    int3 ind =                                                                 \
            get_starting_index(r.position, global_aabb, voxel_dimensions);   \
                                                                               \
    if (all((int3)(0) <= ind) && all(ind < (int3)(side))) {                    \
        const float3 c0 = convert_float3(ind + (int3)(0)) * voxel_dimensions;  \
        const float3 c1 = convert_float3(ind + (int3)(1)) * voxel_dimensions;  \
                                                                               \
        const aabb voxel_bounds = {global_aabb.c0 + c0, global_aabb.c0 + c1};  \
                                                                               \
        const int3 gt = signbit(r.direction);                                \
        const int3 step = select((int3)(1), (int3)(-1), gt);                   \
        const int3 just_out = select((int3)(side), (int3)(-1), gt);            \
        const float3 boundary = select(voxel_bounds.c1, voxel_bounds.c0, gt);  \
                                                                               \
        const float3 t_max_temp =                                              \
                fabs((boundary - r.position) / r.direction);               \
        float3 t_max =                                                         \
                select(t_max_temp, (float3)(INFINITY), isnan(t_max_temp));     \
        const float3 t_delta = fabs(voxel_dimensions / r.direction);         \
                                                                               \
        float prev_max = 0;                                                    \
                                                                               \
        for (;;) {                                                             \
            int min_i = 0;                                                     \
            for (int i = 1; i != 3; ++i) {                                     \
                if (t_max[i] < t_max[min_i]) {                                 \
                    min_i = i;                                                 \
                }                                                              \
            }                                                                  \
                                                                               \
            const uint voxel_offset = get_voxel_index(voxel_index, ind, side); \
            const uint num_triangles = voxel_index[voxel_offset];              \
            const global uint* voxel_begin = voxel_index + voxel_offset + 1;   \
            const float max_dist_inside_voxel = t_max[min_i];                  \
                                                                               \
            TO_INJECT                                                          \
                                                                               \
            ind[min_i] += step[min_i];                                         \
            if (ind[min_i] == just_out[min_i]) {                               \
                break;                                                         \
            }                                                                  \
            prev_max = t_max[min_i];                                           \
            t_max[min_i] += t_delta[min_i];                                    \
        }                                                                      \
    }

intersection voxel_traversal(ray r,
                             const global uint* voxel_index,
                             aabb global_aabb,
                             ulong side,
                             const global triangle* triangles,
                             const global float3* vertices,
                             ulong avoid_intersecting_with);
intersection voxel_traversal(ray r,
                             const global uint* voxel_index,
                             aabb global_aabb,
                             ulong side,
                             const global triangle* triangles,
                             const global float3* vertices,
                             ulong avoid_intersecting_with) {
    VOXEL_TRAVERSAL_ALGORITHM(
        const intersection state = ray_triangle_group_intersection(
                r, triangles, voxel_begin, num_triangles, vertices, avoid_intersecting_with);
        if (state.inter.t && state.inter.t <= max_dist_inside_voxel) {
            return state;
        }
    )
    return (intersection){};
}

bool voxel_inside_(ray r,
                  const global uint* voxel_index,
                  aabb global_aabb,
                  ulong side,
                  const global triangle* triangles,
                  const global float3* vertices);
bool voxel_inside_(ray r,
                  const global uint* voxel_index,
                  aabb global_aabb,
                  ulong side,
                  const global triangle* triangles,
                  const global float3* vertices) {
    uint count = 0;

    VOXEL_TRAVERSAL_ALGORITHM(
        for (uint i = 0; i != num_triangles; ++i) {
            uint triangle_to_test = voxel_begin[i];
            const triangle_inter inter = triangle_intersection(
                    triangles[triangle_to_test], vertices, r);
            if (inter.t < prev_max && inter.t<= max_dist_inside_voxel) {
                count += 1;
            }
        }
    )

    return count % 2;
}

bool voxel_inside(float3 pt,
                  const global uint* voxel_index,
                  aabb global_aabb,
                  ulong side,
                  const global triangle* triangles,
                  const global float3* vertices);
bool voxel_inside(float3 pt,
                  const global uint* voxel_index,
                  aabb global_aabb,
                  ulong side,
                  const global triangle* triangles,
                  const global float3* vertices) {
    const ray rays[] = {(ray){pt, (float3)(-1, 0, 0)},
                         (ray){pt, (float3)(1, 0, 0)},
                         (ray){pt, (float3)(0, -1, 0)},
                         (ray){pt, (float3)(0, 1, 0)},
                         (ray){pt, (float3)(0, 0, -1)},
                         (ray){pt, (float3)(0, 0, 1)}};
    uint count = 0;
    const size_t lim = sizeof(rays) / sizeof(ray);
    for (uint i = 0; i != lim; ++i) {
        if (voxel_inside_(rays[i], voxel_index, global_aabb, side, triangles, vertices)) {
            count += 1;
        }
    }
    return (lim / 2) < count;
}

bool voxel_point_intersection(float3 begin,
                              float3 point,
                              const global uint* voxel_index,
                              aabb global_aabb,
                              ulong side,
                              const global triangle* triangles,
                              const global float3* vertices,
                              ulong avoid_intersecting_with);
bool voxel_point_intersection(float3 begin,
                              float3 point,
                              const global uint* voxel_index,
                              aabb global_aabb,
                              ulong side,
                              const global triangle* triangles,
                              const global float3* vertices,
                              ulong avoid_intersecting_with) {
    const float3 begin_to_point = point - begin;
    const float mag = length(begin_to_point);
    const float3 direction = normalize(begin_to_point);

    const ray to_point = {begin, direction};

    const intersection inter = voxel_traversal(
            to_point, voxel_index, global_aabb, side, triangles, vertices, avoid_intersecting_with);

    return !inter.inter.t || mag < inter.inter.t;
}
#endif
)");
}  // namespace cl_sources
