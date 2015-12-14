#include "helper.h"
#include "vec.h"

#include "gtest/gtest.h"

TEST(multiplies, multiplies) {
    vec_assert_eq(Vec3f(12), Vec3f(3) * 4);
}

TEST(dot, dot) {
    ASSERT_EQ(17, Vec3f(0, 1, 2).dot(Vec3f(4, 5, 6)));
}

TEST(cross, cross) {
    vec_assert_eq(Vec3f(-3, 6, -3), Vec3f(1, 2, 3).cross(Vec3f(4, 5, 6)));
}
