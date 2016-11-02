#include "waveguide/boundary_coefficient_program.h"
#include "waveguide/cl/boundary_index_array.h"
#include "waveguide/cl/structs.h"

#include "core/cl/geometry.h"
#include "core/cl/geometry_structs.h"
#include "core/cl/voxel.h"

namespace waveguide {

constexpr auto source = R"(
//  adapted from
//  http://www.geometrictools.com/GTEngine/Include/Mathematics/GteDistPointtriangleExact.h
float point_triangle_distance_squared(triangle_verts triangle, float3 point);
float point_triangle_distance_squared(triangle_verts triangle, float3 point) {
    //  do I hate this? yes
    //  am I going to do anything about it? it works
    const float3 diff = point - triangle.v0;
    const float3 e0 = triangle.v1 - triangle.v0;
    const float3 e1 = triangle.v2 - triangle.v0;
    const float a00 = dot(e0, e0);
    const float a01 = dot(e0, e1);
    const float a11 = dot(e1, e1);
    const float b0 = -dot(diff, e0);
    const float b1 = -dot(diff, e1);
    const float det = a00 * a11 - a01 * a01;

    float t0 = a01 * b1 - a11 * b0;
    float t1 = a01 * b0 - a00 * b1;

    if (t0 + t1 <= det) {
        if (t0 < 0) {
            if (t1 < 0) {
                if (b0 < 0) {
                    t1 = 0;
                    if (a00 <= -b0) {
                        t0 = 1;
                    } else {
                        t0 = -b0 / a00;
                    }
                } else {
                    t0 = 0;
                    if (0 <= b1) {
                        t1 = 0;
                    } else if (a11 <= -b1) {
                        t1 = 1;
                    } else {
                        t1 = -b1 / a11;
                    }
                }
            } else {
                t0 = 0;
                if (0 <= b1) {
                    t1 = 0;
                } else if (a11 <= -b1) {
                    t1 = 1;
                } else {
                    t1 = -b1 / a11;
                }
            }
        } else if (t1 < 0) {
            t1 = 0;
            if (0 <= b0) {
                t0 = 0;
            } else if (a00 <= -b0) {
                t0 = 1;
            } else {
                t0 = -b0 / a00;
            }
        } else {
            const float invDet = 1 / det;
            t0 *= invDet;
            t1 *= invDet;
        }
    } else {
        if (t0 < 0) {
            const float tmp0 = a01 + b0;
            const float tmp1 = a11 + b1;
            if (tmp0 < tmp1) {
                const float numer = tmp1 - tmp0;
                const float denom = a00 - 2 * a01 + a11;
                if (denom <= numer) {
                    t0 = 1;
                    t1 = 0;
                } else {
                    t0 = numer / denom;
                    t1 = 1 - t0;
                }
            } else {
                t0 = 0;
                if (tmp1 <= 0) {
                    t1 = 1;
                } else if (0 <= b1) {
                    t1 = 0;
                } else {
                    t1 = -b1 / a11;
                }
            }
        } else if (t1 < 0) {
            const float tmp0 = a01 + b1;
            const float tmp1 = a00 + b0;
            if (tmp0 < tmp1) {
                const float numer = tmp1 - tmp0;
                const float denom = a00 - 2 * a01 + a11;
                if (denom <= numer) {
                    t1 = 1;
                    t0 = 0;
                } else {
                    t1 = numer / denom;
                    t0 = 1 - t1;
                }
            } else {
                t1 = 0;
                if (tmp1 <= 0) {
                    t0 = 1;
                } else if (0 <= b0) {
                    t0 = 0;
                } else {
                    t0 = -b0 / a00;
                }
            }
        } else {
            const float numer = a11 + b1 - a01 - b0;
            if (numer <= 0) {
                t0 = 0;
                t1 = 1;
            } else {
                const float denom = a00 - 2 * a01 + a11;
                if (denom <= numer) {
                    t0 = 1;
                    t1 = 0;
                } else {
                    t0 = numer / denom;
                    t1 = 1 - t0;
                }
            }
        }
    }

    const float3 closest = triangle.v0 + e0 * t0 + e1 * t1;
    const float3 d = point - closest;
    return dot(d, d);
}

float point_triangle_dist_squared(triangle t, const global float3* vertices, float3 point);
float point_triangle_dist_squared(triangle t, const global float3* vertices, float3 point) {
    const triangle_verts this_triangle_verts = {vertices[t.v0],
                                                vertices[t.v1],
                                                vertices[t.v2]};
    return point_triangle_distance_squared(this_triangle_verts, point);
}

float min_dist_to_cuboid_squared(float3 pt, aabb cuboid);
float min_dist_to_cuboid_squared(float3 pt, aabb cuboid) {
    const float3 d = max((float3)(0), max(cuboid.c0 - pt, pt - cuboid.c1));
    return dot(d, d);
}

typedef struct {
    uint triangle;
    float distance_squared;
} triangle_distance_pair;

triangle_distance_pair closest_triangle_in_voxel(
        float3 pt,
        const global uint* voxel_index,
        aabb global_aabb,
        uint side,
        const global triangle* triangles,
        const global float3* vertices,
        int3 this_voxel_index,
        float3 voxel_dimensions,
        float distance_squared);
triangle_distance_pair closest_triangle_in_voxel(
        float3 pt,
        const global uint* voxel_index,
        aabb global_aabb,
        uint side,
        const global triangle* triangles,
        const global float3* vertices,
        int3 this_voxel_index,
        float3 voxel_dimensions,
        float distance_squared) {
    const float3 this_voxel_c0 =
            global_aabb.c0 + (this_voxel_index + (int3)(0)) * voxel_dimensions;
    const float3 this_voxel_c1 =
            global_aabb.c1 + (this_voxel_index + (int3)(1)) * voxel_dimensions;

    const aabb this_voxel_aabb = (aabb){this_voxel_c0, this_voxel_c1};

    const float min_dist_to_voxel =
            min_dist_to_cuboid_squared(pt, this_voxel_aabb);

    triangle_distance_pair ret = {0, INFINITY};

    //  if the voxel is within the checking distance
    if (min_dist_to_voxel <= distance_squared) {
        const uint voxel_offset =
                get_voxel_index(voxel_index, this_voxel_index, side);
        const uint num_triangles = voxel_index[voxel_offset];
        const global uint* it = voxel_index + voxel_offset + 1;
        const global uint* voxel_end = it + num_triangles;

        //  for each triangle in the voxel
        for (; it != voxel_end; ++it) {
            //  find squared distance to the triangle
            const uint this_index = *it;
            const triangle this_triangle = triangles[this_index];
            const float d = point_triangle_dist_squared(this_triangle, vertices, pt);

            if (d < ret.distance_squared && d <= distance_squared) {
                ret.triangle = *it;
                ret.distance_squared = d;
            }
        }
    }

    return ret;
}

uint slow_closest_triangle(float3 pt,
                           const global triangle* triangles,
                           uint num_triangles,
                           const global float3* vertices);
uint slow_closest_triangle(float3 pt,
                           const global triangle* triangles,
                           uint num_triangles,
                           const global float3* vertices) {
    uint ret = 0;
    float distance = INFINITY;
    for (uint i = 0; i != num_triangles; ++i) {
        const triangle this_triangle = triangles[i];
        const float new_dist = point_triangle_dist_squared(this_triangle, vertices, pt);
        if (new_dist < distance) {
            ret = i;
            distance = new_dist;
        }
    }
    return ret;
}

uint closest_triangle(float3 pt,
                      const global uint* voxel_index,
                      aabb global_aabb,
                      uint side,
                      const global triangle* triangles,
                      const global float3* vertices);
uint closest_triangle(float3 pt,
                      const global uint* voxel_index,
                      aabb global_aabb,
                      uint side,
                      const global triangle* triangles,
                      const global float3* vertices) {
    const float3 voxel_dimensions = (global_aabb.c1 - global_aabb.c0) / side;
    const int3 starting_index =
            get_starting_index(pt, global_aabb, voxel_dimensions);

    //  keep increasing search distance until we find a triangle
    const float lim = distance(global_aabb.c0, global_aabb.c1);
    for (float dist = length(voxel_dimensions); dist < lim; dist *= 1.6) {
        const float distance_squared = dist * dist;

        //  find the range of the voxel structure to search
        const int3 index_diff = convert_int3(ceil((float3)(dist) / voxel_dimensions));

        const int3 min_diff = max(starting_index - index_diff, (int3)(0));
        const int3 max_diff =
                min(starting_index + index_diff + (int3)(1), (int3)(side));

        triangle_distance_pair ret = {0, INFINITY};

        //  for each voxel in the search range
        for (int x = min_diff.x; x != max_diff.x; ++x) {
            for (int y = min_diff.y; y != max_diff.y; ++y) {
                for (int z = min_diff.z; z != max_diff.z; ++z) {
                    const int3 this_voxel_index = (int3)(x, y, z);

                    //  find the closest triangle in this voxel
                    const triangle_distance_pair pair =
                            closest_triangle_in_voxel(pt,
                                                      voxel_index,
                                                      global_aabb,
                                                      side,
                                                      triangles,
                                                      vertices,
                                                      this_voxel_index,
                                                      voxel_dimensions,
                                                      distance_squared);

                    //  if it's closer than the current closest, update the
                    //  current closest with the nearer results
                    if (pair.distance_squared < ret.distance_squared) {
                        ret = pair;
                    }
                }
            }
        }

        //  if we found a triangle within the search range, return it
        if (ret.distance_squared < INFINITY) {
            return ret.triangle;
        }
    }

    //  if we get here there are bad problems
    return ~(uint)(0);
}

kernel void boundary_coefficient_finder_1d(
        const global condensed_node* nodes,  //  io
        const mesh_descriptor descriptor,

        global boundary_index_array_1* boundary,

        const global uint* voxel_index,  //  voxel
        aabb global_aabb,
        uint side,

        const global triangle* triangles,  //  scene
        uint num_triangles,
        const global float3* vertices) {
    const size_t thread = get_global_id(0);

    const int bt = nodes[thread].boundary_type;
    const int popcnt = popcount(bt);

    //  if node is 1d or reentrant
    if (popcnt != 1) {
        return;
    }

    const uint this_boundary_index = nodes[thread].boundary_index;

    //  find the closest triangle
    const int3 locator = to_locator(thread, descriptor.dimensions);
    const float3 pt = compute_node_position(descriptor, locator);
    const uint closest_triangle_index = slow_closest_triangle(pt, triangles, num_triangles, vertices);
    const uint s = triangles[closest_triangle_index].surface;

    //  now set the boundary to the triangle's surface
    boundary[this_boundary_index].array[0] = s;
}

boundary_type port_index_to_boundary_type(uint i);
boundary_type port_index_to_boundary_type(uint i) { return 1 << (i + 1); }

constant boundary_type boundary_dirs[] = {
        id_nx, id_px, id_ny, id_py, id_nz, id_pz};
constant size_t num_boundary_dirs = sizeof(boundary_dirs) / sizeof(int);

constant int3 adjacent_2d[] = {(int3)(-1, 0, 0),
                               (int3)(1, 0, 0),
                               (int3)(0, -1, 0),
                               (int3)(0, 1, 0),
                               (int3)(0, 0, -1),
                               (int3)(0, 0, 1)};
constant size_t num_adjacent_2d = sizeof(adjacent_2d) / sizeof(int3);

kernel void boundary_coefficient_finder_2d(
        const global condensed_node* nodes,  //  io
        const mesh_descriptor descriptor,
        global boundary_index_array_2* boundary_2d,
        const global boundary_index_array_1* boundary_1d) {
    const size_t thread = get_global_id(0);

    const int bt = nodes[thread].boundary_type;
    const int popcnt = popcount(bt);

    //  if node is 2d
    if (popcnt != 2) {
        return;
    }

    const uint this_bi = nodes[thread].boundary_index;
    const int3 this_locator = to_locator(thread, descriptor.dimensions);

    //  for each boundary direction in order
    uint count = 0;
    for (uint i = 0; i != 6; ++i) {
        const boundary_type boundary_dir = port_index_to_boundary_type(i);
        //  if this is one of this node's boundary directions
        if (!(bt & boundary_dir)) {
            continue;
        }

        //  for each of the six adjacent 1d node locations
        for (uint j = 0; j != num_adjacent_2d; ++j) {
            const int3 adjacent_locator = this_locator + adjacent_2d[j];

            if (any(adjacent_locator < (int3)(0)) ||
                any(descriptor.dimensions <= adjacent_locator)) {
                continue;
            }

            const uint adjacent_index =
                    to_index(adjacent_locator, descriptor.dimensions);
            const int adjacent_type = nodes[adjacent_index].boundary_type;
            const int adjacent_pop = popcount(adjacent_type);

            //  if there is a 1d node in the right direction here
            if (adjacent_pop != 1) {
                continue;
            }

            //  use its surface
            const uint adjacent_bi = nodes[adjacent_index].boundary_index;
            const uint s = boundary_1d[adjacent_bi].array[0];
            boundary_2d[this_bi].array[count] = s;
            count += 1;
            break;
        }
    }
}

constant int3 adjacent_3d[] = {(int3)(-1, -1, 0),
                               (int3)(-1, 1, 0),
                               (int3)(1, -1, 0),
                               (int3)(1, 1, 0),
                               (int3)(-1, 0, -1),
                               (int3)(-1, 0, 1),
                               (int3)(1, 0, -1),
                               (int3)(1, 0, 1),
                               (int3)(0, -1, -1),
                               (int3)(0, -1, 1),
                               (int3)(0, 1, -1),
                               (int3)(0, 1, 1)};
constant size_t num_adjacent_3d = sizeof(adjacent_3d) / sizeof(int3);

kernel void boundary_coefficient_finder_3d(
        const global condensed_node* nodes,  //  io
        const mesh_descriptor descriptor,
        global boundary_index_array_3* boundary_3d,
        const global boundary_index_array_1* boundary_1d) {
    const size_t thread = get_global_id(0);

    const int bt = nodes[thread].boundary_type;
    const int popcnt = popcount(bt);

    //  if node is 3d
    if (popcnt != 3) {
        return;
    }

    const uint this_bi = nodes[thread].boundary_index;
    const int3 this_locator = to_locator(thread, descriptor.dimensions);

    //  for each boundary direction in order
    uint count = 0;
    for (uint i = 0; i != 6; ++i) {
        const boundary_type boundary_dir = port_index_to_boundary_type(i);
        //  if this is one of this node's boundary directions
        if (!(bt & boundary_dir)) {
            continue;
        }

        //  for each of the twelve kinda-adjacent 1d node locations
        for (uint j = 0; j != num_adjacent_3d; ++j) {
            const int3 adjacent_locator = this_locator + adjacent_3d[j];

            if (any(adjacent_locator < (int3)(0)) ||
                any(descriptor.dimensions <= adjacent_locator)) {
                continue;
            }

            const uint adjacent_index = 
                    to_index(adjacent_locator, descriptor.dimensions);
            const int adjacent_type = nodes[adjacent_index].boundary_type;
            const int adjacent_pop = popcount(adjacent_type);

            //  if there is a 1d node in the right direction here
            if (adjacent_pop != 1) {
                continue;
            }

            //  use its surface
            const uint adjacent_bi = nodes[adjacent_index].boundary_index;
            const uint s = boundary_1d[adjacent_bi].array[0];
            boundary_3d[this_bi].array[count] = s;
            count += 1;
            break;
        }
    }
}
)";

boundary_coefficient_program::boundary_coefficient_program(
        const compute_context& cc)
        : wrapper(cc,
                  std::vector<std::string>{
                          cl_representation_v<mesh_descriptor>,
                          cl_representation_v<boundary_type>,
                          cl_representation_v<condensed_node>,
                          cl_representation_v<boundary_index_array_1>,
                          cl_representation_v<boundary_index_array_2>,
                          cl_representation_v<boundary_index_array_3>,
                          cl_representation_v<aabb>,
                          cl_representation_v<ray>,
                          cl_representation_v<triangle_inter>,
                          cl_representation_v<intersection>,
                          cl_representation_v<triangle_verts>,
                          cl_representation_v<triangle>,
                          ::cl_sources::utils,
                          ::cl_sources::geometry,
                          ::cl_sources::voxel,
                          source}) {}

}  // namespace waveguide
