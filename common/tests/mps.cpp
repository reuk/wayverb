#include "common/hilbert.h"

#include "gtest/gtest.h"

#include <cmath>

TEST(mps, arbitrary_spectrum) {
    aligned::vector<std::complex<float>> spec{
            {1, 0}, {1, 0}, {1, 0}, {0, 0}, {0, 0}, {0, 0}, {1, 0}, {1, 0}};
    const auto minimum_phase_spectrum{mps(spec)};

    const auto i_spec{ifft(spec)};
    const auto i_fft_spec{ifft(minimum_phase_spectrum)};
}

TEST(mps, front_dirac) {
    const aligned::vector<float> sig{1, 0, 0, 0, 0, 0, 0, 0};
    auto spec{fft(make_complex(sig))};

    for (auto i : spec) {
        ASSERT_EQ(i.real(), 1);
        ASSERT_EQ(i.imag(), 0);
    }

    const auto minimum_phase_spectrum{mps(spec)};

    ASSERT_EQ(spec.size(), minimum_phase_spectrum.size());

    for (auto i{0u}; i != minimum_phase_spectrum.size(); ++i) {
        ASSERT_EQ(spec[i], minimum_phase_spectrum[i]);
    }
}

TEST(mps, zero_spectrum) {
    aligned::vector<std::complex<float>> spec{
            {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};
    ASSERT_THROW(mps(spec), std::runtime_error);
}

aligned::vector<std::complex<float>> round_trip(
        aligned::vector<std::complex<float>> sig) {
    const auto spec{fft(sig)};
    const auto min_phase{mps(spec)};
    auto ret{ifft(min_phase)};
    for (auto& i : ret) {
        i /= ret.size();
    }
    return ret;
}

TEST(mps, round_trip) {
    const auto sig{
            make_complex(aligned::vector<float>{0, 1, 0, 0, 0, 0, 0, 0})};

    const auto t0{round_trip(sig)};
    const auto t1{round_trip(t0)};

    ASSERT_EQ(t0, t1);
}

TEST(mps, arbitrary_dirac) {
    const auto test_sig{[](const auto& sig) {
        auto spec{fft(make_complex(sig))};
        const auto minimum_phase_spectrum{mps(spec)};
        ASSERT_EQ(spec.size(), minimum_phase_spectrum.size());
        for (auto i : minimum_phase_spectrum) {
            ASSERT_NEAR(std::abs(i), 1, 0.00001);
        }
        auto synth{ifft(minimum_phase_spectrum)};
        for (auto& i : synth) {
            i /= synth.size();
        }
    }};

    test_sig(aligned::vector<float>{0, 1, 0, 0, 0, 0, 0, 0});
    test_sig(aligned::vector<float>{0, 1, 0, 0, 0, 0, 0});
    test_sig(aligned::vector<float>{0, 0, 1, 0, 0, 0, 0, 0});
    test_sig(aligned::vector<float>{0, 0, 1, 0, 0, 0, 0});
    test_sig(aligned::vector<float>{0, 0, 0, 1, 0, 0, 0, 0});
    test_sig(aligned::vector<float>{0, 0, 0, 1, 0, 0, 0});
    test_sig(aligned::vector<float>{0, 0, 0, 0, 1, 0, 0, 0});
    test_sig(aligned::vector<float>{0, 0, 0, 0, 1, 0, 0});
    test_sig(aligned::vector<float>{0, 0, 0, 0, 0, 1, 0, 0});
    test_sig(aligned::vector<float>{0, 0, 0, 0, 0, 1, 0});
    test_sig(aligned::vector<float>{0, 0, 0, 0, 0, 0, 1, 0});
    test_sig(aligned::vector<float>{0, 0, 0, 0, 0, 0, 1});
    test_sig(aligned::vector<float>{0, 0, 0, 0, 0, 0, 0, 1});
}

TEST(mps, sin) {
    const aligned::vector<float> sig{0, 1, 0, -1, 0, 1, 0, -1};
    auto spec{fft(make_complex(sig))};
    for (auto i : spec) {
        std::cout << i << ", ";
    }
    std::cout << '\n';

    const auto minimum_phase_spectrum{mps(spec)};
    for (auto i : minimum_phase_spectrum) {
        std::cout << i << ", ";
    }
    std::cout << '\n';

    auto synth{ifft(minimum_phase_spectrum)};
    for (auto& i : synth) {
        i /= synth.size();
    }

    for (auto i : synth) {
        std::cout << i << ", ";
    }
    std::cout << '\n';
}
