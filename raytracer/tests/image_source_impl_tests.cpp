#include "raytracer/image_source_tree.h"

#include "gtest/gtest.h"

TEST(image_source_impl, compute_mirrored_triangle) {
    const geo::triangle_vec3 tri{
            {glm::vec3{0, 0, 0}, glm::vec3{1, 2, 3}, glm::vec3{6, 6, 6}}};
    {
        const aligned::vector<geo::triangle_vec3> vec{};
        ASSERT_EQ(raytracer::compute_mirrored_triangle(
                          vec.begin(), vec.end(), tri),
                  tri);
    }
    {
        const aligned::vector<geo::triangle_vec3> vec{geo::triangle_vec3{
                {glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1)}}};
        ASSERT_EQ(raytracer::compute_mirrored_triangle(
                          vec.begin(), vec.end(), tri),
                  (geo::triangle_vec3{{glm::vec3{0, 0, 0},
                                       glm::vec3{1, -2, 3},
                                       glm::vec3{6, -6, 6}}}));
    }
    {
        const aligned::vector<geo::triangle_vec3> vec{
                geo::triangle_vec3{{glm::vec3(0, 0, 0),
                                    glm::vec3(1, 0, 0),
                                    glm::vec3(0, 0, 1)}},
                geo::triangle_vec3{{glm::vec3(0, 2, 0),
                                    glm::vec3(1, 2, 0),
                                    glm::vec3(0, 2, 1)}}};

        ASSERT_EQ(raytracer::compute_mirrored_triangle(
                          vec.begin(), vec.end(), tri),
                  (geo::triangle_vec3{{glm::vec3(0, 4, 0),
                                       glm::vec3(1, 6, 3),
                                       glm::vec3(6, 10, 6)}}));
    }
}

TEST(image_source_impl, compute_intersection_distances) {
    {
        const aligned::vector<geo::triangle_vec3> vec{};
        ASSERT_EQ(*raytracer::compute_intersection_distances(
                          vec.begin(),
                          vec.end(),
                          geo::ray{glm::vec3(0, 1, 0), glm::vec3(0, -1, 0)}),
                  aligned::vector<float>{});
    }
    {
        const aligned::vector<geo::triangle_vec3> vec{geo::triangle_vec3{
                {glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1)}}};
        ASSERT_EQ(*raytracer::compute_intersection_distances(
                          vec.begin(),
                          vec.end(),
                          geo::ray{glm::vec3(0, 1, 0), glm::vec3(0, -1, 0)}),
                  aligned::vector<float>{1});
    }
    {
        const aligned::vector<geo::triangle_vec3> vec{geo::triangle_vec3{
                {glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1)}}};
        ASSERT_FALSE(raytracer::compute_intersection_distances(
                vec.begin(),
                vec.end(),
                geo::ray{glm::vec3(0, 1, 0), glm::vec3(0, 1, 0)}));
    }
}
