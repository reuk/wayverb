#pragma once

#include "common/geo/overlaps_2d.h"
#include "utilities/range.h"

namespace geo {

using rect = range<glm::vec2>;

namespace detail {
std::array<glm::vec2, 2> normals(const rect& r);
std::array<glm::vec2, 4> outline(const rect& r);
}  // namespace detail

template <size_t n>
bool overlaps(const rect& rect, const std::array<glm::vec2, n>& shape) {
    const auto rect_axes{detail::normals(rect)};
    const auto rect_outline{detail::outline(rect)};
    const auto shape_axes{detail::normals_2d(shape)};
    return detail::overlaps(rect_outline.begin(),
                            rect_outline.end(),
                            shape.begin(),
                            shape.end(),
                            rect_axes.begin(),
                            rect_axes.end()) &&
           detail::overlaps(rect_outline.begin(),
                            rect_outline.end(),
                            shape.begin(),
                            shape.end(),
                            shape_axes.begin(),
                            shape_axes.end());
}

}  // namespace geo

inline auto inside(const geo::rect& a, const glm::vec2& b) {
    return glm::all(glm::lessThan(a.get_min(), b)) &&
           glm::all(glm::lessThan(b, a.get_max()));
}
