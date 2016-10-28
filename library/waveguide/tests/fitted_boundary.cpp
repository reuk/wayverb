#include "waveguide/fitted_boundary.h"

#include "common/serialize/filter_coefficients.h"

#include "utilities/apply.h"
#include "utilities/decibels.h"
#include "utilities/for_each.h"

#include "cereal/archives/json.hpp"

#include "gtest/gtest.h"

namespace {

template <size_t B, size_t A>
std::ostream &operator<<(std::ostream &os,
                         const filter_coefficients<B, A> &coeffs) {
    cereal::JSONOutputArchive{os}(coeffs);
    return os;
}

}  // namespace

template <typename T, size_t N>
void test_arrays_near(const std::array<T, N>& a, const std::array<T, N>& b) {
    for_each([](auto a, auto b) { ASSERT_NEAR(a, b, 0.00000001); }, a, b);
}

TEST(fitted_boundary, eqnerror) {
    const auto test = [](const auto &computed, const auto &desired) {
        test_arrays_near(computed.b, desired.b);
        test_arrays_near(computed.a, desired.a);
    };

    {
        const auto frequencies = std::array<double, 3>{{0.2, 0.4, 0.6}};
        test(waveguide::eqnerror<2, 2>(
                     frequencies,
                     waveguide::make_response(
                             std::array<double, 3>{{1, 1, 1}}, frequencies, 1),
                     std::array<double, 3>{{1, 1, 1}}),

             filter_coefficients<2, 2>{
                     {{-2.6366e-16, 1.0000e+00, 2.0390e-15}},
                     {{1.0000e+00, 3.2715e-15, -4.4409e-16}}});
    }

    {
        const auto frequencies =
                map([](const auto &i) { return M_PI * i; },
                    std::array<double, 5>{{0.1, 0.25, 0.5, 0.75, 0.9}});

        const auto amplitudes =
                map([](const auto &i) { return decibels::db2a(i); },
                    std::array<double, 5>{{-40, -6, -24, -3, -40}});

        std::cout << waveguide::eqnerror<7, 7>(
                             frequencies,
                             waveguide::make_response(
                                     amplitudes, frequencies, 1),
                             std::array<double, 5>{{1, 1, 1, 1, 1}})
                  << '\n';
    }
}

TEST(fitted_boundary, compute_boundary_filter) {
    constexpr std::array<double, 5> centres{{0.2, 0.4, 0.6, 0.8, 1.0}};
    constexpr std::array<double, 5> amplitudes{{0, 1, 0.5, 1, 0}};
    constexpr auto order = 6;
    constexpr auto delay = 0.0;
    constexpr auto iterations = 10;

    const auto coeffs = waveguide::arbitrary_magnitude_filter<order>(
            centres, amplitudes, delay, iterations);

    std::cout << coeffs << '\n';
}
