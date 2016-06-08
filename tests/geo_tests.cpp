#include "common/geometric.h"

#include "gtest/gtest.h"

TEST(point_tri_distance, point_tri_distance) {
    ASSERT_EQ(geo::point_triangle_distance_squared(
                      TriangleVec3{{glm::vec3(1, 0, 0),
                                    glm::vec3(100, 0, 0),
                                    glm::vec3(100, 100, 0)}},
                      glm::vec3(1, 0, 0)),
              0);

    ASSERT_EQ(geo::point_triangle_distance_squared(
                      TriangleVec3{{glm::vec3(1, 0, 0),
                                    glm::vec3(100, 0, 0),
                                    glm::vec3(100, 100, 0)}},
                      glm::vec3(0, 0, 0)),
              1);

    ASSERT_EQ(geo::point_triangle_distance_squared(
                      TriangleVec3{{glm::vec3(1, 0, 10),
                                    glm::vec3(100, 0, 10),
                                    glm::vec3(100, 100, 10)}},
                      glm::vec3(1, 0, 0)),
              100);
}
