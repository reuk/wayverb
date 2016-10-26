#pragma once

#include "common/freqz.h"

#include "utilities/foldl.h"
#include "utilities/map.h"

//  Eigen generates a lot of warnings when I compile with -Wall -Werror which
//  means *my* warnings get buried under a mountain of stuff that I should
//  probably fix and submit a PR about.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-register"
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#include "eigen3/Eigen/Dense"
#pragma clang diagnostic pop

#include <complex>
#include <iostream>

/// see https://gist.github.com/mattdsp/7617379e9920c5cbb016

namespace waveguide {

template <size_t... Ix>
auto make_response(const std::array<double, sizeof...(Ix)>& amplitudes,
                   const std::array<double, sizeof...(Ix)>& frequencies,
                   double delay,
                   std::index_sequence<Ix...>) {
    using namespace std::complex_literals;
    return std::array<std::complex<double>, sizeof...(Ix)>{
            {std::get<Ix>(amplitudes) *
             std::exp(-1i * delay * std::get<Ix>(frequencies))...}};
}

template <size_t N>
auto make_response(const std::array<double, N>& amplitudes,
                   const std::array<double, N>& frequencies,
                   double delay) {
    return make_response(
            amplitudes, frequencies, delay, std::make_index_sequence<N>{});
}

////////////////////////////////////////////////////////////////////////////////

template <typename T, size_t N>
auto to_eigen_vector(const std::array<T, N>& arr) {
    Eigen::Matrix<T, N, 1> ret{};
    for (auto i = 0; i != N; ++i) {
        ret(i) = arr[i];
    }
    return ret;
}

template <typename T, size_t... Ix>
auto to_array(const Eigen::Matrix<T, sizeof...(Ix), 1>& mat,
              std::index_sequence<Ix...>) {
    return std::array<T, sizeof...(Ix)>{{mat(Ix)...}};
}

template <typename T, int N>
auto to_array(const Eigen::Matrix<T, N, 1>& mat) {
    return to_array(mat, std::make_index_sequence<N>{});
}

template <int Row, int Col>
auto split_real_imag(const Eigen::Matrix<std::complex<double>, Row, Col>& mat) {
    Eigen::Matrix<double, 2 * Row, Col> ret{};
    ret << real(mat.array()), imag(mat.array());
    return ret;
}

////////////////////////////////////////////////////////////////////////////////

template <size_t M, size_t N, size_t I>
auto eqnerror(const std::array<double, I>& frequencies,
              const std::array<std::complex<double>, I>& response,
              const std::array<double, I>& weight,
              size_t iter = 0) {
    using namespace std::complex_literals;

    // D0 = D(:); w = w(:); W0 = W(:);
    const auto D0 = to_eigen_vector(response);
    const auto w0 = to_eigen_vector(frequencies);
    const auto W0 = to_eigen_vector(weight);

    // A0 = [-D0(:,ones(N,1)).*exp(-1i*w*(1:N)), exp(-1i*w*(0:M))];
    const auto A0 = [&] {
        const auto first_bit =
                -D0.template replicate<1, N>().array() *
                exp((-1.i * w0 * Eigen::Matrix<double, 1, N>::LinSpaced(1, N))
                            .array());

        const auto second_bit = exp(
                (-1.i * w0 * Eigen::Matrix<double, 1, M + 1>::LinSpaced(0, M))
                        .array());

        Eigen::Matrix<std::complex<double>, I, N + M + 1> ret{};
        ret << first_bit, second_bit;
        return ret;
    }();

    // den = ones(L,1);
    Eigen::Matrix<std::complex<double>, I, 1> den =
            Eigen::Matrix<std::complex<double>, I, 1>::Ones();
    std::array<double, N + 1> a;
    std::array<double, M + 1> b;

    // for k = 1:iter,
    for (auto k = 0ul; k != iter + 1; ++k) {
        // W = W0./abs(den);
        const Eigen::Matrix<double, I, 1> W = W0.array() / abs(den.array());

        // A = A0.*W(:,ones(M+N+1,1)); D = D0.*W;
        const Eigen::Matrix<std::complex<double>, I, M + N + 1> A =
                A0.array() * W.template replicate<1, M + N + 1>().array();

        const Eigen::Matrix<std::complex<double>, I, 1> D =
                D0.array() * W.array();

        // x = [real(A);imag(A)]\[real(D);imag(D)];
        const auto x = split_real_imag(A).colPivHouseholderQr().solve(
                split_real_imag(D));

        // a = [1;x(1:N)]; b = x(N+1:M+N+1);
        a = to_array([&] {
            Eigen::Matrix<double, N + 1, 1> ret{};
            ret << 1.0, x.template segment<N>(0);
            return ret;
        }());
        b = to_array(
                Eigen::Matrix<double, M + 1, 1>{x.template segment<M + 1>(N)});

        // den = freqz(a,1,w);
        den = to_eigen_vector(freqz(a, frequencies));

        // end
    }

    return std::make_tuple(b, a);
}

}  // namespace waveguide
