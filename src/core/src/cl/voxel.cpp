#include "core/cl/voxel.h"

namespace wayverb {
namespace core {
namespace cl_sources {
const char* voxel = R"(
int3 get_starting_index(float3 position,
                        aabb global_aabb,
                        float3 voxel_dimensions);
int3 get_starting_index(float3 position,
                        aabb global_aabb,
                        float3 voxel_dimensions) {
    return convert_int3(floor((position - global_aabb.c0) / voxel_dimensions));
}

uint get_voxel_index(const global uint* voxel_index, int3 i, uint side);
uint get_voxel_index(const global uint* voxel_index, int3 i, uint side) {
    const size_t offset = i.x * side * side + i.y * side + i.z;
    return voxel_index[offset];
}

#define VOXEL_TRAVERSAL_ALGORITHM(TO_INJECT)                                   \
    const float3 voxel_dimensions = (global_aabb.c1 - global_aabb.c0) / side;  \
                                                                               \
    int3 ind = get_starting_index(r.position, global_aabb, voxel_dimensions);  \
                                                                               \
    if (all((int3)(0) <= ind) && all(ind < (int3)(side))) {                    \
        const float3 c0 = convert_float3(ind + (int3)(0)) * voxel_dimensions;  \
        const float3 c1 = convert_float3(ind + (int3)(1)) * voxel_dimensions;  \
                                                                               \
        const aabb voxel_bounds = {global_aabb.c0 + c0, global_aabb.c0 + c1};  \
                                                                               \
        const int3 gt = signbit(r.direction);                                  \
        const int3 step = select((int3)(1), (int3)(-1), gt);                   \
        const int3 just_out = select((int3)(side), (int3)(-1), gt);            \
        const float3 boundary = select(voxel_bounds.c1, voxel_bounds.c0, gt);  \
                                                                               \
        const float3 t_max_temp = fabs((boundary - r.position) / r.direction); \
        float3 t_max =                                                         \
                select(t_max_temp, (float3)(INFINITY), isnan(t_max_temp));     \
        const float3 t_delta = fabs(voxel_dimensions / r.direction);           \
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
                             uint side,
                             const global triangle* triangles,
                             const global float3* vertices,
                             uint avoid_intersecting_with);
intersection voxel_traversal(ray r,
                             const global uint* voxel_index,
                             aabb global_aabb,
                             uint side,
                             const global triangle* triangles,
                             const global float3* vertices,
                             uint avoid_intersecting_with) {
    VOXEL_TRAVERSAL_ALGORITHM(
            const intersection state =
                    ray_triangle_group_intersection(r,
                                                    triangles,
                                                    voxel_begin,
                                                    num_triangles,
                                                    vertices,
                                                    avoid_intersecting_with);
            if (state.inter.t && state.inter.t <= max_dist_inside_voxel) {
                return state;
            })
    return (intersection){};
}

//  Given a ray, and some voxelised geometry, find the number of intersections
//  between the ray and that geometry.
//  Returns the number of intersections, or ~(uint)(0) if any of the
//  intersections is questionable/degenerate (near edge or vertex)
uint count_intersections(ray r,
                         const global uint* voxel_index,
                         aabb global_aabb,
                         uint side,
                         const global triangle* triangles,
                         const global float3* vertices);
uint count_intersections(ray r,
                         const global uint* voxel_index,
                         aabb global_aabb,
                         uint side,
                         const global triangle* triangles,
                         const global float3* vertices) {
    uint count = 0;

    VOXEL_TRAVERSAL_ALGORITHM(for (uint i = 0; i != num_triangles; ++i) {
        const uint tri_ind = voxel_begin[i];
        const triangle tri = triangles[tri_ind];
        const triangle_inter inter = triangle_intersection(tri, vertices, r);
        if (inter.t) {
            if (is_degenerate(inter)) {
                return ~(uint)(0);
            }
            if (prev_max < inter.t && inter.t <= max_dist_inside_voxel) {
                count += 1;
            }
        }
    })

    return count;
}

//  Given a ray and some geometry, will work out whether the origin of the ray
//  is inside the geometry.
//  If not, it will return 0, if it is, it will return 1, it unsure, it will
//  return ~(uint)(0).
uint single_ray_inside(ray r,
                       const global uint* voxel_index,
                       aabb global_aabb,
                       uint side,
                       const global triangle* triangles,
                       const global float3* vertices);
uint single_ray_inside(ray r,
                       const global uint* voxel_index,
                       aabb global_aabb,
                       uint side,
                       const global triangle* triangles,
                       const global float3* vertices) {
    const uint intersections = count_intersections(
            r, voxel_index, global_aabb, side, triangles, vertices);
    if (intersections == ~(uint)(0)) {
        return ~(uint)(0);
    }
    return intersections % 2;
}

constant float3 random_directions[] = {
        (float3)(-0.427602, 0.791267, -0.437096),
        (float3)(-0.832527, -0.545442, 0.0969113),
        (float3)(0.633363, 0.413131, 0.65435),
        (float3)(0.985873, 0.140209, 0.0916325),
        (float3)(0.384519, 0.0309011, -0.9226),
        (float3)(-0.532584, -0.0244727, 0.846023),
        (float3)(0.844848, 0.230031, -0.483029),
        (float3)(-0.186143, -0.291698, -0.938223),
        (float3)(-0.108511, -0.861706, 0.495669),
        (float3)(0.0951741, 0.959367, -0.265625),
        (float3)(0.407194, 0.907127, -0.106369),
        (float3)(0.521731, -0.00522727, -0.853094),
        (float3)(0.369627, 0.218276, 0.903179),
        (float3)(-0.518837, 0.815586, -0.25618),
        (float3)(-0.954901, 0.105507, 0.277548),
        (float3)(0.63419, 0.768703, 0.0830607),
        (float3)(-0.0258027, 0.998294, 0.052379),
        (float3)(-0.868361, 0.473347, 0.147958),
        (float3)(0.346294, -0.131168, 0.928911),
        (float3)(-0.635896, 0.649019, 0.417624),
        (float3)(0.293121, 0.235495, -0.926619),
        (float3)(-0.55088, -0.0237137, -0.834247),
        (float3)(-0.661022, -0.653122, -0.369434),
        (float3)(0.224176, -0.351092, 0.909109),
        (float3)(0.456587, 0.736627, -0.498907),
        (float3)(0.965231, 0.154753, 0.210667),
        (float3)(0.626034, -0.245898, 0.740011),
        (float3)(0.435825, 0.794758, -0.422393),
        (float3)(0.662049, 0.713267, 0.23009),
        (float3)(0.261843, -0.620862, 0.738897),
        (float3)(0.23673, 0.714889, 0.657946),
        (float3)(-0.404007, 0.699316, 0.589691),
};

constant ulong num_random_directions =
        sizeof(random_directions) / sizeof(float3);

//  Given a point and some geometry, will work out whether the point lies inside
//  the geometry.
//  Will fire random rays, counting intersections along the rays.
bool voxel_inside(float3 pt,
                  const global uint* voxel_index,
                  aabb global_aabb,
                  uint side,
                  const global triangle* triangles,
                  const global float3* vertices);
bool voxel_inside(float3 pt,
                  const global uint* voxel_index,
                  aabb global_aabb,
                  uint side,
                  const global triangle* triangles,
                  const global float3* vertices) {
    for (uint i = 0; i != num_random_directions; ++i) {
        const float3 direction = random_directions[i];
        const ray r = {pt, direction};
        const uint single_result = single_ray_inside(
                r, voxel_index, global_aabb, side, triangles, vertices);
        if (single_result == 0 || single_result == 1) {
            return single_result;
        }
    }
    //  if we haven't returned here, we have a problem!
    //  options:
    //      return true or false
    //          we have a 50% chance of being right!
    //      return an error code
    //          not sure how the caller would handle this
    return false;
}

bool voxel_point_intersection(float3 begin,
                              float3 point,
                              const global uint* voxel_index,
                              aabb global_aabb,
                              uint side,
                              const global triangle* triangles,
                              const global float3* vertices,
                              uint avoid_intersecting_with);
bool voxel_point_intersection(float3 begin,
                              float3 point,
                              const global uint* voxel_index,
                              aabb global_aabb,
                              uint side,
                              const global triangle* triangles,
                              const global float3* vertices,
                              uint avoid_intersecting_with) {
    const float3 begin_to_point = point - begin;
    const float mag = length(begin_to_point);
    const float3 direction = normalize(begin_to_point);

    const ray to_point = {begin, direction};

    const intersection inter = voxel_traversal(to_point,
                                               voxel_index,
                                               global_aabb,
                                               side,
                                               triangles,
                                               vertices,
                                               avoid_intersecting_with);

    return !inter.inter.t || mag < inter.inter.t;
}
)";

}  // namespace cl_sources
}  // namespace core
}  // namespace wayverb
