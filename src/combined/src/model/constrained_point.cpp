#include "combined/model/constrained_point.h"

namespace wayverb {
namespace combined {
namespace model {

constrained_point::constrained_point(const core::geo::box& aabb)
        : constrained_point(aabb, centre(aabb)) {}

constrained_point::constrained_point(const core::geo::box& aabb,
                                     const glm::vec3& point)
        : aabb_{aabb}
        , point_{point} {}

void constrained_point::set(const glm::vec3& position) {
    point_ = clamp(position, aabb_);
    notify();
}

glm::vec3 constrained_point::get() const { return point_; }

}  // namespace model
}  // namespace combined
}  // namespace wayverb
