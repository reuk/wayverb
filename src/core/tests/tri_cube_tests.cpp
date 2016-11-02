#include "core/geo/box.h"
#include "core/geo/tri_cube_intersection.h"

#include "utilities/range.h"

#include "tri_cube_c.h"

#include "gtest/gtest.h"

#include <random>

using namespace wayverb::core;

TEST(tri_cube_tests, face_plane) {
    ASSERT_EQ(0x00, geo::face_plane(glm::vec3(0.5, 0.5, 0.5)));
    ASSERT_EQ(0x00, geo::face_plane(glm::vec3(-0.5, -0.5, -0.5)));

    ASSERT_EQ(0x01, geo::face_plane(glm::vec3(1, 0, 0)));
    ASSERT_EQ(0x02, geo::face_plane(glm::vec3(-1, 0, 0)));
    ASSERT_EQ(0x04, geo::face_plane(glm::vec3(0, 1, 0)));
    ASSERT_EQ(0x08, geo::face_plane(glm::vec3(0, -1, 0)));
    ASSERT_EQ(0x10, geo::face_plane(glm::vec3(0, 0, 1)));
    ASSERT_EQ(0x20, geo::face_plane(glm::vec3(0, 0, -1)));

    ASSERT_EQ(0x05, geo::face_plane(glm::vec3(1, 1, 0)));
    ASSERT_EQ(0x09, geo::face_plane(glm::vec3(1, -1, 0)));
    ASSERT_EQ(0x06, geo::face_plane(glm::vec3(-1, 1, 0)));
    ASSERT_EQ(0x0a, geo::face_plane(glm::vec3(-1, -1, 0)));

    ASSERT_EQ(0x14, geo::face_plane(glm::vec3(0, 1, 1)));
    ASSERT_EQ(0x24, geo::face_plane(glm::vec3(0, 1, -1)));
    ASSERT_EQ(0x18, geo::face_plane(glm::vec3(0, -1, 1)));
    ASSERT_EQ(0x28, geo::face_plane(glm::vec3(0, -1, -1)));

    ASSERT_EQ(0x11, geo::face_plane(glm::vec3(1, 0, 1)));
    ASSERT_EQ(0x12, geo::face_plane(glm::vec3(-1, 0, 1)));
    ASSERT_EQ(0x21, geo::face_plane(glm::vec3(1, 0, -1)));
    ASSERT_EQ(0x22, geo::face_plane(glm::vec3(-1, 0, -1)));

    ASSERT_EQ(0x15, geo::face_plane(glm::vec3(1, 1, 1)));
    ASSERT_EQ(0x25, geo::face_plane(glm::vec3(1, 1, -1)));
    ASSERT_EQ(0x19, geo::face_plane(glm::vec3(1, -1, 1)));
    ASSERT_EQ(0x29, geo::face_plane(glm::vec3(1, -1, -1)));
    ASSERT_EQ(0x16, geo::face_plane(glm::vec3(-1, 1, 1)));
    ASSERT_EQ(0x26, geo::face_plane(glm::vec3(-1, 1, -1)));
    ASSERT_EQ(0x1a, geo::face_plane(glm::vec3(-1, -1, 1)));
    ASSERT_EQ(0x2a, geo::face_plane(glm::vec3(-1, -1, -1)));

    std::default_random_engine engine{std::random_device()()};
    std::uniform_real_distribution<float> dist(-10.0f, 10.0f);

    for (auto i = 0u; i != 100; ++i) {
        const auto x = dist(engine);
        const auto y = dist(engine);
        const auto z = dist(engine);
        ASSERT_EQ(face_plane(Point3{x, y, z}),
                  geo::face_plane(glm::vec3(x, y, z)));
    }
}

TEST(tri_cube_tests, bevel_2d) {
    ASSERT_EQ(0x000, geo::bevel_2d(glm::vec3(0.5, 0.5, 0.5)));
    ASSERT_EQ(0x000, geo::bevel_2d(glm::vec3(-0.5, -0.5, -0.5)));

    ASSERT_EQ(0x000, geo::bevel_2d(glm::vec3(1, 0, 0)));
    ASSERT_EQ(0x000, geo::bevel_2d(glm::vec3(-1, 0, 0)));
    ASSERT_EQ(0x000, geo::bevel_2d(glm::vec3(0, 1, 0)));
    ASSERT_EQ(0x000, geo::bevel_2d(glm::vec3(0, -1, 0)));
    ASSERT_EQ(0x000, geo::bevel_2d(glm::vec3(0, 0, 1)));
    ASSERT_EQ(0x000, geo::bevel_2d(glm::vec3(0, 0, -1)));

    ASSERT_EQ(0x001, geo::bevel_2d(glm::vec3(1, 1, 0)));
    ASSERT_EQ(0x002, geo::bevel_2d(glm::vec3(1, -1, 0)));
    ASSERT_EQ(0x004, geo::bevel_2d(glm::vec3(-1, 1, 0)));
    ASSERT_EQ(0x008, geo::bevel_2d(glm::vec3(-1, -1, 0)));

    ASSERT_EQ(0x100, geo::bevel_2d(glm::vec3(0, 1, 1)));
    ASSERT_EQ(0x200, geo::bevel_2d(glm::vec3(0, 1, -1)));
    ASSERT_EQ(0x400, geo::bevel_2d(glm::vec3(0, -1, 1)));
    ASSERT_EQ(0x800, geo::bevel_2d(glm::vec3(0, -1, -1)));

    ASSERT_EQ(0x010, geo::bevel_2d(glm::vec3(1, 0, 1)));
    ASSERT_EQ(0x040, geo::bevel_2d(glm::vec3(-1, 0, 1)));
    ASSERT_EQ(0x020, geo::bevel_2d(glm::vec3(1, 0, -1)));
    ASSERT_EQ(0x080, geo::bevel_2d(glm::vec3(-1, 0, -1)));

    ASSERT_EQ(0x111, geo::bevel_2d(glm::vec3(1, 1, 1)));
    ASSERT_EQ(0x221, geo::bevel_2d(glm::vec3(1, 1, -1)));
    ASSERT_EQ(0x412, geo::bevel_2d(glm::vec3(1, -1, 1)));
    ASSERT_EQ(0x822, geo::bevel_2d(glm::vec3(1, -1, -1)));
    ASSERT_EQ(0x144, geo::bevel_2d(glm::vec3(-1, 1, 1)));
    ASSERT_EQ(0x284, geo::bevel_2d(glm::vec3(-1, 1, -1)));
    ASSERT_EQ(0x448, geo::bevel_2d(glm::vec3(-1, -1, 1)));
    ASSERT_EQ(0x888, geo::bevel_2d(glm::vec3(-1, -1, -1)));

    std::default_random_engine engine{std::random_device()()};
    std::uniform_real_distribution<float> dist(-10.0f, 10.0f);

    for (auto i = 0u; i != 100; ++i) {
        auto x = dist(engine);
        auto y = dist(engine);
        auto z = dist(engine);
        ASSERT_EQ(bevel_2d(Point3{x, y, z}), geo::bevel_2d(glm::vec3(x, y, z)));
    }
}

TEST(tri_cube_tests, bevel_3d) {
    ASSERT_EQ(0x00, geo::bevel_3d(glm::vec3(0.5, 0.5, 0.5)));
    ASSERT_EQ(0x00, geo::bevel_3d(glm::vec3(-0.5, -0.5, -0.5)));

    ASSERT_EQ(0x00, geo::bevel_3d(glm::vec3(1, 0, 0)));
    ASSERT_EQ(0x00, geo::bevel_3d(glm::vec3(-1, 0, 0)));
    ASSERT_EQ(0x00, geo::bevel_3d(glm::vec3(0, 1, 0)));
    ASSERT_EQ(0x00, geo::bevel_3d(glm::vec3(0, -1, 0)));
    ASSERT_EQ(0x00, geo::bevel_3d(glm::vec3(0, 0, 1)));
    ASSERT_EQ(0x00, geo::bevel_3d(glm::vec3(0, 0, -1)));

    ASSERT_EQ(0x03, geo::bevel_3d(glm::vec3(1, 1, 0)));
    ASSERT_EQ(0x0c, geo::bevel_3d(glm::vec3(1, -1, 0)));
    ASSERT_EQ(0x30, geo::bevel_3d(glm::vec3(-1, 1, 0)));
    ASSERT_EQ(0xc0, geo::bevel_3d(glm::vec3(-1, -1, 0)));

    ASSERT_EQ(0x11, geo::bevel_3d(glm::vec3(0, 1, 1)));
    ASSERT_EQ(0x22, geo::bevel_3d(glm::vec3(0, 1, -1)));
    ASSERT_EQ(0x44, geo::bevel_3d(glm::vec3(0, -1, 1)));
    ASSERT_EQ(0x88, geo::bevel_3d(glm::vec3(0, -1, -1)));

    ASSERT_EQ(0x05, geo::bevel_3d(glm::vec3(1, 0, 1)));
    ASSERT_EQ(0x50, geo::bevel_3d(glm::vec3(-1, 0, 1)));
    ASSERT_EQ(0x0a, geo::bevel_3d(glm::vec3(1, 0, -1)));
    ASSERT_EQ(0xa0, geo::bevel_3d(glm::vec3(-1, 0, -1)));

    ASSERT_EQ(0x01, geo::bevel_3d(glm::vec3(1, 1, 1)));
    ASSERT_EQ(0x02, geo::bevel_3d(glm::vec3(1, 1, -1)));
    ASSERT_EQ(0x04, geo::bevel_3d(glm::vec3(1, -1, 1)));
    ASSERT_EQ(0x08, geo::bevel_3d(glm::vec3(1, -1, -1)));
    ASSERT_EQ(0x10, geo::bevel_3d(glm::vec3(-1, 1, 1)));
    ASSERT_EQ(0x20, geo::bevel_3d(glm::vec3(-1, 1, -1)));
    ASSERT_EQ(0x40, geo::bevel_3d(glm::vec3(-1, -1, 1)));
    ASSERT_EQ(0x80, geo::bevel_3d(glm::vec3(-1, -1, -1)));

    std::default_random_engine engine{std::random_device()()};
    std::uniform_real_distribution<float> dist(-10.0f, 10.0f);

    for (auto i = 0u; i != 100; ++i) {
        auto x = dist(engine);
        auto y = dist(engine);
        auto z = dist(engine);
        ASSERT_EQ(bevel_3d(Point3{x, y, z}), geo::bevel_3d(glm::vec3(x, y, z)));
    }
}

TEST(tri_cube_tests, check_line) {
    ASSERT_EQ(geo::where::inside,
              geo::check_line(glm::vec3(-1, 0, 0), glm::vec3(1, 0, 0), 0x01));
    ASSERT_EQ(geo::where::inside,
              geo::check_line(glm::vec3(-1, 0, 0), glm::vec3(1, 0, 0), 0x02));
    ASSERT_EQ(geo::where::outside,
              geo::check_line(glm::vec3(-1, 0, 0), glm::vec3(1, 0, 0), 0x04));
    ASSERT_EQ(geo::where::outside,
              geo::check_line(glm::vec3(-1, 0, 0), glm::vec3(1, 0, 0), 0x08));
    ASSERT_EQ(geo::where::outside,
              geo::check_line(glm::vec3(-1, 0, 0), glm::vec3(1, 0, 0), 0x10));
    ASSERT_EQ(geo::where::outside,
              geo::check_line(glm::vec3(-1, 0, 0), glm::vec3(1, 0, 0), 0x20));

    ASSERT_EQ(geo::where::outside,
              geo::check_line(glm::vec3(0, -1, 0), glm::vec3(0, 1, 0), 0x01));
    ASSERT_EQ(geo::where::outside,
              geo::check_line(glm::vec3(0, -1, 0), glm::vec3(0, 1, 0), 0x02));
    ASSERT_EQ(geo::where::inside,
              geo::check_line(glm::vec3(0, -1, 0), glm::vec3(0, 1, 0), 0x04));
    ASSERT_EQ(geo::where::inside,
              geo::check_line(glm::vec3(0, -1, 0), glm::vec3(0, 1, 0), 0x08));
    ASSERT_EQ(geo::where::outside,
              geo::check_line(glm::vec3(0, -1, 0), glm::vec3(0, 1, 0), 0x10));
    ASSERT_EQ(geo::where::outside,
              geo::check_line(glm::vec3(0, -1, 0), glm::vec3(0, 1, 0), 0x20));

    ASSERT_EQ(geo::where::outside,
              geo::check_line(glm::vec3(0, 0, -1), glm::vec3(0, 0, 1), 0x01));
    ASSERT_EQ(geo::where::outside,
              geo::check_line(glm::vec3(0, 0, -1), glm::vec3(0, 0, 1), 0x02));
    ASSERT_EQ(geo::where::outside,
              geo::check_line(glm::vec3(0, 0, -1), glm::vec3(0, 0, 1), 0x04));
    ASSERT_EQ(geo::where::outside,
              geo::check_line(glm::vec3(0, 0, -1), glm::vec3(0, 0, 1), 0x08));
    ASSERT_EQ(geo::where::inside,
              geo::check_line(glm::vec3(0, 0, -1), glm::vec3(0, 0, 1), 0x10));
    ASSERT_EQ(geo::where::inside,
              geo::check_line(glm::vec3(0, 0, -1), glm::vec3(0, 0, 1), 0x20));

    std::default_random_engine engine{std::random_device()()};
    std::uniform_real_distribution<float> dist(-10.0f, 10.0f);

    for (auto i = 0u; i != 100; ++i) {
        auto x0 = dist(engine);
        auto y0 = dist(engine);
        auto z0 = dist(engine);
        auto x1 = dist(engine);
        auto y1 = dist(engine);
        auto z1 = dist(engine);
        for (auto diff = 0u; diff != 0x100; ++diff)
            ASSERT_EQ(check_line(Point3{x0, y0, z0}, Point3{x1, y1, z1}, diff),
                      geo::where::outside ==
                              geo::check_line(glm::vec3(x0, y0, z0),
                                              glm::vec3(x1, y1, z1),
                                              diff));
    }
}

TEST(tri_cube_tests, point_triangle_intersection) {
    ASSERT_EQ(geo::where::inside,
              geo::point_triangle_intersection(
                      glm::vec3(0, 0, 0),
                      geo::triangle_vec3({{glm::vec3(-2, -1, 0),
                                           glm::vec3(1, -1, 0),
                                           glm::vec3(2, 1, 0)}})));

    ASSERT_EQ(geo::where::outside,
              geo::point_triangle_intersection(
                      glm::vec3(4, 4, 4),
                      geo::triangle_vec3({{glm::vec3(-2, -1, 0),
                                           glm::vec3(1, -1, 0),
                                           glm::vec3(2, 1, 0)}})));

    ASSERT_EQ(point_triangle_intersection(Point3{0.25, 0.25, 0.25},
                                          Triangle3{Point3{-0.25, -10, -5},
                                                    Point3{-0.25, 5, -5},
                                                    Point3{-0.25, 5, 10}}),
              geo::where::outside ==
                      geo::point_triangle_intersection(
                              glm::vec3(0.25, 0.25, 0.25),
                              geo::triangle_vec3{{glm::vec3(-0.25, -10, -5),
                                                  glm::vec3(-0.25, 5, -5),
                                                  glm::vec3(-0.25, 5, 10)}}));

    std::default_random_engine engine{std::random_device()()};
    std::uniform_real_distribution<float> dist(-10.0f, 10.0f);

    for (auto i = 0u; i != 100; ++i) {
        auto x0 = dist(engine);
        auto y0 = dist(engine);
        auto z0 = dist(engine);
        auto x1 = dist(engine);
        auto y1 = dist(engine);
        auto z1 = dist(engine);
        auto x2 = dist(engine);
        auto y2 = dist(engine);
        auto z2 = dist(engine);
        auto x3 = dist(engine);
        auto y3 = dist(engine);
        auto z3 = dist(engine);
        ASSERT_EQ(point_triangle_intersection(Point3{x0, y0, z0},
                                              Triangle3{Point3{x1, y1, z1},
                                                        Point3{x2, y2, z2},
                                                        Point3{x3, y3, z3}}),
                  geo::where::outside ==
                          geo::point_triangle_intersection(
                                  glm::vec3(x0, y0, z0),
                                  geo::triangle_vec3{{glm::vec3(x1, y1, z1),
                                                      glm::vec3(x2, y2, z2),
                                                      glm::vec3(x3, y3, z3)}}));
    }
}

TEST(tri_cube_tests, old) {
    auto edge = 0.5f;
    ASSERT_EQ(INSIDE,
              t_c_intersection(Triangle3{Point3{-10, -5, edge},
                                         Point3{5, -5, edge},
                                         Point3{5, 10, edge}}));
    ASSERT_EQ(INSIDE,
              t_c_intersection(Triangle3{
                      Point3{-10, -5, 0}, Point3{5, -5, 0}, Point3{5, 10, 0}}));
    ASSERT_EQ(INSIDE,
              t_c_intersection(Triangle3{Point3{-10, -5, -edge},
                                         Point3{5, -5, -edge},
                                         Point3{5, 10, -edge}}));

    ASSERT_EQ(INSIDE,
              t_c_intersection(Triangle3{Point3{-10, edge, -5},
                                         Point3{5, edge, -5},
                                         Point3{5, edge, 10}}));
    ASSERT_EQ(INSIDE,
              t_c_intersection(Triangle3{
                      Point3{-10, 0, -5}, Point3{5, 0, -5}, Point3{5, 0, 10}}));
    ASSERT_EQ(INSIDE,
              t_c_intersection(Triangle3{Point3{-10, -edge, -5},
                                         Point3{5, -edge, -5},
                                         Point3{5, -edge, 10}}));

    ASSERT_EQ(INSIDE,
              t_c_intersection(Triangle3{Point3{edge, -10, -5},
                                         Point3{edge, 5, -5},
                                         Point3{edge, 5, 10}}));
    ASSERT_EQ(INSIDE,
              t_c_intersection(Triangle3{
                      Point3{0, -10, -5}, Point3{0, 5, -5}, Point3{0, 5, 10}}));
    ASSERT_EQ(INSIDE,
              t_c_intersection(Triangle3{Point3{-0.25, -10, -5},
                                         Point3{-0.25, 5, -5},
                                         Point3{-0.25, 5, 10}}));
    ASSERT_EQ(INSIDE,
              t_c_intersection(Triangle3{Point3{-edge, -10, -5},
                                         Point3{-edge, 5, -5},
                                         Point3{-edge, 5, 10}}));

    auto outside = 1.0f;
    ASSERT_EQ(OUTSIDE,
              t_c_intersection(Triangle3{Point3{-10, -5, outside},
                                         Point3{5, -5, outside},
                                         Point3{5, 10, outside}}));
    ASSERT_EQ(OUTSIDE,
              t_c_intersection(Triangle3{Point3{-10, -5, -outside},
                                         Point3{5, -5, -outside},
                                         Point3{5, 10, -outside}}));
    ASSERT_EQ(OUTSIDE,
              t_c_intersection(Triangle3{Point3{-10, outside, -5},
                                         Point3{5, outside, -5},
                                         Point3{5, outside, 10}}));
    ASSERT_EQ(OUTSIDE,
              t_c_intersection(Triangle3{Point3{-10, -outside, -5},
                                         Point3{5, -outside, -5},
                                         Point3{5, -outside, 10}}));
    ASSERT_EQ(OUTSIDE,
              t_c_intersection(Triangle3{Point3{outside, -10, -5},
                                         Point3{outside, 5, -5},
                                         Point3{outside, 5, 10}}));
    ASSERT_EQ(OUTSIDE,
              t_c_intersection(Triangle3{Point3{-outside, -10, -5},
                                         Point3{-outside, 5, -5},
                                         Point3{-outside, 5, 10}}));
}

TEST(tri_cube_tests, specific) {
    auto edge = 0.5;
    ASSERT_EQ(geo::where::inside,
              geo::t_c_intersection(
                      geo::triangle_vec3({{glm::vec3(-10, -5, edge),
                                           glm::vec3(5, -5, edge),
                                           glm::vec3(5, 10, edge)}})));
    ASSERT_EQ(
            geo::where::inside,
            geo::t_c_intersection(geo::triangle_vec3({{glm::vec3(-10, -5, 0),
                                                       glm::vec3(5, -5, 0),
                                                       glm::vec3(5, 10, 0)}})));
    ASSERT_EQ(geo::where::inside,
              geo::t_c_intersection(
                      geo::triangle_vec3({{glm::vec3(-10, -5, -edge),
                                           glm::vec3(5, -5, -edge),
                                           glm::vec3(5, 10, -edge)}})));

    ASSERT_EQ(geo::where::inside,
              geo::t_c_intersection(
                      geo::triangle_vec3({{glm::vec3(-10, edge, -5),
                                           glm::vec3(5, edge, -5),
                                           glm::vec3(5, edge, 10)}})));
    ASSERT_EQ(
            geo::where::inside,
            geo::t_c_intersection(geo::triangle_vec3({{glm::vec3(-10, 0, -5),
                                                       glm::vec3(5, 0, -5),
                                                       glm::vec3(5, 0, 10)}})));
    ASSERT_EQ(geo::where::inside,
              geo::t_c_intersection(
                      geo::triangle_vec3({{glm::vec3(-10, -edge, -5),
                                           glm::vec3(5, -edge, -5),
                                           glm::vec3(5, -edge, 10)}})));

    ASSERT_EQ(geo::where::inside,
              geo::t_c_intersection(
                      geo::triangle_vec3({{glm::vec3(edge, -10, -5),
                                           glm::vec3(edge, 5, -5),
                                           glm::vec3(edge, 5, 10)}})));
    ASSERT_EQ(
            geo::where::inside,
            geo::t_c_intersection(geo::triangle_vec3({{glm::vec3(0, -10, -5),
                                                       glm::vec3(0, 5, -5),
                                                       glm::vec3(0, 5, 10)}})));
    ASSERT_EQ(geo::where::inside,
              geo::t_c_intersection(
                      geo::triangle_vec3({{glm::vec3(-0.25, -10, -5),
                                           glm::vec3(-0.25, 5, -5),
                                           glm::vec3(-0.25, 5, 10)}})));
    ASSERT_EQ(geo::where::inside,
              geo::t_c_intersection(
                      geo::triangle_vec3({{glm::vec3(-edge, -10, -5),
                                           glm::vec3(-edge, 5, -5),
                                           glm::vec3(-edge, 5, 10)}})));

    auto outside = 1;
    ASSERT_EQ(geo::where::outside,
              geo::t_c_intersection(
                      geo::triangle_vec3({{glm::vec3(-10, -5, outside),
                                           glm::vec3(5, -5, outside),
                                           glm::vec3(5, 10, outside)}})));
    ASSERT_EQ(geo::where::outside,
              geo::t_c_intersection(
                      geo::triangle_vec3({{glm::vec3(-10, -5, -outside),
                                           glm::vec3(5, -5, -outside),
                                           glm::vec3(5, 10, -outside)}})));
    ASSERT_EQ(geo::where::outside,
              geo::t_c_intersection(
                      geo::triangle_vec3({{glm::vec3(-10, outside, -5),
                                           glm::vec3(5, outside, -5),
                                           glm::vec3(5, outside, 10)}})));
    ASSERT_EQ(geo::where::outside,
              geo::t_c_intersection(
                      geo::triangle_vec3({{glm::vec3(-10, -outside, -5),
                                           glm::vec3(5, -outside, -5),
                                           glm::vec3(5, -outside, 10)}})));
    ASSERT_EQ(geo::where::outside,
              geo::t_c_intersection(
                      geo::triangle_vec3({{glm::vec3(outside, -10, -5),
                                           glm::vec3(outside, 5, -5),
                                           glm::vec3(outside, 5, 10)}})));
    ASSERT_EQ(geo::where::outside,
              geo::t_c_intersection(
                      geo::triangle_vec3({{glm::vec3(-outside, -10, -5),
                                           glm::vec3(-outside, 5, -5),
                                           glm::vec3(-outside, 5, 10)}})));

    {
        const geo::triangle_vec3 triangle_verts_0(
                {{glm::vec3(-1.5, 3.0999999, -7.5999999),
                  glm::vec3(0, 3.0999999, -9.10000038),
                  glm::vec3(
                          0.00000000000000144381996, 3.5999999, -7.5999999)}});

        const geo::box b0(glm::vec3(-1.78125, 3.14999986, -8.55000019),
                          glm::vec3(-1.1875, 3.26249981, -8.0749998));
        const geo::box b1(glm::vec3(-1.78125, 3.14999986, -8.0749998),
                          glm::vec3(-1.1875, 3.26249981, -7.5999999));

        const geo::box b2(
                glm::vec3(-9.5, -0.0000000000000021191102, -7.5999999),
                glm::vec3(-7.125, 0.449999988, -5.69999981));
        const auto cent = centre(b2);
        const auto x0 = b2.get_min().x;
        //      auto y0 = b2.get_min().y;
        const auto z0 = b2.get_min().z;
        const auto xc = cent.x;
        const auto yc = cent.y;
        const auto zc = cent.z;
        //      auto x1 = b2.get_max().x;
        const auto y1 = b2.get_max().y;
        //      auto z1 = b2.get_max().z;
        const geo::box b3(glm::vec3(x0, yc, z0), glm::vec3(xc, y1, zc));

        const geo::triangle_vec3 triangle_verts_1(
                {{glm::vec3(-9.5, 1.10000002, -6.0999999),
                  glm::vec3(-9.5, 0.100000001, -6.0999999),
                  glm::vec3(-9.5, 0.100000001, -9)}});

        ASSERT_EQ(false, geo::overlaps(b0, triangle_verts_0));
        ASSERT_EQ(true, geo::overlaps(b1, triangle_verts_0));

        ASSERT_EQ(true, geo::overlaps(b2, triangle_verts_1));
        ASSERT_EQ(true, geo::overlaps(b3, triangle_verts_1));
    }

    {
        //        ASSERT_EQ(INSIDE,
        //        t_c_intersection(Triangle3{Point3{-5.49999523, 0.184210628,
        //        4.60390043},
        //            Point3{0.833333611, 0.184210628, 4.60390043},
        //            Point3{0.833333611, 0.184210628, -3.29220581}}));

        const geo::box aabb(glm::vec3(-4.20000029, -0.040625006, -5.02812529),
                            glm::vec3(-3.9000001, 0.0187499933, -4.78750038));

        const geo::triangle_vec3 verts{
                {glm::vec3(-5.69999981, 0, -3.79999995),
                 glm::vec3(-3.79999995, 0, -3.79999995),
                 glm::vec3(-3.79999995, 0, -5.69999981)}};

        ASSERT_EQ(true, geo::overlaps(aabb, verts));
    }

    {
        const geo::box aabb(glm::vec3(-7.80000019, 3.46249986, -8.63750076),
                            glm::vec3(-7.20000029, 3.58124971, -8.15625));

        const geo::triangle_vec3 v0{
                {glm::vec3(-7.5999999, 3.5999999, -7.5999999),
                 glm::vec3(-7.5999999, 3.0999999, -9.10000038),
                 glm::vec3(-6.0999999, 3.0999999, -7.5999999)}};

        const geo::triangle_vec3 v1{
                {glm::vec3(-9.10000038, 3.0999999, -7.5999999),
                 glm::vec3(-7.5999999, 3.0999999, -9.10000038),
                 glm::vec3(-7.5999999, 3.5999999, -7.5999999)}};

        ASSERT_EQ(false, geo::overlaps(aabb, v0));
        ASSERT_EQ(false, geo::overlaps(aabb, v1));
    }
}

TEST(tri_cube_tests, comparison) {
    std::default_random_engine engine{std::random_device{}()};
    std::uniform_real_distribution<float> dist{-10, 10};

    for (auto i{0ul}; i != 1 << 20; ++i) {
        const auto x0{dist(engine)};
        const auto y0{dist(engine)};
        const auto z0{dist(engine)};

        const auto x1{dist(engine)};
        const auto y1{dist(engine)};
        const auto z1{dist(engine)};

        const auto x2{dist(engine)};
        const auto y2{dist(engine)};
        const auto z2{dist(engine)};

        ASSERT_EQ(geo::overlaps(geo::box{glm::vec3{-0.5}, glm::vec3{0.5}},
                                geo::triangle_vec3{{glm::vec3{x0, y0, z0},
                                                    glm::vec3{x1, y1, z1},
                                                    glm::vec3{x2, y2, z2}}}),
                  t_c_intersection(Triangle3{Point3{x0, y0, z0},
                                             Point3{x1, y1, z1},
                                             Point3{x2, y2, z2}}) == INSIDE)
                << i;
    }
}
