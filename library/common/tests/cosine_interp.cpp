#include "common/cosine_interp.h"

#include "utilities/identity.h"

#include "gtest/gtest.h"

TEST(interp, linear) {
    ASSERT_EQ(linear_interp(0, -100, 100), -100);
    ASSERT_EQ(linear_interp(1, -100, 100), 100);
    ASSERT_EQ(linear_interp(0.5, -100, 100), 0);
}

TEST(interp, cosine) {
    ASSERT_EQ(cosine_interp(0, -100, 100), -100);
    ASSERT_EQ(cosine_interp(1, -100, 100), 100);
    ASSERT_EQ(cosine_interp(0.5, -100, 100), 0);
}

TEST(interp, interp) {
    ASSERT_EQ(interp(0, -1, 1, -100, 100, linear_interp_functor{}), 0);
    ASSERT_EQ(interp(0, -1, 1, -100, 100, cosine_interp_functor{}), 0);

    ASSERT_EQ(interp(0, -2, 1, 0, 3, linear_interp_functor{}), 2);

    ////////////////////////////////////////////////////////////////////////////

    struct point final {
        double x;
        double y;
    };

    std::vector<point> points{{0, 0}, {2, 4}, {4, 0}, {8, -100}, {10, 20}};

    const auto test = [&](auto x, auto actual) {
        ASSERT_EQ(
                interp(begin(points), end(points), x, linear_interp_functor{}),
                actual);
    };

    test(0, 0);
    test(1, 2);
    test(2, 4);
    test(3, 2);
    test(4, 0);
    test(5, -25);
    test(6, -50);
    test(7, -75);
    test(8, -100);
    test(9, -40);
    test(10, 20);
}
