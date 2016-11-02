#include "core/dc_blocker.h"
#include "core/dsp_vector_ops.h"

#include "frequency_domain/envelope.h"
#include "frequency_domain/filter.h"

#include "utilities/string_builder.h"

#include "audio_file/audio_file.h"

#include "gtest/gtest.h"

#include <algorithm>
#include <random>

using namespace wayverb::core;

TEST(dc_blocker, delay_line) {
    constexpr auto LENGTH = 4;

    filter::delay_line dl(LENGTH);

    for (auto i = 0; i != LENGTH; ++i) {
        ASSERT_EQ(dl[i], 0);
    }

    dl.push(1);
    ASSERT_EQ(dl[0], 1);

    dl.push(2);
    ASSERT_EQ(dl[0], 2);
    ASSERT_EQ(dl[1], 1);

    dl.push(3);
    ASSERT_EQ(dl[0], 3);
    ASSERT_EQ(dl[1], 2);
    ASSERT_EQ(dl[2], 1);

    dl.push(0);
    dl.push(0);
    dl.push(0);
    dl.push(0);
    ASSERT_EQ(dl[0], 0);
    ASSERT_EQ(dl[1], 0);
    ASSERT_EQ(dl[2], 0);
}

TEST(dc_blocker, moving_average) {
    filter::moving_average ma(4);
    ASSERT_EQ(ma.filter(0), 0);
    ASSERT_EQ(ma.filter(0), 0);
    ASSERT_EQ(ma.filter(0), 0);
    ASSERT_EQ(ma.filter(0), 0);
    ASSERT_EQ(ma.filter(0), 0);
    ASSERT_EQ(ma.filter(0), 0);

    ASSERT_EQ(ma.filter(1), 1 / 4.0);
    ASSERT_EQ(ma.filter(0), 1 / 4.0);
    ASSERT_EQ(ma.filter(0), 1 / 4.0);
    ASSERT_EQ(ma.filter(0), 1 / 4.0);

    ASSERT_EQ(ma.filter(0), 0);
    ASSERT_EQ(ma.filter(0), 0);
}

TEST(dc_blocker, two_moving_average) {
    filter::n_moving_averages<2> ma(4);
    ASSERT_EQ(ma.filter(0), 0);
    ASSERT_EQ(ma.filter(0), 0);
    ASSERT_EQ(ma.filter(0), 0);
    ASSERT_EQ(ma.filter(0), 0);
    ASSERT_EQ(ma.filter(0), 0);
    ASSERT_EQ(ma.filter(0), 0);

    ASSERT_EQ(ma.filter(1), 1 / 16.0);
    ASSERT_EQ(ma.filter(0), 2 / 16.0);
    ASSERT_EQ(ma.filter(0), 3 / 16.0);
    ASSERT_EQ(ma.filter(0), 4 / 16.0);

    ASSERT_EQ(ma.filter(0), 3 / 16.0);
    ASSERT_EQ(ma.filter(0), 2 / 16.0);
    ASSERT_EQ(ma.filter(0), 1 / 16.0);
    ASSERT_EQ(ma.filter(0), 0 / 16.0);

    ASSERT_EQ(ma.filter(0), 0);
    ASSERT_EQ(ma.filter(0), 0);
    ASSERT_EQ(ma.filter(0), 0);
    ASSERT_EQ(ma.filter(0), 0);
}

TEST(dc_blocker, dc_blocker) {
    filter::linear_dc_blocker dc(4);
    ASSERT_EQ(dc.filter(0), 0);
    ASSERT_EQ(dc.filter(0), 0);
    ASSERT_EQ(dc.filter(0), 0);
    ASSERT_EQ(dc.filter(0), 0);
    ASSERT_EQ(dc.filter(0), 0);
    ASSERT_EQ(dc.filter(0), 0);

    ASSERT_EQ(dc.filter(1), -1 / 16.0);
    ASSERT_EQ(dc.filter(0), -2 / 16.0);
    ASSERT_EQ(dc.filter(0), -3 / 16.0);
    ASSERT_EQ(dc.filter(0), 12 / 16.0);

    ASSERT_EQ(dc.filter(0), -3 / 16.0);
    ASSERT_EQ(dc.filter(0), -2 / 16.0);
    ASSERT_EQ(dc.filter(0), -1 / 16.0);
    ASSERT_EQ(dc.filter(0), -0 / 16.0);

    ASSERT_EQ(dc.filter(0), 0);
    ASSERT_EQ(dc.filter(0), 0);
    ASSERT_EQ(dc.filter(0), 0);
    ASSERT_EQ(dc.filter(0), 0);
}

TEST(dc_blocker, big_offset) {
    filter::linear_dc_blocker dc(4);
    ASSERT_EQ(dc.filter(2), -2 / 16.0);
    ASSERT_EQ(dc.filter(2), -6 / 16.0);
    ASSERT_EQ(dc.filter(2), -12 / 16.0);
    ASSERT_EQ(dc.filter(2), 12 / 16.0);
    ASSERT_EQ(dc.filter(2), 6 / 16.0);
    ASSERT_EQ(dc.filter(2), 2 / 16.0);
}

auto generate_noise(size_t samples, float mag = 0.5f) {
    std::default_random_engine engine{std::random_device()()};
    std::uniform_real_distribution<float> dist{-mag, mag};
    util::aligned::vector<float> ret(samples);
    std::generate(
            ret.begin(), ret.end(), [&engine, &dist] { return dist(engine); });
    return ret;
}

auto generate_sweep(size_t samples) {
    double phase{0};
    util::aligned::vector<float> ret(samples);
    for (auto i = 0u; i != samples; ++i) {
        ret[i] = std::sin(phase * 2 * M_PI);
        phase += (i * 0.5) / samples;
        phase = std::fmod(phase, 1);
    }
    return ret;
}

auto generate_impulse(size_t samples) {
    util::aligned::vector<float> ret(samples, 0);
    ret[ret.size() / 2] = 1;
    return ret;
}

TEST(dc_blocker, io) {
    struct signal final {
        std::string name;
        util::aligned::vector<float> kernel;
    };

    constexpr auto samples = 1000000;

    util::aligned::vector<signal> signals{
            signal{"noise", generate_noise(samples)},
            signal{"sweep", generate_sweep(samples)},
            signal{"impulse", generate_impulse(samples)},
    };

    for (const auto& i : signals) {
        write(util::build_string("dc_test.input.", i.name, ".wav"),
              audio_file::make_audio_file(i.kernel, 44100),
              16);

        auto run{[&i](auto& filter, const auto& filter_name, auto i) {
            filter::run_two_pass(filter, i.kernel.begin(), i.kernel.end());
            normalize(i.kernel);
            write(util::build_string(
                          "dc_test.output.", filter_name, ".", i.name, ".wav"),
                  audio_file::make_audio_file(i.kernel, 44100),
                  16);
        }};

        {
            filter::linear_dc_blocker dc;
            run(dc, "normal", i);
        }

        {
            filter::extra_linear_dc_blocker dc;
            run(dc, "super", i);
        }
    }
}

namespace {
using callback = std::function<util::aligned::vector<float>(
        util::aligned::vector<float> sig, float cutoff, float sample_rate)>;

const util::aligned::vector<std::tuple<callback, std::string>> trial_blockers{

        {[](auto sig, auto cutoff, auto sample_rate) {
             auto blocker{make_series_biquads(
                     filter::compute_hipass_butterworth_coefficients<1>(
                             10, sample_rate))};
             run_two_pass(blocker, sig.begin(), sig.end());
             return sig;
         },
         "butterworth_1"},

        {[](auto sig, auto cutoff, auto sample_rate) {
             const util::aligned::vector<float> zeros(sig.size(), 0);
             sig.insert(sig.begin(), zeros.begin(), zeros.end());
             auto blocker{make_series_biquads(
                     filter::compute_hipass_butterworth_coefficients<1>(
                             10, sample_rate))};
             run_two_pass(blocker, sig.begin(), sig.end());
             return util::aligned::vector<float>(sig.begin() + zeros.size(),
                                                 sig.end());
         },
         "prepadded_butterworth_1"},

        {[](auto sig, auto cutoff, auto sample_rate) {
             auto blocker{make_series_biquads(
                     filter::compute_hipass_butterworth_coefficients<2>(
                             10, sample_rate))};
             run_two_pass(blocker, sig.begin(), sig.end());
             return sig;
         },
         "butterworth_2"},

        {[](auto sig, auto cutoff, auto sample_rate) {
             auto blocker{make_series_biquads(
                     filter::compute_hipass_butterworth_coefficients<3>(
                             10, sample_rate))};
             run_two_pass(blocker, sig.begin(), sig.end());
             return sig;
         },
         "butterworth_3"},

        {[](auto sig, auto cutoff, auto sample_rate) {
             auto blocker = filter::biquad{
                     filter::compute_dc_blocker_coefficients(0.999)};
             run_one_pass(blocker, sig.begin(), sig.end());
             return sig;
         },
         "biquad_onepass"},

        {[](auto sig, auto cutoff, auto sample_rate) {
             auto blocker = filter::biquad{
                     filter::compute_dc_blocker_coefficients(0.999)};
             run_two_pass(blocker, sig.begin(), sig.end());
             return sig;
         },
         "biquad_twopass"},

        {[](auto sig, auto cutoff, auto sample_rate) {
             const auto normalised_cutoff{cutoff / sample_rate};
             frequency_domain::filter blocker{sig.size()};
             blocker.run(sig.begin(),
                         sig.end(),
                         sig.begin(),
                         [=](auto cplx, auto freq) {
                             return static_cast<float>(
                                            frequency_domain::
                                                    compute_hipass_magnitude(
                                                            freq,
                                                            normalised_cutoff,
                                                            normalised_cutoff *
                                                                    0.5)) *
                                    cplx;
                         });
             return sig;
         },
         "fft"},

        {[](auto sig, auto cutoff, auto sample_rate) {
             const auto normalised_cutoff{cutoff / sample_rate};
             frequency_domain::filter blocker{sig.size() * 2};
             blocker.run(sig.begin(),
                         sig.end(),
                         sig.begin(),
                         [=](auto cplx, auto freq) {
                             return static_cast<float>(
                                            frequency_domain::
                                                    compute_hipass_magnitude(
                                                            freq,
                                                            normalised_cutoff,
                                                            normalised_cutoff *
                                                                    0.5)) *
                                    cplx;
                         });
             return sig;
         },
         "padded_fft"},

};
}  // namespace

TEST(dc_blocker, impulses) {
    const auto sample_rate{44100.0};
    const auto cutoff{10.0};

    util::aligned::vector<float> input(sample_rate * 10, 0);
    input[20000] = 1;
    input[40000] = 1;
    input[60000] = 1;
    input[80000] = 1;
    input[100000] = 1;
    input[200000] = 1;
    input[300000] = 1;
    input[400000] = 1;

    for (const auto& i : trial_blockers) {
        {
            const auto output{std::get<0>(i)(input, cutoff, sample_rate)};
            write(util::build_string(
                          "impulses.dc_blocker.", std::get<1>(i), ".wav"),
                  audio_file::make_audio_file(output, sample_rate),
                  16);
        }
    }
}

TEST(dc_blocker, increasing_offset) {
    const auto sample_rate{44100.0};
    const auto cutoff{40.0};
    const auto noise{generate_noise(sample_rate * 10)};

    auto increasing_offset{noise};
    const auto inc{0.001};
    auto offset{0.0};
    for (auto& sample : increasing_offset) {
        sample = (sample * 0.1) + offset;
        offset += inc;
    }

    for (const auto& i : trial_blockers) {
        {
            const auto output{
                    std::get<0>(i)(increasing_offset, cutoff, sample_rate)};
            write(util::build_string("dc_blocker.", std::get<1>(i), ".wav"),
                  audio_file::make_audio_file(output, sample_rate),
                  16);
        }
        {
            auto input{increasing_offset};
            input.insert(input.end(),
                         increasing_offset.crbegin(),
                         increasing_offset.crend());
            const auto output{std::get<0>(i)(input, cutoff, sample_rate)};
            write(util::build_string(
                          "mirrored.dc_blocker.", std::get<1>(i), ".wav"),
                  audio_file::make_audio_file(output, sample_rate),
                  16);
        }
    }
}
