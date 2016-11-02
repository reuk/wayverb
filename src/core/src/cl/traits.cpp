#include "core/cl/traits.h"

//  vector types should know they're vector types
static_assert(detail::is_vector_type_v<cl_float3>, "");
static_assert(detail::is_vector_type_v<cl_uchar16>, "");

//  non-vector types should know they're not vector types
static_assert(!detail::is_vector_type_v<float>, "");
static_assert(!detail::is_vector_type_v<double>, "");
static_assert(!detail::is_vector_type_v<unsigned>, "");
static_assert(!detail::is_vector_type_v<cl_ulong>, "");

//  should be able to accumulate across vectors
static_assert(detail::accumulate(cl_int2{{1, 2}}, 0, std::plus<>()) == 3, "");

static_assert(
        detail::accumulate(cl_int4{{1, 1, 1, 1}}, 1, std::logical_and<>()), "");
static_assert(!detail::accumulate(cl_int4{{1, 0, 1, 1}},
                                  1,
                                  std::logical_and<>()),
              "");

//  zip
static_assert(detail::zip(cl_int4{{1, 2, 3, 4}}, cl_float{1}, std::plus<>()) ==
                      cl_float4{{2, 3, 4, 5}},
              "");
static_assert(detail::zip(cl_int4{{4, 5, 6, 7}},
                          cl_float4{{0, 1, 2, 3}},
                          std::multiplies<>()) == cl_float4{{0, 5, 12, 21}},
              "");

//  check some arithmetic!
static_assert(cl_float2{{1, 2}} + cl_float2{{3, 4}} == cl_float2{{4, 6}}, "");
static_assert(cl_int2{{1, 2}} + cl_float2{{3, 4}} == cl_float2{{4, 6}}, "");

static_assert(cl_int2{{1, 2}} + cl_float{3} == cl_float2{{4, 5}}, "");
static_assert(cl_int8{{1, 2, 3, 4, 5, 6, 7, 8}} + cl_float{3} ==
                      cl_float8{{4, 5, 6, 7, 8, 9, 10, 11}},
              "");
static_assert(cl_double{2} * cl_float4{{1, 0, 1, 0}} ==
                      cl_double4{{2, 0, 2, 0}},
              "");
static_assert(cl_int4{{10, 20, 30, 40}} % 4 == cl_int4{{2, 0, 2, 0}}, "");
