#include "common/cl_math.h"

#include <array>

constexpr auto a = cl_float4{{1, 2, 3, 4}};
constexpr auto b = cl_float4{{2, 4, 6, 8}};
constexpr auto c = cl_float4{{3, 6, 9, 12}};

namespace cl_math {

namespace detail {

static_assert(zip_row<0>(a, b) == std::make_tuple(1, 2), "zip_row");
static_assert(zip_row<1>(a, b) == std::make_tuple(2, 4), "zip_row");
static_assert(zip_row<2>(a, b) == std::make_tuple(3, 6), "zip_row");
static_assert(zip_row<3>(a, b) == std::make_tuple(4, 8), "zip_row");

static_assert(zip_row<0>(a, b, c) == std::make_tuple(1, 2, 3), "zip_row");
static_assert(zip_row<1>(a, b, c) == std::make_tuple(2, 4, 6), "zip_row");
static_assert(zip_row<2>(a, b, c) == std::make_tuple(3, 6, 9), "zip_row");
static_assert(zip_row<3>(a, b, c) == std::make_tuple(4, 8, 12), "zip_row");

}  // namespace detail

static_assert(proc::c_eq(zip(a, b),
                         std::array<std::tuple<float, float>, 4>{{
                             std::make_tuple(1, 2),
                             std::make_tuple(2, 4),
                             std::make_tuple(3, 6),
                             std::make_tuple(4, 8),
                         }}),
              "zip");

static_assert(proc::c_eq(zip(a, b, c),
                         std::array<std::tuple<float, float, float>, 4>{{
                             std::make_tuple(1, 2, 3),
                             std::make_tuple(2, 4, 6),
                             std::make_tuple(3, 6, 9),
                             std::make_tuple(4, 8, 12),
                         }}),
              "zip");

static_assert(
    proc::c_eq(
        apply(proc::InvokeFunctor<std::plus<float>>(std::plus<float>()), a, b),
        std::array<float, 4>{{3, 6, 9, 12}}),
    "apply");

}  // namespace cl_math

static_assert(a + b == cl_float4{{3, 6, 9, 12}}, "apply");
static_assert(a - b == cl_float4{{-1, -2, -3, -4}}, "apply");
static_assert(a * b == cl_float4{{2, 8, 18, 32}}, "apply");
static_assert(a / b == cl_float4{{0.5f, 0.5f, 0.5f, 0.5f}}, "apply");
