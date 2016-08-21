#include "raytracer/image_source_impl.h"

#include "gtest/gtest.h"

TEST(image_source_impl, compute_unique_paths) {
    struct item final {
        cl_ulong index;
        bool visible;
    };

    ASSERT_EQ(raytracer::compute_unique_paths(
                      aligned::vector<aligned::vector<item>>{}),
              aligned::set<aligned::vector<cl_ulong>>{});

    ASSERT_EQ(raytracer::compute_unique_paths(
                      aligned::vector<aligned::vector<item>>{
                              aligned::vector<item>{}}),
              aligned::set<aligned::vector<cl_ulong>>{});

    ASSERT_EQ(raytracer::compute_unique_paths(
                      aligned::vector<aligned::vector<item>>{
                              aligned::vector<item>{item{0, true}}}),
              aligned::set<aligned::vector<cl_ulong>>{
                      aligned::vector<cl_ulong>{0}});

    ASSERT_EQ(raytracer::compute_unique_paths(
                      aligned::vector<aligned::vector<item>>{
                              aligned::vector<item>{item{0, false}}}),
              aligned::set<aligned::vector<cl_ulong>>{});

    ASSERT_EQ(raytracer::compute_unique_paths(
                      aligned::vector<aligned::vector<item>>{
                              aligned::vector<item>{item{0, true},
                                                    item{1, false},
                                                    item{2, false}}}),
              aligned::set<aligned::vector<cl_ulong>>{
                      aligned::vector<cl_ulong>{0}});

    ASSERT_EQ(raytracer::compute_unique_paths(
                      aligned::vector<aligned::vector<item>>{
                              aligned::vector<item>{item{0, true},
                                                    item{1, true},
                                                    item{2, true}}}),
              (aligned::set<aligned::vector<cl_ulong>>{
                      aligned::vector<cl_ulong>{0},
                      aligned::vector<cl_ulong>{0, 1},
                      aligned::vector<cl_ulong>{0, 1, 2}}));

    ASSERT_EQ(raytracer::compute_unique_paths(
                      aligned::vector<aligned::vector<item>>{
                              aligned::vector<item>{item{0, true},
                                                    item{1, true},
                                                    item{2, true}},
                              aligned::vector<item>{item{3, true},
                                                    item{4, true},
                                                    item{5, true}}}),
              (aligned::set<aligned::vector<cl_ulong>>{
                      aligned::vector<cl_ulong>{0},
                      aligned::vector<cl_ulong>{0, 1},
                      aligned::vector<cl_ulong>{0, 1, 2},
                      aligned::vector<cl_ulong>{3},
                      aligned::vector<cl_ulong>{3, 4},
                      aligned::vector<cl_ulong>{3, 4, 5}}));
}

TEST(image_source_impl, compute_mirrored_triangles) {
    ASSERT_EQ(raytracer::compute_mirrored_triangles(
                      aligned::vector<geo::triangle_vec3>{}),
              aligned::vector<geo::triangle_vec3>{});

    ASSERT_EQ(raytracer::compute_mirrored_triangles(
                      aligned::vector<geo::triangle_vec3>{
                              geo::triangle_vec3{{glm::vec3(0, 0, 0),
                                                  glm::vec3(1, 0, 0),
                                                  glm::vec3(0, 0, 1)}}}),
              (aligned::vector<geo::triangle_vec3>{
                      geo::triangle_vec3{{glm::vec3(0, 0, 0),
                                          glm::vec3(1, 0, 0),
                                          glm::vec3(0, 0, 1)}}}));

    ASSERT_EQ(raytracer::compute_mirrored_triangles(
                      aligned::vector<geo::triangle_vec3>{
                              geo::triangle_vec3{{glm::vec3(0, 0, 0),
                                                  glm::vec3(1, 0, 0),
                                                  glm::vec3(0, 0, 1)}},

                              geo::triangle_vec3{{glm::vec3(0, 2, 0),
                                                  glm::vec3(1, 2, 0),
                                                  glm::vec3(0, 2, 1)}},
                      }),
              (aligned::vector<geo::triangle_vec3>{
                      geo::triangle_vec3{{glm::vec3(0, 0, 0),
                                          glm::vec3(1, 0, 0),
                                          glm::vec3(0, 0, 1)}},

                      geo::triangle_vec3{{glm::vec3(0, -2, 0),
                                          glm::vec3(1, -2, 0),
                                          glm::vec3(0, -2, 1)}},
              }));

    ASSERT_EQ(raytracer::compute_mirrored_triangles(
                      aligned::vector<geo::triangle_vec3>{
                              geo::triangle_vec3{{glm::vec3(0, 0, 0),
                                                  glm::vec3(1, 0, 0),
                                                  glm::vec3(0, 0, 1)}},

                              geo::triangle_vec3{{glm::vec3(0, 2, 0),
                                                  glm::vec3(1, 2, 0),
                                                  glm::vec3(0, 2, 1)}},

                              geo::triangle_vec3{{glm::vec3(0, 0, 0),
                                                  glm::vec3(1, 0, 0),
                                                  glm::vec3(0, 0, 1)}},
                      }),
              (aligned::vector<geo::triangle_vec3>{
                      geo::triangle_vec3{{glm::vec3(0, 0, 0),
                                          glm::vec3(1, 0, 0),
                                          glm::vec3(0, 0, 1)}},

                      geo::triangle_vec3{{glm::vec3(0, -2, 0),
                                          glm::vec3(1, -2, 0),
                                          glm::vec3(0, -2, 1)}},

                      geo::triangle_vec3{{glm::vec3(0, -4, 0),
                                          glm::vec3(1, -4, 0),
                                          glm::vec3(0, -4, 1)}},
              }));
}

TEST(image_source_impl, compute_intersection_distances) {
    ASSERT_EQ(*raytracer::compute_intersection_distances(
                      aligned::vector<geo::triangle_vec3>{},
                      geo::ray{glm::vec3(0, 1, 0), glm::vec3(0, -1, 0)}),
              aligned::vector<float>{});

    ASSERT_EQ(*raytracer::compute_intersection_distances(
                      aligned::vector<geo::triangle_vec3>{
                              geo::triangle_vec3{{glm::vec3(0, 0, 0),
                                                  glm::vec3(1, 0, 0),
                                                  glm::vec3(0, 0, 1)}}},
                      geo::ray{glm::vec3(0, 1, 0), glm::vec3(0, -1, 0)}),
              aligned::vector<float>{1});

    ASSERT_FALSE(raytracer::compute_intersection_distances(
            aligned::vector<geo::triangle_vec3>{
                    geo::triangle_vec3{{glm::vec3(0, 0, 0),
                                        glm::vec3(1, 0, 0),
                                        glm::vec3(0, 0, 1)}}},
            geo::ray{glm::vec3(0, 1, 0), glm::vec3(0, 1, 0)}));
}
