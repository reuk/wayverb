#pragma once

#include "common/cl/include.h"
#include "common/geo/box.h"

#include "glm/glm.hpp"

#include <array>

namespace waveguide {

struct descriptor final {
    static constexpr auto no_neighbor{~cl_uint{0}};
    using locator = glm::ivec3;

    glm::vec3 min_corner;
    glm::ivec3 dimensions;
    float spacing;
};

size_t compute_index(const descriptor& d, const descriptor::locator& pos);
size_t compute_index(const descriptor& d, const glm::vec3& pos);

descriptor::locator compute_locator(const descriptor& d, size_t index);
descriptor::locator compute_locator(const descriptor& d, const glm::vec3& v);

glm::vec3 compute_position(const descriptor& d,
                           const descriptor::locator& locator);
glm::vec3 compute_position(const descriptor& d, size_t index);

void compute_neighbors(const descriptor& d, size_t index, cl_uint* output);
std::array<cl_uint, 6> compute_neighbors(const descriptor& d, size_t index);

geo::box compute_aabb(const descriptor& d);

double compute_sample_rate(const descriptor& d, double speed_of_sound);

}  // namespace waveguide
