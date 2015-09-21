#include "vec.h"

#include "gtest/gtest.h"

using namespace std;

template <typename T, typename U>
void vec_assert_eq(T t, U u) {
    t.zip(u).for_each([](auto i) { ASSERT_EQ(get<0>(i), get<1>(i)); });
}

TEST(multiplies, multiplies) {
    vec_assert_eq(Vec3f(12), Vec3f(3) * 4);
}

TEST(dot, dot) {
    ASSERT_EQ(17, Vec3f(0, 1, 2).dot(Vec3f(4, 5, 6)));
}

TEST(cross, cross) {
    vec_assert_eq(Vec3f(-3, 6, -3), Vec3f(1, 2, 3).cross(Vec3f(4, 5, 6)));
}
