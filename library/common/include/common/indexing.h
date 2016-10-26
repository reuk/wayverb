#pragma once

#include "glm/glm.hpp"

namespace indexing {

/// types for indexing into n-dimensional data

template <size_t dimensions>
struct index;

template <>
struct index<1> {
    using type = unsigned;
    static constexpr auto numbered_dimensions = type{0};
};
template <>
struct index<2> {
    using type = glm::uvec2;
    static constexpr auto numbered_dimensions = type{0, 1};
};
template <>
struct index<3> {
    using type = glm::uvec3;
    static constexpr auto numbered_dimensions = type{0, 1, 2};
};

template <size_t n>
using index_t = typename index<n>::type;

template <size_t n>
constexpr auto numbered_dimensions_t = index<n>::numbered_dimensions;

////////////////////////////////////////////////////////////////////////////////

template <size_t n>
inline index_t<n> relative_position(size_t i) {
    const auto mask = index_t<n>(1) << numbered_dimensions_t<n>;
    return (mask & static_cast<unsigned>(i)) / mask;
}

////////////////////////////////////////////////////////////////////////////////

template <size_t n>
inline auto front(index_t<n> i) {
    return i[0];
}

template <size_t n>
inline auto back(index_t<n> i) {
    return i[n - 1];
}

////////////////////////////////////////////////////////////////////////////////

template <size_t n>
index_t<n - 1> reduce(index_t<n> i);

template <>
inline index_t<1> reduce<2>(index_t<2> i) {
    return front<2>(i);
}
template <>
inline index_t<2> reduce<3>(index_t<3> i) {
    return i;
}

////////////////////////////////////////////////////////////////////////////////

template <size_t n>
index_t<n - 1> tail(index_t<n> i);

template<>
inline index_t<1> tail<2>(index_t<2> i) {
    return back<2>(i);
}

template<>
inline index_t<2> tail<3>(index_t<3> i) {
    return index_t<2>{i[1], i[2]};
}

////////////////////////////////////////////////////////////////////////////////

template <size_t n>
inline unsigned product(index_t<n> i) {
    return front<n>(i) * product<n - 1>(tail<n>(i));
}

template<>
inline unsigned product<1>(index_t<1> i) {
    return i;
}

////////////////////////////////////////////////////////////////////////////////

/// Convert an n-dimensional index into a 1D one to index into a flat array.
template <size_t n>
inline unsigned flatten(index_t<n> i, index_t<n> size) {
    auto to_find_prod = size;
    to_find_prod[0] = i[0];
    return product<n>(to_find_prod) + flatten<n - 1>(tail<n>(i), tail<n>(size));
}

template <>
inline unsigned flatten<1>(index_t<1> i, index_t<1>) {
    return i;
}

}  // namespace indexing
