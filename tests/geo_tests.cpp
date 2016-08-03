#include "common/geometric.h"

#include "gtest/gtest.h"

TEST(geo, point_tri_distance) {
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

TEST(geo, normal) {
    ASSERT_EQ(geo::normal(TriangleVec3{{glm::vec3(0, 0, 0),
                                        glm::vec3(0, 1, 0),
                                        glm::vec3(0, 0, 1)}}),
              glm::vec3(1, 0, 0));

    ASSERT_EQ(geo::normal(TriangleVec3{{glm::vec3(0, 0, 0),
                                        glm::vec3(0, 0, 1),
                                        glm::vec3(1, 0, 0)}}),
              glm::vec3(0, 1, 0));

    ASSERT_EQ(geo::normal(TriangleVec3{{glm::vec3(0, 0, 0),
                                        glm::vec3(1, 0, 0),
                                        glm::vec3(0, 1, 0)}}),
              glm::vec3(0, 0, 1));

    ASSERT_EQ(geo::normal(TriangleVec3{{glm::vec3(0, 0, 0),
                                        glm::vec3(0, 0, 1),
                                        glm::vec3(0, 1, 0)}}),
              glm::vec3(-1, 0, 0));

    ASSERT_EQ(geo::normal(TriangleVec3{{glm::vec3(0, 0, 0),
                                        glm::vec3(1, 0, 0),
                                        glm::vec3(0, 0, 1)}}),
              glm::vec3(0, -1, 0));

    ASSERT_EQ(geo::normal(TriangleVec3{{glm::vec3(0, 0, 0),
                                        glm::vec3(0, 1, 0),
                                        glm::vec3(1, 0, 0)}}),
              glm::vec3(0, 0, -1));

    ASSERT_EQ(geo::normal(TriangleVec3{{glm::vec3(-1, 0, 1),
                                        glm::vec3(0, 1, 0),
                                        glm::vec3(1, 0, -1)}}),
              glm::normalize(glm::vec3(-1, 0, -1)));
}

void assert_near(const glm::vec3& a, const glm::vec3& b, float epsilon) {
    ASSERT_TRUE(glm::distance(a, b) < epsilon);
}

TEST(geo, mirror) {
    ASSERT_EQ(geo::mirror(glm::vec3(-100, 0, 0),
                          TriangleVec3{{glm::vec3(0, 0, 0),
                                        glm::vec3(0, 1, 0),
                                        glm::vec3(0, 0, 1)}}),
              glm::vec3(100, 0, 0));
    ASSERT_EQ(geo::mirror(glm::vec3(-100, 0, 0),
                          TriangleVec3{{glm::vec3(0, 0, 0),
                                        glm::vec3(0, 0, 1),
                                        glm::vec3(0, 1, 0)}}),
              glm::vec3(100, 0, 0));

    assert_near(geo::mirror(glm::vec3(-1, 0, -1),
                            TriangleVec3{{glm::vec3(-1, 0, 1),
                                          glm::vec3(0, 1, 0),
                                          glm::vec3(1, 0, -1)}}),
                glm::vec3(1, 0, 1),
                0.0001);
}