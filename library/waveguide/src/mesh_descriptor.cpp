#include "waveguide/config.h"
#include "waveguide/mesh_descriptor.h"

size_t compute_index(const mesh_descriptor& d, const glm::ivec3& pos) {
    return pos.x + pos.y * d.dimensions.s[0] +
           pos.z * d.dimensions.s[0] * d.dimensions.s[1];
}

size_t compute_index(const mesh_descriptor& d, const glm::vec3& pos) {
    return compute_index(d, compute_locator(d, pos));
}

glm::ivec3 compute_locator(const mesh_descriptor& d, size_t index) {
    const auto x{div(index, d.dimensions.s[0])};
    const auto y{div(x.quot, d.dimensions.s[1])};
    return glm::ivec3{x.rem, y.rem, y.quot % d.dimensions.s[2]};
}

glm::ivec3 compute_locator(const mesh_descriptor& d, const glm::vec3& v) {
    const auto transformed{v - to_vec3(d.min_corner)};
    return glm::round(transformed / d.spacing);
}

glm::vec3 compute_position(const mesh_descriptor& d,
                           const glm::ivec3& locator) {
    return to_vec3(d.min_corner) + glm::vec3{locator} * d.spacing;
}

glm::vec3 compute_position(const mesh_descriptor& d, size_t index) {
    return compute_position(d, compute_locator(d, index));
}

void compute_neighbors(const mesh_descriptor& d,
                       size_t index,
                       cl_uint* output) {
    const auto loc = compute_locator(d, index);
    const std::array<glm::ivec3, 6> n_loc{{
            glm::ivec3(loc.x - 1, loc.y, loc.z),
            glm::ivec3(loc.x + 1, loc.y, loc.z),
            glm::ivec3(loc.x, loc.y - 1, loc.z),
            glm::ivec3(loc.x, loc.y + 1, loc.z),
            glm::ivec3(loc.x, loc.y, loc.z - 1),
            glm::ivec3(loc.x, loc.y, loc.z + 1),
    }};

    std::transform(
            std::begin(n_loc), std::end(n_loc), output, [&](const auto& i) {
                auto inside =
                        glm::all(glm::lessThanEqual(glm::ivec3(0), i)) &&
                        glm::all(glm::lessThan(i, to_ivec3(d.dimensions)));
                return inside ? compute_index(d, i)
                              : mesh_descriptor::no_neighbor;
            });
}

std::array<cl_uint, 6> compute_neighbors(const mesh_descriptor& d,
                                         size_t index) {
    std::array<cl_uint, 6> ret;
    compute_neighbors(d, index, ret.data());
    return ret;
}

geo::box compute_aabb(const mesh_descriptor& d) {
    return geo::box{to_vec3(d.min_corner),
                    to_vec3(d.min_corner) +
                            glm::vec3{to_ivec3(d.dimensions)} * d.spacing};
}

double compute_sample_rate(const mesh_descriptor& d, double speed_of_sound) {
    return 1 / waveguide::config::time_step(speed_of_sound, d.spacing);
}

size_t compute_num_nodes(const mesh_descriptor& d) {
    return d.dimensions.s[0] * d.dimensions.s[1] * d.dimensions.s[2];
}

aligned::vector<glm::vec3> compute_node_positions(const mesh_descriptor& d) {
    aligned::vector<glm::vec3> ret;
    const auto nodes{compute_num_nodes(d)};
    ret.reserve(nodes);
    for (auto i{0u}; i != nodes; ++i) {
        ret.emplace_back(compute_position(d, i));
    }
    return ret;
}
