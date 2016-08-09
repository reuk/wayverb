#pragma once

#include "common/geo/overlaps_2d.h"
#include "common/range.h"

namespace geo {

using rect = util::range<glm::vec2>;

template <size_t n>
bool overlaps(const rect& b, const std::array<glm::vec2, n>& shape) {
    return geo::overlaps_2d(
            std::array<glm::vec2, 2>{{b.get_min(), b.get_max()}}, shape);
}

}  // namespace geo
