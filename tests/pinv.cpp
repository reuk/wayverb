#include "waveguide/pinv.h"

#include "gtest/gtest.h"

TEST(pinv, pinv) {
    {
        Eigen::Matrix<double, 2, 3> m;
        m.row(0) << 1, 2, 3;
        m.row(1) << 4, 5, 6;
        std::cout << pinv(m) << '\n';
    }

    {
        Eigen::Matrix<double, 3, 2> m;
        m.row(0) << 1, 2;
        m.row(1) << 3, 4;
        m.row(2) << 5, 6;
        std::cout << pinv(m) << '\n';
    }
}
