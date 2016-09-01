#include "waveguide/descriptor.h"
#include "common/stl_wrappers.h"
#include "waveguide/config.h"

namespace waveguide {

size_t compute_index(const descriptor& d, const glm::ivec3& pos) {
    return pos.x + pos.y * d.dimensions.x +
           pos.z * d.dimensions.x * d.dimensions.y;
}

size_t compute_index(const descriptor& d, const glm::vec3& pos) {
    return compute_index(d, compute_locator(d, pos));
}

glm::ivec3 compute_locator(const descriptor& d, size_t index) {
    auto x = div(index, d.dimensions.x);
    auto y = div(x.quot, d.dimensions.y);
    return glm::ivec3{x.rem, y.rem, y.quot % d.dimensions.z};
}

glm::ivec3 compute_locator(const descriptor& d, const glm::vec3& v) {
    const auto transformed{v - d.min_corner};
    return glm::round(transformed / d.spacing);
}

glm::vec3 compute_position(const descriptor& d, const glm::ivec3& locator) {
    return d.min_corner + glm::vec3{locator} * d.spacing;
}

glm::vec3 compute_position(const descriptor& d, size_t index) {
    return compute_position(d, compute_locator(d, index));
}

void compute_neighbors(const descriptor& d, size_t index, cl_uint* output) {
    const auto loc = compute_locator(d, index);
    const std::array<glm::ivec3, 6> n_loc{{
            glm::ivec3(loc.x - 1, loc.y, loc.z),
            glm::ivec3(loc.x + 1, loc.y, loc.z),
            glm::ivec3(loc.x, loc.y - 1, loc.z),
            glm::ivec3(loc.x, loc.y + 1, loc.z),
            glm::ivec3(loc.x, loc.y, loc.z - 1),
            glm::ivec3(loc.x, loc.y, loc.z + 1),
    }};

    proc::transform(n_loc, output, [&](const auto& i) {
        auto inside = glm::all(glm::lessThanEqual(glm::ivec3(0), i)) &&
                      glm::all(glm::lessThan(i, d.dimensions));
        return inside ? compute_index(d, i) : descriptor::no_neighbor;
    });
}

std::array<cl_uint, 6> compute_neighbors(const descriptor& d, size_t index) {
    std::array<cl_uint, 6> ret;
    compute_neighbors(d, index, ret.data());
    return ret;
}

geo::box compute_aabb(const descriptor& d) {
    return geo::box{d.min_corner,
                    d.min_corner + glm::vec3{d.dimensions} * d.spacing};
}

double compute_sample_rate(const descriptor& d, double speed_of_sound) {
    return 1 / config::time_step(speed_of_sound, d.spacing);
}

size_t compute_num_nodes(const descriptor& d) {
    return d.dimensions.x * d.dimensions.y * d.dimensions.z;
}

aligned::vector<glm::vec3> compute_node_positions(const descriptor& d) {
    aligned::vector<glm::vec3> ret;
    const auto nodes{compute_num_nodes(d)};
    ret.reserve(nodes);
    for (auto i{0u}; i != nodes; ++i) {
        ret.push_back(compute_position(d, i));
    }
    return ret;
}

}  // namespace waveguide
