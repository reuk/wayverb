#pragma once

#include "combined/model/member.h"

#include "core/geo/box.h"

namespace wayverb {
namespace combined {
namespace model {

class constrained_point final : public basic_member<constrained_point> {
public:
    explicit constrained_point(const core::geo::box& aabb);
    constrained_point(const core::geo::box& aabb, const glm::vec3& point);

    void set(const glm::vec3& position);
    glm::vec3 get() const;

    template <typename Archive>
    void load(Archive& archive) {
        archive(aabb_, point_);
    }

    template <typename Archive>
    void save(Archive& archive) const {
        archive(aabb_, point_); 
    }

    core::geo::box get_bounds() const;

private:
    core::geo::box aabb_;
    glm::vec3 point_;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
