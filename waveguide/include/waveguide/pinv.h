#pragma once

#include <eigen3/Eigen/LU>
#include <eigen3/Eigen/SVD>

template <typename Scalar, int Rows, int Columns>
inline Eigen::Matrix<Scalar, Columns, Rows> pinv(
        const Eigen::Matrix<Scalar, Rows, Columns>& a,
        double epsilon = std::numeric_limits<Scalar>::epsilon()) {
    //  taken from http://eigen.tuxfamily.org/bz/show_bug.cgi?id=257
    Eigen::JacobiSVD <
            Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic>> svd(
                    a, Eigen::ComputeThinU | Eigen::ComputeThinV);
    constexpr auto max_rows_cols = std::max(Rows, Columns);
    const auto tolerance =
            epsilon * max_rows_cols * svd.singularValues().array().abs()(0);
    const auto v = svd.matrixV();
    const Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> intermediate =
            (svd.singularValues().array().abs() > tolerance)
                    .select(svd.singularValues().array().inverse(), 0)
                    .matrix()
                    .asDiagonal();
    const auto u_adjoint = svd.matrixU().adjoint();
    return v * intermediate * u_adjoint;
}
