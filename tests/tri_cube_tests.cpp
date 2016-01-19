#include "helper.h"
#include "tri_cube_intersection.h"
#include "boundaries.h"

#include "tri_cube_c.h"

#include "gtest/gtest.h"

#include <random>

TEST(face_plane, face_plane) {
    ASSERT_EQ(0x00, face_plane(Vec3f(0.5, 0.5, 0.5)));
    ASSERT_EQ(0x00, face_plane(Vec3f(-0.5, -0.5, -0.5)));

    ASSERT_EQ(0x01, face_plane(Vec3f(1, 0, 0)));
    ASSERT_EQ(0x02, face_plane(Vec3f(-1, 0, 0)));
    ASSERT_EQ(0x04, face_plane(Vec3f(0, 1, 0)));
    ASSERT_EQ(0x08, face_plane(Vec3f(0, -1, 0)));
    ASSERT_EQ(0x10, face_plane(Vec3f(0, 0, 1)));
    ASSERT_EQ(0x20, face_plane(Vec3f(0, 0, -1)));

    ASSERT_EQ(0x05, face_plane(Vec3f(1, 1, 0)));
    ASSERT_EQ(0x09, face_plane(Vec3f(1, -1, 0)));
    ASSERT_EQ(0x06, face_plane(Vec3f(-1, 1, 0)));
    ASSERT_EQ(0x0a, face_plane(Vec3f(-1, -1, 0)));

    ASSERT_EQ(0x14, face_plane(Vec3f(0, 1, 1)));
    ASSERT_EQ(0x24, face_plane(Vec3f(0, 1, -1)));
    ASSERT_EQ(0x18, face_plane(Vec3f(0, -1, 1)));
    ASSERT_EQ(0x28, face_plane(Vec3f(0, -1, -1)));

    ASSERT_EQ(0x11, face_plane(Vec3f(1, 0, 1)));
    ASSERT_EQ(0x12, face_plane(Vec3f(-1, 0, 1)));
    ASSERT_EQ(0x21, face_plane(Vec3f(1, 0, -1)));
    ASSERT_EQ(0x22, face_plane(Vec3f(-1, 0, -1)));

    ASSERT_EQ(0x15, face_plane(Vec3f(1, 1, 1)));
    ASSERT_EQ(0x25, face_plane(Vec3f(1, 1, -1)));
    ASSERT_EQ(0x19, face_plane(Vec3f(1, -1, 1)));
    ASSERT_EQ(0x29, face_plane(Vec3f(1, -1, -1)));
    ASSERT_EQ(0x16, face_plane(Vec3f(-1, 1, 1)));
    ASSERT_EQ(0x26, face_plane(Vec3f(-1, 1, -1)));
    ASSERT_EQ(0x1a, face_plane(Vec3f(-1, -1, 1)));
    ASSERT_EQ(0x2a, face_plane(Vec3f(-1, -1, -1)));

    std::default_random_engine engine{std::random_device()()};
    std::uniform_real_distribution<float> dist(-10.0f, 10.0f);

    for (auto i = 0u; i != 100; ++i) {
        auto x = dist(engine);
        auto y = dist(engine);
        auto z = dist(engine);
        ASSERT_EQ(face_plane(Point3{x, y, z}), face_plane(Vec3f(x, y, z)));
    }
}

TEST(bevel_2d, bevel_2d) {
    ASSERT_EQ(0x000, bevel_2d(Vec3f(0.5, 0.5, 0.5)));
    ASSERT_EQ(0x000, bevel_2d(Vec3f(-0.5, -0.5, -0.5)));

    ASSERT_EQ(0x000, bevel_2d(Vec3f(1, 0, 0)));
    ASSERT_EQ(0x000, bevel_2d(Vec3f(-1, 0, 0)));
    ASSERT_EQ(0x000, bevel_2d(Vec3f(0, 1, 0)));
    ASSERT_EQ(0x000, bevel_2d(Vec3f(0, -1, 0)));
    ASSERT_EQ(0x000, bevel_2d(Vec3f(0, 0, 1)));
    ASSERT_EQ(0x000, bevel_2d(Vec3f(0, 0, -1)));

    ASSERT_EQ(0x001, bevel_2d(Vec3f(1, 1, 0)));
    ASSERT_EQ(0x002, bevel_2d(Vec3f(1, -1, 0)));
    ASSERT_EQ(0x004, bevel_2d(Vec3f(-1, 1, 0)));
    ASSERT_EQ(0x008, bevel_2d(Vec3f(-1, -1, 0)));

    ASSERT_EQ(0x100, bevel_2d(Vec3f(0, 1, 1)));
    ASSERT_EQ(0x200, bevel_2d(Vec3f(0, 1, -1)));
    ASSERT_EQ(0x400, bevel_2d(Vec3f(0, -1, 1)));
    ASSERT_EQ(0x800, bevel_2d(Vec3f(0, -1, -1)));

    ASSERT_EQ(0x010, bevel_2d(Vec3f(1, 0, 1)));
    ASSERT_EQ(0x040, bevel_2d(Vec3f(-1, 0, 1)));
    ASSERT_EQ(0x020, bevel_2d(Vec3f(1, 0, -1)));
    ASSERT_EQ(0x080, bevel_2d(Vec3f(-1, 0, -1)));

    ASSERT_EQ(0x111, bevel_2d(Vec3f(1, 1, 1)));
    ASSERT_EQ(0x221, bevel_2d(Vec3f(1, 1, -1)));
    ASSERT_EQ(0x412, bevel_2d(Vec3f(1, -1, 1)));
    ASSERT_EQ(0x822, bevel_2d(Vec3f(1, -1, -1)));
    ASSERT_EQ(0x144, bevel_2d(Vec3f(-1, 1, 1)));
    ASSERT_EQ(0x284, bevel_2d(Vec3f(-1, 1, -1)));
    ASSERT_EQ(0x448, bevel_2d(Vec3f(-1, -1, 1)));
    ASSERT_EQ(0x888, bevel_2d(Vec3f(-1, -1, -1)));

    std::default_random_engine engine{std::random_device()()};
    std::uniform_real_distribution<float> dist(-10.0f, 10.0f);

    for (auto i = 0u; i != 100; ++i) {
        auto x = dist(engine);
        auto y = dist(engine);
        auto z = dist(engine);
        ASSERT_EQ(bevel_2d(Point3{x, y, z}), bevel_2d(Vec3f(x, y, z)));
    }
}

TEST(bevel_3d, bevel_3d) {
    ASSERT_EQ(0x00, bevel_3d(Vec3f(0.5, 0.5, 0.5)));
    ASSERT_EQ(0x00, bevel_3d(Vec3f(-0.5, -0.5, -0.5)));

    ASSERT_EQ(0x00, bevel_3d(Vec3f(1, 0, 0)));
    ASSERT_EQ(0x00, bevel_3d(Vec3f(-1, 0, 0)));
    ASSERT_EQ(0x00, bevel_3d(Vec3f(0, 1, 0)));
    ASSERT_EQ(0x00, bevel_3d(Vec3f(0, -1, 0)));
    ASSERT_EQ(0x00, bevel_3d(Vec3f(0, 0, 1)));
    ASSERT_EQ(0x00, bevel_3d(Vec3f(0, 0, -1)));

    ASSERT_EQ(0x03, bevel_3d(Vec3f(1, 1, 0)));
    ASSERT_EQ(0x0c, bevel_3d(Vec3f(1, -1, 0)));
    ASSERT_EQ(0x30, bevel_3d(Vec3f(-1, 1, 0)));
    ASSERT_EQ(0xc0, bevel_3d(Vec3f(-1, -1, 0)));

    ASSERT_EQ(0x11, bevel_3d(Vec3f(0, 1, 1)));
    ASSERT_EQ(0x22, bevel_3d(Vec3f(0, 1, -1)));
    ASSERT_EQ(0x44, bevel_3d(Vec3f(0, -1, 1)));
    ASSERT_EQ(0x88, bevel_3d(Vec3f(0, -1, -1)));

    ASSERT_EQ(0x05, bevel_3d(Vec3f(1, 0, 1)));
    ASSERT_EQ(0x50, bevel_3d(Vec3f(-1, 0, 1)));
    ASSERT_EQ(0x0a, bevel_3d(Vec3f(1, 0, -1)));
    ASSERT_EQ(0xa0, bevel_3d(Vec3f(-1, 0, -1)));

    ASSERT_EQ(0x01, bevel_3d(Vec3f(1, 1, 1)));
    ASSERT_EQ(0x02, bevel_3d(Vec3f(1, 1, -1)));
    ASSERT_EQ(0x04, bevel_3d(Vec3f(1, -1, 1)));
    ASSERT_EQ(0x08, bevel_3d(Vec3f(1, -1, -1)));
    ASSERT_EQ(0x10, bevel_3d(Vec3f(-1, 1, 1)));
    ASSERT_EQ(0x20, bevel_3d(Vec3f(-1, 1, -1)));
    ASSERT_EQ(0x40, bevel_3d(Vec3f(-1, -1, 1)));
    ASSERT_EQ(0x80, bevel_3d(Vec3f(-1, -1, -1)));

    std::default_random_engine engine{std::random_device()()};
    std::uniform_real_distribution<float> dist(-10.0f, 10.0f);

    for (auto i = 0u; i != 100; ++i) {
        auto x = dist(engine);
        auto y = dist(engine);
        auto z = dist(engine);
        ASSERT_EQ(bevel_3d(Point3{x, y, z}), bevel_3d(Vec3f(x, y, z)));
    }
}

TEST(lerp, lerp) {
    ASSERT_EQ(0, lerp(0, 0, 1));
    ASSERT_EQ(0, lerp(0.5, -1, 1));
    vec_assert_eq(Vec3f(0), lerp(0, Vec3f(0), Vec3f(1)));
    vec_assert_eq(Vec3f(0), lerp(0.5, Vec3f(-1), Vec3f(1)));
}

TEST(check_line, check_line) {
    ASSERT_EQ(Rel::idInside, check_line(Vec3f(-1, 0, 0), Vec3f(1, 0, 0), 0x01));
    ASSERT_EQ(Rel::idInside, check_line(Vec3f(-1, 0, 0), Vec3f(1, 0, 0), 0x02));
    ASSERT_EQ(Rel::idOutside,
              check_line(Vec3f(-1, 0, 0), Vec3f(1, 0, 0), 0x04));
    ASSERT_EQ(Rel::idOutside,
              check_line(Vec3f(-1, 0, 0), Vec3f(1, 0, 0), 0x08));
    ASSERT_EQ(Rel::idOutside,
              check_line(Vec3f(-1, 0, 0), Vec3f(1, 0, 0), 0x10));
    ASSERT_EQ(Rel::idOutside,
              check_line(Vec3f(-1, 0, 0), Vec3f(1, 0, 0), 0x20));

    ASSERT_EQ(Rel::idOutside,
              check_line(Vec3f(0, -1, 0), Vec3f(0, 1, 0), 0x01));
    ASSERT_EQ(Rel::idOutside,
              check_line(Vec3f(0, -1, 0), Vec3f(0, 1, 0), 0x02));
    ASSERT_EQ(Rel::idInside, check_line(Vec3f(0, -1, 0), Vec3f(0, 1, 0), 0x04));
    ASSERT_EQ(Rel::idInside, check_line(Vec3f(0, -1, 0), Vec3f(0, 1, 0), 0x08));
    ASSERT_EQ(Rel::idOutside,
              check_line(Vec3f(0, -1, 0), Vec3f(0, 1, 0), 0x10));
    ASSERT_EQ(Rel::idOutside,
              check_line(Vec3f(0, -1, 0), Vec3f(0, 1, 0), 0x20));

    ASSERT_EQ(Rel::idOutside,
              check_line(Vec3f(0, 0, -1), Vec3f(0, 0, 1), 0x01));
    ASSERT_EQ(Rel::idOutside,
              check_line(Vec3f(0, 0, -1), Vec3f(0, 0, 1), 0x02));
    ASSERT_EQ(Rel::idOutside,
              check_line(Vec3f(0, 0, -1), Vec3f(0, 0, 1), 0x04));
    ASSERT_EQ(Rel::idOutside,
              check_line(Vec3f(0, 0, -1), Vec3f(0, 0, 1), 0x08));
    ASSERT_EQ(Rel::idInside, check_line(Vec3f(0, 0, -1), Vec3f(0, 0, 1), 0x10));
    ASSERT_EQ(Rel::idInside, check_line(Vec3f(0, 0, -1), Vec3f(0, 0, 1), 0x20));

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
            ASSERT_EQ(
                check_line(Point3{x0, y0, z0}, Point3{x1, y1, z1}, diff),
                Rel::idOutside ==
                    check_line(Vec3f(x0, y0, z0), Vec3f(x1, y1, z1), diff));
    }
}

TEST(point_triangle_intersection, point_triangle_intersection) {
    ASSERT_EQ(Rel::idInside,
              point_triangle_intersection(
                  Vec3f(0, 0, 0),
                  TriangleVec3f(
                      {{Vec3f(-2, -1, 0), Vec3f(1, -1, 0), Vec3f(2, 1, 0)}})));

    ASSERT_EQ(Rel::idOutside,
              point_triangle_intersection(
                  Vec3f(4, 4, 4),
                  TriangleVec3f(
                      {{Vec3f(-2, -1, 0), Vec3f(1, -1, 0), Vec3f(2, 1, 0)}})));

    ASSERT_EQ(point_triangle_intersection(Point3{0.25, 0.25, 0.25},
                                          Triangle3{Point3{-0.25, -10, -5},
                                                    Point3{-0.25, 5, -5},
                                                    Point3{-0.25, 5, 10}}),
              Rel::idOutside == point_triangle_intersection(
                                    Vec3f(0.25, 0.25, 0.25),
                                    TriangleVec3f{{Vec3f(-0.25, -10, -5),
                                                   Vec3f(-0.25, 5, -5),
                                                   Vec3f(-0.25, 5, 10)}}));

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
                  Rel::idOutside == point_triangle_intersection(
                                        Vec3f(x0, y0, z0),
                                        TriangleVec3f{{Vec3f(x1, y1, z1),
                                                       Vec3f(x2, y2, z2),
                                                       Vec3f(x3, y3, z3)}}));
    }
}

TEST(old, old) {
    auto edge = 0.5f;
    ASSERT_EQ(
        INSIDE,
        t_c_intersection(Triangle3{
            Point3{-10, -5, edge}, Point3{5, -5, edge}, Point3{5, 10, edge}}));
    ASSERT_EQ(INSIDE,
              t_c_intersection(Triangle3{
                  Point3{-10, -5, 0}, Point3{5, -5, 0}, Point3{5, 10, 0}}));
    ASSERT_EQ(INSIDE,
              t_c_intersection(Triangle3{Point3{-10, -5, -edge},
                                         Point3{5, -5, -edge},
                                         Point3{5, 10, -edge}}));

    ASSERT_EQ(
        INSIDE,
        t_c_intersection(Triangle3{
            Point3{-10, edge, -5}, Point3{5, edge, -5}, Point3{5, edge, 10}}));
    ASSERT_EQ(INSIDE,
              t_c_intersection(Triangle3{
                  Point3{-10, 0, -5}, Point3{5, 0, -5}, Point3{5, 0, 10}}));
    ASSERT_EQ(INSIDE,
              t_c_intersection(Triangle3{Point3{-10, -edge, -5},
                                         Point3{5, -edge, -5},
                                         Point3{5, -edge, 10}}));

    ASSERT_EQ(
        INSIDE,
        t_c_intersection(Triangle3{
            Point3{edge, -10, -5}, Point3{edge, 5, -5}, Point3{edge, 5, 10}}));
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

TEST(specific, specific) {
    auto edge = 0.5;
    ASSERT_EQ(
        Rel::idInside,
        t_c_intersection(TriangleVec3f(
            {{Vec3f(-10, -5, edge), Vec3f(5, -5, edge), Vec3f(5, 10, edge)}})));
    ASSERT_EQ(Rel::idInside,
              t_c_intersection(TriangleVec3f(
                  {{Vec3f(-10, -5, 0), Vec3f(5, -5, 0), Vec3f(5, 10, 0)}})));
    ASSERT_EQ(Rel::idInside,
              t_c_intersection(TriangleVec3f({{Vec3f(-10, -5, -edge),
                                               Vec3f(5, -5, -edge),
                                               Vec3f(5, 10, -edge)}})));

    ASSERT_EQ(
        Rel::idInside,
        t_c_intersection(TriangleVec3f(
            {{Vec3f(-10, edge, -5), Vec3f(5, edge, -5), Vec3f(5, edge, 10)}})));
    ASSERT_EQ(Rel::idInside,
              t_c_intersection(TriangleVec3f(
                  {{Vec3f(-10, 0, -5), Vec3f(5, 0, -5), Vec3f(5, 0, 10)}})));
    ASSERT_EQ(Rel::idInside,
              t_c_intersection(TriangleVec3f({{Vec3f(-10, -edge, -5),
                                               Vec3f(5, -edge, -5),
                                               Vec3f(5, -edge, 10)}})));

    ASSERT_EQ(
        Rel::idInside,
        t_c_intersection(TriangleVec3f(
            {{Vec3f(edge, -10, -5), Vec3f(edge, 5, -5), Vec3f(edge, 5, 10)}})));
    ASSERT_EQ(Rel::idInside,
              t_c_intersection(TriangleVec3f(
                  {{Vec3f(0, -10, -5), Vec3f(0, 5, -5), Vec3f(0, 5, 10)}})));
    ASSERT_EQ(Rel::idInside,
              t_c_intersection(TriangleVec3f({{Vec3f(-0.25, -10, -5),
                                               Vec3f(-0.25, 5, -5),
                                               Vec3f(-0.25, 5, 10)}})));
    ASSERT_EQ(Rel::idInside,
              t_c_intersection(TriangleVec3f({{Vec3f(-edge, -10, -5),
                                               Vec3f(-edge, 5, -5),
                                               Vec3f(-edge, 5, 10)}})));

    auto outside = 1;
    ASSERT_EQ(Rel::idOutside,
              t_c_intersection(TriangleVec3f({{Vec3f(-10, -5, outside),
                                               Vec3f(5, -5, outside),
                                               Vec3f(5, 10, outside)}})));
    ASSERT_EQ(Rel::idOutside,
              t_c_intersection(TriangleVec3f({{Vec3f(-10, -5, -outside),
                                               Vec3f(5, -5, -outside),
                                               Vec3f(5, 10, -outside)}})));
    ASSERT_EQ(Rel::idOutside,
              t_c_intersection(TriangleVec3f({{Vec3f(-10, outside, -5),
                                               Vec3f(5, outside, -5),
                                               Vec3f(5, outside, 10)}})));
    ASSERT_EQ(Rel::idOutside,
              t_c_intersection(TriangleVec3f({{Vec3f(-10, -outside, -5),
                                               Vec3f(5, -outside, -5),
                                               Vec3f(5, -outside, 10)}})));
    ASSERT_EQ(Rel::idOutside,
              t_c_intersection(TriangleVec3f({{Vec3f(outside, -10, -5),
                                               Vec3f(outside, 5, -5),
                                               Vec3f(outside, 5, 10)}})));
    ASSERT_EQ(Rel::idOutside,
              t_c_intersection(TriangleVec3f({{Vec3f(-outside, -10, -5),
                                               Vec3f(-outside, 5, -5),
                                               Vec3f(-outside, 5, 10)}})));

    {
        TriangleVec3f triangle_verts_0(
            {{Vec3f(-1.5, 3.0999999, -7.5999999),
              Vec3f(0, 3.0999999, -9.10000038),
              Vec3f(0.00000000000000144381996, 3.5999999, -7.5999999)}});

        CuboidBoundary b0(Vec3f(-1.78125, 3.14999986, -8.55000019),
                          Vec3f(-1.1875, 3.26249981, -8.0749998));
        CuboidBoundary b1(Vec3f(-1.78125, 3.14999986, -8.0749998),
                          Vec3f(-1.1875, 3.26249981, -7.5999999));

        CuboidBoundary b2(Vec3f(-9.5, -0.0000000000000021191102, -7.5999999),
                          Vec3f(-7.125, 0.449999988, -5.69999981));
        auto centre = b2.get_centre();
        auto x0 = b2.get_c0().x;
        auto y0 = b2.get_c0().y;
        auto z0 = b2.get_c0().z;
        auto xc = centre.x;
        auto yc = centre.y;
        auto zc = centre.z;
        auto x1 = b2.get_c1().x;
        auto y1 = b2.get_c1().y;
        auto z1 = b2.get_c1().z;
        CuboidBoundary b3(Vec3f(x0, yc, z0), Vec3f(xc, y1, zc));

        TriangleVec3f triangle_verts_1({{Vec3f(-9.5, 1.10000002, -6.0999999),
                                         Vec3f(-9.5, 0.100000001, -6.0999999),
                                         Vec3f(-9.5, 0.100000001, -9)}});

        ASSERT_EQ(false, b0.overlaps(triangle_verts_0));
        ASSERT_EQ(true, b1.overlaps(triangle_verts_0));

        ASSERT_EQ(true, b2.overlaps(triangle_verts_1));
        ASSERT_EQ(true, b3.overlaps(triangle_verts_1));
    }

    {
        //        ASSERT_EQ(INSIDE,
        //        t_c_intersection(Triangle3{Point3{-5.49999523, 0.184210628,
        //        4.60390043},
        //            Point3{0.833333611, 0.184210628, 4.60390043},
        //            Point3{0.833333611, 0.184210628, -3.29220581}}));

        CuboidBoundary aabb(Vec3f(-4.20000029, -0.040625006, -5.02812529),
                            Vec3f(-3.9000001, 0.0187499933, -4.78750038));

        TriangleVec3f verts{{Vec3f(-5.69999981, 0, -3.79999995),
                             Vec3f(-3.79999995, 0, -3.79999995),
                             Vec3f(-3.79999995, 0, -5.69999981)}};

        ASSERT_EQ(true, aabb.overlaps(verts));
    }

    {
        CuboidBoundary aabb(Vec3f(-7.80000019, 3.46249986, -8.63750076),
                            Vec3f(-7.20000029, 3.58124971, -8.15625));

        TriangleVec3f v0{{Vec3f(-7.5999999, 3.5999999, -7.5999999),
                          Vec3f(-7.5999999, 3.0999999, -9.10000038),
                          Vec3f(-6.0999999, 3.0999999, -7.5999999)}};

        TriangleVec3f v1{{Vec3f(-9.10000038, 3.0999999, -7.5999999),
                          Vec3f(-7.5999999, 3.0999999, -9.10000038),
                          Vec3f(-7.5999999, 3.5999999, -7.5999999)}};

        ASSERT_EQ(false, aabb.overlaps(v0));
        ASSERT_EQ(false, aabb.overlaps(v1));
    }
}
