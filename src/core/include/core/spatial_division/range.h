#pragma once

#include "core/conversions.h"

#include "utilities/range.h"

#include "glm/glm.hpp"

namespace core {
namespace detail {
template <size_t dimensions>
struct range_value;

template <>
struct range_value<1> final {
    using type = float;
};
template <>
struct range_value<2> final {
    using type = glm::vec2;
};
template <>
struct range_value<3> final {
    using type = glm::vec3;
};

template <size_t n>
using range_value_t = typename range_value<n>::type;

template <size_t n>
using range_t = util::range<range_value_t<n>>;
}  // namespace detail
}  // namespace core
