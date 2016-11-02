#include "waveguide/fitted_boundary.h"

#include "core/serialize/filter_coefficients.h"

#include "utilities/apply.h"
#include "utilities/decibels.h"
#include "utilities/for_each.h"

#include "cereal/archives/json.hpp"

#include "gtest/gtest.h"

using namespace wayverb::waveguide;
using namespace wayverb::core;

namespace {
template <size_t B, size_t A>
std::ostream &operator<<(std::ostream &os,
                         const filter_coefficients<B, A> &coeffs) {
    cereal::JSONOutputArchive archive{os};
    archive(coeffs);
    return os;
}
}  // namespace

template <typename T, size_t N>
void test_arrays_near(const std::array<T, N> &a, const std::array<T, N> &b) {
    for_each([](auto a, auto b) { ASSERT_NEAR(a, b, 0.00000001); }, a, b);
}

TEST(fitted_boundary, compute_boundary_filter) {
    constexpr std::array<double, 5> centres{{0.2, 0.4, 0.6, 0.8, 1.0}};
    constexpr std::array<double, 5> amplitudes{{0, 1, 0.5, 1, 0}};
    constexpr auto order = 6;

    const auto coeffs = arbitrary_magnitude_filter<order>(
            make_frequency_domain_envelope(centres, amplitudes));

    std::cout << coeffs << '\n';
}
