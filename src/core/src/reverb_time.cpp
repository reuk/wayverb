#include "core/reverb_time.h"
#include "core/geo/box.h"
#include "core/geo/geometric.h"

namespace wayverb {
namespace core {

std::array<std::pair<cl_uint, cl_uint>, 3> get_index_pairs(const triangle& t) {
    return {{std::make_pair(t.v0, t.v1),
             std::make_pair(t.v1, t.v2),
             std::make_pair(t.v2, t.v0)}};
}

float six_times_tetrahedron_volume(const geo::triangle_vec3& t) {
    /// From Efficient Feature Extraction for 2d/3d Objects in Mesh
    /// Representation, Cha Zhang and Tsuhan Chen
    const auto volume =
            (std::get<1>(t.s).x * std::get<2>(t.s).y * std::get<0>(t.s).z) -
            (std::get<2>(t.s).x * std::get<1>(t.s).y * std::get<0>(t.s).z) +
            (std::get<2>(t.s).x * std::get<0>(t.s).y * std::get<1>(t.s).z) -
            (std::get<0>(t.s).x * std::get<2>(t.s).y * std::get<1>(t.s).z) +
            (std::get<0>(t.s).x * std::get<1>(t.s).y * std::get<2>(t.s).z) -
            (std::get<1>(t.s).x * std::get<0>(t.s).y * std::get<2>(t.s).z);
    const auto sign =
            glm::dot(glm::normalize(std::get<0>(t.s)), geo::normal(t));
    return std::copysign(volume, sign);
}

////////////////////////////////////////////////////////////////////////////////

float estimate_air_intensity_absorption(float frequency, float humidity) {
    return (0.0275 / humidity) * std::pow(frequency / 1000, 1.7);
}

}  // namespace core
}  // namespace wayverb
