#include "extended_algorithms.h"

#include <array>

namespace proc {

static_assert(get_min_tuple_size(std::make_tuple()) == 0, "min_tuple_size");
static_assert(get_min_tuple_size(std::make_tuple(1, 2, 3)) == 3,
              "min_tuple_size");
static_assert(get_min_tuple_size(std::make_tuple(1, 2, 3), std::make_tuple()) ==
                  0,
              "min_tuple_size");
static_assert(get_min_tuple_size(std::make_tuple(1, 2, 3),
                                 std::make_tuple(true, false, 1, 2, 3),
                                 std::make_tuple(5, 6),
                                 std::make_tuple("hello world", 1, 2, 3, 4)) ==
                  2,
              "min_tuple_size");

static_assert(zip_impl<0>(std::make_tuple(1, 2, 3)) == std::make_tuple(1),
              "zip_impl");
static_assert(zip_impl<1>(std::make_tuple(1, 2, 3)) == std::make_tuple(2),
              "zip_impl");
static_assert(zip_impl<0>(std::array<int, 5>{{1, 2, 3, 4, 5}},
                          std::make_tuple(true, false, "hi")) ==
                  std::make_tuple(1, true),
              "zip_impl");

static_assert(zip_to_tuple(std::make_tuple()) == std::make_tuple(),
              "zip_to_tuple");
static_assert(zip_to_tuple(std::make_tuple(1, 2, 3),
                           std::make_tuple(4, 5, 6)) ==
                  std::make_tuple(std::make_tuple(1, 4),
                                  std::make_tuple(2, 5),
                                  std::make_tuple(3, 6)),
              "zip_to_tuple");
static_assert(zip_to_tuple(std::make_tuple(1, 2, 3), std::make_tuple(4, 5)) ==
                  std::make_tuple(std::make_tuple(1, 4), std::make_tuple(2, 5)),
              "zip_to_tuple");

static_assert(c_eq(std::index_sequence<>(), 0), "c_eq");
static_assert(c_eq(std::index_sequence<0>(),
                   std::array<std::tuple<int, int>, 3>{
                       {std::make_tuple(1, 1),
                        std::make_tuple(3, 3),
                        std::make_tuple(5, 5)}}),
              "c_eq");
static_assert(c_eq(std::index_sequence<0, 2, 1>(),
                   std::array<std::tuple<int, int>, 3>{
                       {std::make_tuple(1, 1),
                        std::make_tuple(3, 3),
                        std::make_tuple(5, 5)}}),
              "c_eq");

static_assert(c_eq(std::array<std::tuple<bool, bool>, 2>{
                  {std::make_tuple(true, true),
                   std::make_tuple(false, false)}}),
              "c_eq");
static_assert(
    c_eq(std::array<std::tuple<int, int>, 3>{
        {std::make_tuple(1, 1), std::make_tuple(3, 3), std::make_tuple(5, 5)}}),
    "c_eq");

static_assert(!c_eq(std::array<int, 2>{{0, 1}}, std::array<int, 2>{{1, 2}}),
              "c_eq");
static_assert(c_eq(std::array<int, 2>{{1, 2}}, std::array<int, 2>{{1, 2}}),
              "c_eq");

static_assert(c_eq(tuple_to_array(std::make_tuple(1, 2, 3)),
                   std::array<int, 3>{{1, 2, 3}}),
              "tuple_to_array");
}
