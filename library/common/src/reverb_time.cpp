#include "common/reverb_time.h"
#include "common/geo/box.h"
#include "common/geo/geometric.h"

std::array<std::pair<cl_uint, cl_uint>, 3> get_index_pairs(const triangle& t) {
    return {{std::make_pair(t.v0, t.v1),
             std::make_pair(t.v1, t.v2),
             std::make_pair(t.v2, t.v0)}};
}

float six_times_tetrahedron_volume(const geo::triangle_vec3& t) {
    /// From Efficient Feature Extraction for 2d/3d Objects in Mesh
    /// Representation, Cha Zhang and Tsuhan Chen
    const auto volume =
            (t[1].x * t[2].y * t[0].z) - (t[2].x * t[1].y * t[0].z) +
            (t[2].x * t[0].y * t[1].z) - (t[0].x * t[2].y * t[1].z) +
            (t[0].x * t[1].y * t[2].z) - (t[1].x * t[0].y * t[2].z);
    const auto sign = glm::dot(glm::normalize(t[0]), geo::normal(t));
    return std::copysign(volume, sign);
}

////////////////////////////////////////////////////////////////////////////////

float estimate_air_intensity_absorption(float frequency, float humidity) {
    return (0.0275 / humidity) * std::pow(frequency / 1000, 1.7);
}
