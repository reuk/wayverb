#include "core/geo/rect.h"

namespace wayverb {
namespace core {
namespace geo {
namespace detail {

std::array<glm::vec2, 2> normals(const rect& r) {
    return std::array<glm::vec2, 2>{{glm::vec2(1, 0), glm::vec2(0, 1)}};
}

std::array<glm::vec2, 4> outline(const rect& r) {
    return std::array<glm::vec2, 4>{{r.get_min(),
                                     glm::vec2(r.get_max().x, r.get_min().y),
                                     r.get_max(),
                                     glm::vec2(r.get_min().x, r.get_max().y)}};
}

}  // namespace detail
}  // namespace geo
}  // namespace core
}  // namespace wayverb
