#include "helper.h"
#include "tri_cube_intersection.h"

#include "gtest/gtest.h"

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
}

TEST(point_triangle_intersection, point_triangle_intersection) {
    ASSERT_EQ(Rel::idInside,
              point_triangle_intersection(
                  Vec3f(0, 0, 0),
                  Triangle{0, 0, 1, 2},
                  {Vec3f(-2, -1, 0), Vec3f(1, -1, 0), Vec3f(2, 1, 0)}));

    ASSERT_EQ(Rel::idOutside,
              point_triangle_intersection(
                  Vec3f(4, 4, 4),
                  Triangle{0, 0, 1, 2},
                  {Vec3f(-2, -1, 0), Vec3f(1, -1, 0), Vec3f(2, 1, 0)}));
}
