#include "waveguide/filters.h"
#include "waveguide/fitted_boundary.h"
#include "waveguide/program.h"
#include "waveguide/waveguide.h"

#include "utilities/map.h"

#include "common/cl/common.h"

#include "audio_file/audio_file.h"

#include "gtest/gtest.h"

#include <array>
#include <random>
#include <type_traits>

class InputGenerator {
public:
    virtual util::aligned::vector<util::aligned::vector<cl_float>>
    compute_input(int size) = 0;

protected:
    ~InputGenerator() noexcept = default;
};

class NoiseGenerator : public InputGenerator {
public:
    virtual util::aligned::vector<util::aligned::vector<cl_float>>
    compute_input(int size) override {
        static auto ret = generate(size);
        return ret;
    }

private:
    static util::aligned::vector<util::aligned::vector<cl_float>> generate(
            int size) {
        auto ret = util::aligned::vector<util::aligned::vector<cl_float>>{
                10000, util::aligned::vector<cl_float>(size, 0)};
        for (auto& i : ret)
            std::generate(i.begin(), i.end(), [] { return range(engine); });
        return ret;
    }

    static std::default_random_engine engine;
    static constexpr float r{0.25};
    static std::uniform_real_distribution<cl_float> range;
};

std::default_random_engine NoiseGenerator::engine{std::random_device()()};
std::uniform_real_distribution<cl_float> NoiseGenerator::range{-r, r};

class QuietNoiseGenerator : public InputGenerator {
public:
    virtual util::aligned::vector<util::aligned::vector<cl_float>>
    compute_input(int size) override {
        static auto ret = generate(size);
        return ret;
    }

private:
    static util::aligned::vector<util::aligned::vector<cl_float>> generate(
            int size) {
        auto ret = util::aligned::vector<util::aligned::vector<cl_float>>{
                40000, util::aligned::vector<cl_float>(size, 0)};
        for (auto& i : ret)
            std::generate(begin(i), end(i), [] { return range(engine); });
        return ret;
    }

    static std::default_random_engine engine;
    static constexpr float r{1e-35};
    static std::uniform_real_distribution<cl_float> range;
};

std::default_random_engine QuietNoiseGenerator::engine{std::random_device()()};
std::uniform_real_distribution<cl_float> QuietNoiseGenerator::range{-r, r};

template <size_t SAMPLES>
class ImpulseGenerator : public InputGenerator {
public:
    virtual util::aligned::vector<util::aligned::vector<cl_float>>
    compute_input(int size) override {
        auto ret = util::aligned::vector<util::aligned::vector<cl_float>>(
                SAMPLES, util::aligned::vector<cl_float>(size, 0));
        for (auto& i : ret.front()) {
            i = 0.25;
        }
        return ret;
    }
};

namespace testing {
constexpr auto sr{44100.0};

constexpr auto parallel_size{1 << 8};

std::default_random_engine engine{std::random_device()()};

auto random_filter_descriptor() {
    return waveguide::filter_descriptor{
            std::uniform_real_distribution<double>{0.1, 1}(engine),
            std::uniform_real_distribution<double>{0, 0.5}(engine),
            std::uniform_real_distribution<double>{0, 1}(engine)};
}

template <size_t... Ix>
auto random_filter_descriptors(std::index_sequence<Ix...>) {
    return std::array<waveguide::filter_descriptor, sizeof...(Ix)>{
            {((void)Ix, random_filter_descriptor())...}};
}

auto random_filter_descriptors() {
    return random_filter_descriptors(
            std::make_index_sequence<biquad_sections>{});
}

template <size_t... Ix>
auto compute_descriptors(std::index_sequence<Ix...>) {
    return std::array<std::array<waveguide::filter_descriptor, biquad_sections>,
                      sizeof...(Ix)>{
            {((void)Ix, random_filter_descriptors())...}};
}

auto compute_descriptors() {
    return compute_descriptors(std::make_index_sequence<parallel_size>{});
}

static const auto descriptors{compute_descriptors()};

enum class FilterType {
    biquad_cascade,
    single_reflectance,
    single_impedance,
};

auto compute_coeffs(
        std::integral_constant<FilterType, FilterType::biquad_cascade>) {
    return util::map(
            [](const auto& n) { return waveguide::get_peak_biquads_array(n); },
            descriptors);
}

auto compute_coeffs(
        std::integral_constant<FilterType, FilterType::single_reflectance>) {
    return util::map(
            [](const auto& n) {
                return waveguide::convolve(
                        waveguide::get_peak_biquads_array(n));
            },
            descriptors);
}

auto compute_coeffs(
        std::integral_constant<FilterType, FilterType::single_impedance>) {
    return util::map(
            [](const auto& n) {
                return waveguide::to_impedance_coefficients(waveguide::convolve(
                        waveguide::get_peak_biquads_array(n)));
            },
            descriptors);
}

template <typename Memory, testing::FilterType FT, typename Generator>
class kernel : Generator {
public:
    virtual ~kernel() noexcept = default;

    template <typename Kernel>
    auto run_kernel(Kernel k) {
        auto kernel = std::move(k);
        cl::CommandQueue queue{cc.context, cc.device};
        for (auto i = 0u; i != input.size(); ++i) {
            cl::copy(queue, input[i].begin(), input[i].end(), cl_input);

            kernel(cl::EnqueueArgs(queue, cl::NDRange(testing::parallel_size)),
                   cl_input,
                   cl_output,
                   cl_memory,
                   cl_coeffs);

            cl::copy(queue, cl_output, output[i].begin(), output[i].end());
        }
        auto buf = util::aligned::vector<cl_float>(output.size());
        std::transform(
                begin(output), end(output), buf.begin(), [](const auto& i) {
                    return i.front();
                });
        return buf;
    }

    const compute_context cc;
    const waveguide::program program{cc};
    util::aligned::vector<Memory> memory{testing::parallel_size, Memory{}};

    static auto compute_coeffs() {
        return testing::compute_coeffs(
                std::integral_constant<FilterType, FT>{});
    }

    decltype(compute_coeffs()) coeffs{compute_coeffs()};

    cl::Buffer cl_memory{cc.context, memory.begin(), memory.end(), false};
    cl::Buffer cl_coeffs{cc.context, coeffs.begin(), coeffs.end(), false};
    util::aligned::vector<util::aligned::vector<cl_float>> input{
            Generator::compute_input(testing::parallel_size)};
    util::aligned::vector<util::aligned::vector<cl_float>> output{
            input.size(),
            util::aligned::vector<cl_float>(testing::parallel_size, 0)};
    cl::Buffer cl_input{cc.context,
                        CL_MEM_READ_WRITE,
                        testing::parallel_size * sizeof(cl_float)};
    cl::Buffer cl_output{cc.context,
                         CL_MEM_READ_WRITE,
                         testing::parallel_size * sizeof(cl_float)};
};

template <typename Generator>
using rk_biquad = kernel<biquad_memory_array,
                         testing::FilterType::biquad_cascade,
                         Generator>;

template <typename Generator>
using rk_filter = kernel<memory_canonical,
                         testing::FilterType::single_reflectance,
                         Generator>;

template <typename Generator>
using rk_impedance = kernel<memory_canonical,
                            testing::FilterType::single_impedance,
                            Generator>;

class testing_rk_biquad : public rk_biquad<NoiseGenerator>,
                          public ::testing::Test {};
class testing_rk_filter : public rk_filter<NoiseGenerator>,
                          public ::testing::Test {};

TEST_F(testing_rk_biquad, filtering) {
    auto results{run_kernel(program.get_filter_test_kernel())};
    ASSERT_TRUE(std::none_of(begin(results), end(results), [](auto i) {
        return std::isnan(i);
    }));
    ASSERT_TRUE(std::none_of(begin(results), end(results), [](auto i) {
        return std::isinf(i);
    }));
    write("./filtered_noise.wav",
          audio_file::make_audio_file(results, testing::sr),
          16);
}

TEST_F(testing_rk_filter, filtering_2) {
    auto results = run_kernel(program.get_filter_test_2_kernel());
    ASSERT_TRUE(std::none_of(begin(results), end(results), [](auto i) {
        return std::isnan(i);
    }));
    ASSERT_TRUE(std::none_of(begin(results), end(results), [](auto i) {
        return std::isinf(i);
    }));
    write("./filtered_noise_2.wav",
          audio_file::make_audio_file(results, testing::sr),
          16);
}

class testing_rk_biquad_quiet : public rk_biquad<QuietNoiseGenerator>,
                                public ::testing::Test {};
class testing_rk_filter_quiet : public rk_filter<QuietNoiseGenerator>,
                                public ::testing::Test {};

TEST_F(testing_rk_biquad_quiet, filtering) {
    auto results = run_kernel(program.get_filter_test_kernel());
    ASSERT_TRUE(std::none_of(begin(results), end(results), [](auto i) {
        return std::isnan(i);
    }));
    ASSERT_TRUE(std::none_of(begin(results), end(results), [](auto i) {
        return std::isinf(i);
    }));
    write("./filtered_noise_quiet.wav",
          audio_file::make_audio_file(results, testing::sr),
          16);
}

TEST_F(testing_rk_filter_quiet, filtering_2) {
    auto results = run_kernel(program.get_filter_test_2_kernel());
    ASSERT_TRUE(std::none_of(begin(results), end(results), [](auto i) {
        return std::isnan(i);
    }));
    ASSERT_TRUE(std::none_of(begin(results), end(results), [](auto i) {
        return std::isinf(i);
    }));
    write("./filtered_noise_2_quiet.wav",
          audio_file::make_audio_file(results, testing::sr),
          16);
}

TEST(compare_filters, compare_filters) {
    const auto test = [](auto biquad, auto filter) {
        for (auto i = 0; i != biquad.input.size(); ++i) {
            for (auto j = 0; j != biquad.input[i].size(); ++j) {
                ASSERT_EQ(biquad.input[i][j], filter.input[i][j]);
            }
        }

        const auto buf_1 =
                biquad.run_kernel(biquad.program.get_filter_test_kernel());
        const auto buf_2 =
                filter.run_kernel(filter.program.get_filter_test_2_kernel());

        auto diff = buf_1;
        std::transform(std::begin(buf_1),
                       std::end(buf_1),
                       buf_2.begin(),
                       diff.begin(),
                       [](auto i, auto j) { return std::abs(i - j); });

        auto div = buf_1;
        std::transform(
                std::begin(buf_1),
                std::end(buf_1),
                buf_2.begin(),
                div.begin(),
                [](auto i, auto j) {
                    if (i == 0 || j == 0) {
                        return 0.0f;
                    }
                    return std::abs(util::decibels::a2db(std::abs(i / j)));
                });

        const auto max_diff{*std::max_element(begin(diff), end(diff))};

        ASSERT_TRUE(max_diff < 0.001) << max_diff;

        write("./buf_1.wav",
              audio_file::make_audio_file(
                      std::vector<float>(buf_1.begin(), buf_1.end()),
                      testing::sr),
              16);
        write("./buf_2.wav",
              audio_file::make_audio_file(
                      std::vector<float>(buf_2.begin(), buf_2.end()),
                      testing::sr),
              16);
    };

    using Imp = ImpulseGenerator<200>;

    test(rk_biquad<Imp>{}, rk_filter<Imp>{});
    test(rk_biquad<NoiseGenerator>{}, rk_filter<NoiseGenerator>{});
}

template <typename T>
util::aligned::vector<util::aligned::vector<T>> transpose(
        const util::aligned::vector<util::aligned::vector<T>>& t) {
    util::aligned::vector<util::aligned::vector<T>> ret(
            t.front().size(), util::aligned::vector<T>(t.size()));
    for (auto i = 0u; i != ret.size(); ++i) {
        for (auto j = 0u; j != ret.front().size(); ++j) {
            ret[i][j] = t[j][i];
        }
    }
    return ret;
}

}  // namespace testing
