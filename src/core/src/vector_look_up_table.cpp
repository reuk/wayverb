#include "core/vector_look_up_table.h"

namespace {

using test_table_a = vector_look_up_table<float, 24, 11>;

static_assert(test_table_a::get_azimuth_angle() == 15, "get_azimuth_angle");
static_assert(test_table_a::get_elevation_angle() == 15, "get_elevation_angle");

static_assert(test_table_a::azimuth_to_index(0) == 0, "azimuth_to_index");
static_assert(test_table_a::azimuth_to_index(5) == 0, "azimuth_to_index");
static_assert(test_table_a::azimuth_to_index(-5) == 0, "azimuth_to_index");
static_assert(test_table_a::azimuth_to_index(355) == 0, "azimuth_to_index");
static_assert(test_table_a::azimuth_to_index(10) == 1, "azimuth_to_index");
static_assert(test_table_a::azimuth_to_index(15) == 1, "azimuth_to_index");
static_assert(test_table_a::azimuth_to_index(20) == 1, "azimuth_to_index");
static_assert(test_table_a::azimuth_to_index(-45) == 21, "azimuth_to_index");

static_assert(test_table_a::elevation_to_index(0) == 5, "elevation_to_index");
static_assert(test_table_a::elevation_to_index(5) == 5, "elevation_to_index");
static_assert(test_table_a::elevation_to_index(-5) == 5, "elevation_to_index");
static_assert(test_table_a::elevation_to_index(355) == 5, "elevation_to_index");
static_assert(test_table_a::elevation_to_index(10) == 6, "elevation_to_index");
static_assert(test_table_a::elevation_to_index(15) == 6, "elevation_to_index");
static_assert(test_table_a::elevation_to_index(20) == 6, "elevation_to_index");
static_assert(test_table_a::elevation_to_index(90) == 10, "elevation_to_index");
static_assert(test_table_a::elevation_to_index(-90) == 0, "elevation_to_index");
static_assert(test_table_a::elevation_to_index(270) == 0, "elevation_to_index");

using test_table_b = vector_look_up_table<float, 20, 9>;

static_assert(test_table_b::get_azimuth_angle() == 18, "get_azimuth_angle");
static_assert(test_table_b::get_elevation_angle() == 18, "get_elevation_angle");

static_assert(test_table_b::azimuth_to_index(0) == 0, "azimuth_to_index");
static_assert(test_table_b::azimuth_to_index(8) == 0, "azimuth_to_index");
static_assert(test_table_b::azimuth_to_index(9) == 1, "azimuth_to_index");
static_assert(test_table_b::azimuth_to_index(26) == 1, "azimuth_to_index");
static_assert(test_table_b::azimuth_to_index(27) == 2, "azimuth_to_index");
static_assert(test_table_b::azimuth_to_index(355) == 0, "azimuth_to_index");
static_assert(test_table_b::azimuth_to_index(-5) == 0, "azimuth_to_index");
static_assert(test_table_b::azimuth_to_index(180) == 10, "azimuth_to_index");

static_assert(test_table_b::elevation_to_index(0) == 4, "elevation_to_index");
static_assert(test_table_b::elevation_to_index(-8) == 4, "elevation_to_index");
static_assert(test_table_b::elevation_to_index(8) == 4, "elevation_to_index");
static_assert(test_table_b::elevation_to_index(9) == 5, "elevation_to_index");
static_assert(test_table_b::elevation_to_index(-10) == 3, "elevation_to_index");
static_assert(test_table_b::elevation_to_index(90) == 8, "elevation_to_index");
static_assert(test_table_b::elevation_to_index(270) == 0, "elevation_to_index");
static_assert(test_table_b::elevation_to_index(-90) == 0, "elevation_to_index");

}  // namespace
