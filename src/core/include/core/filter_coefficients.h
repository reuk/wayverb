#pragma once

#include <array>

namespace wayverb {
namespace core {

template <size_t B, size_t A>
struct filter_coefficients final {
    using b_order = std::integral_constant<size_t, B>;
    using a_order = std::integral_constant<size_t, A>;

    using b_size = std::integral_constant<size_t, b_order{} + 1>;
    using a_size = std::integral_constant<size_t, a_order{} + 1>;

    std::array<double, b_size{}> b;
    std::array<double, a_size{}> a;
};

template <size_t B, size_t A>
constexpr auto make_filter_coefficients(const std::array<double, B>& b,
                                        const std::array<double, A>& a) {
    return filter_coefficients<B - 1, A - 1>{b, a};
}

}  // namespace core
}  // namespace wayverb
