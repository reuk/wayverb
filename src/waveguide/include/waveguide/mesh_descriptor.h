#pragma once

#include "core/cl/representation.h"
#include "core/geo/box.h"

#include "glm/glm.hpp"

#include <array>

struct alignas(1 << 4) mesh_descriptor final {
    static constexpr auto no_neighbor = ~cl_uint{0};

    cl_float3 min_corner;
    cl_int3 dimensions;
    cl_float spacing;
};

template <>
struct core::cl_representation<mesh_descriptor> final {
    static constexpr auto value = R"(
typedef struct {
    float3 min_corner;
    int3 dimensions;
    float spacing;
} mesh_descriptor;
)";
};

constexpr auto to_tuple(const mesh_descriptor& x) {
    return std::tie(x.min_corner, x.dimensions, x.spacing);
}

constexpr bool operator==(const mesh_descriptor& a, const mesh_descriptor& b) {
    return to_tuple(a) == to_tuple(b);
}

constexpr bool operator!=(const mesh_descriptor& a, const mesh_descriptor& b) {
    return !(a == b);
}

size_t compute_index(const mesh_descriptor& d, const glm::ivec3& pos);
size_t compute_index(const mesh_descriptor& d, const glm::vec3& pos);

glm::ivec3 compute_locator(const mesh_descriptor& d, size_t index);
glm::ivec3 compute_locator(const mesh_descriptor& d, const glm::vec3& v);

glm::vec3 compute_position(const mesh_descriptor& d, const glm::ivec3& locator);
glm::vec3 compute_position(const mesh_descriptor& d, size_t index);

void compute_neighbors(const mesh_descriptor& d, size_t index, cl_uint* output);
std::array<cl_uint, 6> compute_neighbors(const mesh_descriptor& d,
                                         size_t index);

core::geo::box compute_aabb(const mesh_descriptor& d);

double compute_sample_rate(const mesh_descriptor& d, double speed_of_sound);

size_t compute_num_nodes(const mesh_descriptor& d);

util::aligned::vector<glm::vec3> compute_node_positions(
        const mesh_descriptor& d);
