#include "rectangular_program.h"
#include "cl_common.h"
#include "timed_scope.h"
#include "db.h"
#include "waveguide.h"
#include "testing_notches.h"

#include "write_audio_file.h"

#include "gtest/gtest.h"

#include <random>
#include <array>
#include <type_traits>

template <typename T>
std::vector<T> compute_coeffs(
    int size,
    const std::array<
        RectangularProgram::NotchFilterDescriptor,
        RectangularProgram::BiquadCoefficientsArray::BIQUAD_SECTIONS>& n,
    float sr) {
    return std::vector<T>(size);
}

template <>
std::vector<RectangularProgram::BiquadCoefficientsArray> compute_coeffs(
    int size,
    const std::array<
        RectangularProgram::NotchFilterDescriptor,
        RectangularProgram::BiquadCoefficientsArray::BIQUAD_SECTIONS>& n,
    float sr) {
    auto ret = std::vector<RectangularProgram::BiquadCoefficientsArray>(size);
    std::generate(ret.begin(),
                  ret.end(),
                  [&n, sr] {
                      return RectangularProgram::get_notch_biquads_array(n, sr);
                  });
    return ret;
}

using FC = RectangularProgram::FilterCoefficients<
    RectangularProgram::BiquadCoefficientsArray::BIQUAD_SECTIONS *
    RectangularProgram::BiquadCoefficients::ORDER>;

template <>
std::vector<FC> compute_coeffs(
    int size,
    const std::array<
        RectangularProgram::NotchFilterDescriptor,
        RectangularProgram::BiquadCoefficientsArray::BIQUAD_SECTIONS>& n,
    float sr) {
    auto ret = std::vector<FC>(size);
    std::generate(
        ret.begin(),
        ret.end(),
        [&n, sr] { return RectangularProgram::get_notch_filter_array(n, sr); });
    return ret;
}

class InputGenerator {
public:
    virtual std::vector<std::vector<cl_float>> compute_input(int size) = 0;
};

class NoiseGenerator : public InputGenerator {
public:
    virtual std::vector<std::vector<cl_float>> compute_input(
        int size) override {
        static auto ret = generate(size);
        return ret;
    }

private:
    static std::vector<std::vector<cl_float>> generate(int size) {
        auto ret = std::vector<std::vector<cl_float>>{
            10000, std::vector<cl_float>(size, 0)};
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
    virtual std::vector<std::vector<cl_float>> compute_input(
        int size) override {
        static auto ret = generate(size);
        return ret;
    }

private:
    static std::vector<std::vector<cl_float>> generate(int size) {
        auto ret = std::vector<std::vector<cl_float>>{
            40000, std::vector<cl_float>(size, 0)};
        for (auto& i : ret)
            std::generate(i.begin(), i.end(), [] { return range(engine); });
        return ret;
    }

    static std::default_random_engine engine;
    static constexpr float r{1e-35};
    static std::uniform_real_distribution<cl_float> range;
};

std::default_random_engine QuietNoiseGenerator::engine{std::random_device()()};
std::uniform_real_distribution<cl_float> QuietNoiseGenerator::range{-r, r};

class ImpulseGenerator : public InputGenerator {
public:
    virtual std::vector<std::vector<cl_float>> compute_input(
        int size) override {
        auto ret = std::vector<std::vector<cl_float>>{
            200, std::vector<cl_float>(size, 0)};
        for (auto& i : ret.front())
            i = 0.25;
        return ret;
    }
};

template <typename Memory, typename Coeffs, typename Generator>
class rectangular_kernel : Generator {
public:
    virtual ~rectangular_kernel() noexcept = default;

    template <typename Kernel>
    auto run_kernel(Kernel&& k) {
        auto kernel = std::move(k);

        {
            TimedScope timer("filtering");
            for (auto i = 0u; i != input.size(); ++i) {
                cl::copy(queue, input[i].begin(), input[i].end(), cl_input);

                kernel(cl::EnqueueArgs(queue, cl::NDRange(size)),
                       cl_input,
                       cl_output,
                       cl_memory,
                       cl_coeffs);

                cl::copy(queue, cl_output, output[i].begin(), output[i].end());
            }
        }
        auto buf = std::vector<cl_float>(output.size());
        std::transform(output.begin(),
                       output.end(),
                       buf.begin(),
                       [](const auto& i) { return i.front(); });
        return buf;
    }

    cl::Context context{get_context()};
    cl::Device device{get_device(context)};
    cl::CommandQueue queue{context, device};
    RectangularProgram program{
        get_program<RectangularProgram>(context, device)};
    static constexpr int size{1 << 8};
    static constexpr int sr{44100};
    std::vector<Memory> memory{size, Memory{}};
    std::array<RectangularProgram::NotchFilterDescriptor,
               RectangularProgram::BiquadCoefficientsArray::BIQUAD_SECTIONS>
        notches{Testing::notches};
    std::vector<Coeffs> coeffs{compute_coeffs<Coeffs>(size, notches, sr)};
    cl::Buffer cl_memory{context, memory.begin(), memory.end(), false};
    cl::Buffer cl_coeffs{context, coeffs.begin(), coeffs.end(), false};
    std::vector<std::vector<cl_float>> input{Generator::compute_input(size)};
    std::vector<std::vector<cl_float>> output{input.size(),
                                              std::vector<cl_float>(size, 0)};
    cl::Buffer cl_input{context, CL_MEM_READ_WRITE, size * sizeof(cl_float)};
    cl::Buffer cl_output{context, CL_MEM_READ_WRITE, size * sizeof(cl_float)};
};

template <typename Generator>
using rk_biquad =
    rectangular_kernel<RectangularProgram::BiquadMemoryArray,
                       RectangularProgram::BiquadCoefficientsArray,
                       Generator>;

template <typename Generator>
using rk_filter = rectangular_kernel<RectangularProgram::CanonicalMemory,
                                     RectangularProgram::CanonicalCoefficients,
                                     Generator>;

class testing_rk_biquad : public rk_biquad<NoiseGenerator>,
                          public ::testing::Test {};
class testing_rk_filter : public rk_filter<NoiseGenerator>,
                          public ::testing::Test {};

TEST_F(testing_rk_biquad, filtering) {
    auto results = run_kernel(program.get_filter_test_kernel());
    ASSERT_TRUE(log_nan(results, "filter 1 results") == results.end());
    write_sndfile(
        "./filtered_noise.wav", {results}, sr, SF_FORMAT_PCM_16, SF_FORMAT_WAV);
}

TEST_F(testing_rk_filter, filtering_2) {
    auto results = run_kernel(program.get_filter_test_2_kernel());
    ASSERT_TRUE(log_nan(results, "filter 2 results") == results.end());
    write_sndfile("./filtered_noise_2.wav",
                  {results},
                  sr,
                  SF_FORMAT_PCM_16,
                  SF_FORMAT_WAV);
}

class testing_rk_biquad_quiet : public rk_biquad<QuietNoiseGenerator>,
                                public ::testing::Test {};
class testing_rk_filter_quiet : public rk_filter<QuietNoiseGenerator>,
                                public ::testing::Test {};

TEST_F(testing_rk_biquad_quiet, filtering) {
    auto results = run_kernel(program.get_filter_test_kernel());
    ASSERT_TRUE(log_nan(results, "filter 1 quiet results") == results.end());
    write_sndfile("./filtered_noise_quiet.wav",
                  {results},
                  sr,
                  SF_FORMAT_PCM_16,
                  SF_FORMAT_WAV);
}

TEST_F(testing_rk_filter_quiet, filtering_2) {
    auto results = run_kernel(program.get_filter_test_2_kernel());
    ASSERT_TRUE(log_nan(results, "filter 2 quiet results") == results.end());
    write_sndfile("./filtered_noise_2_quiet.wav",
                  {results},
                  sr,
                  SF_FORMAT_PCM_16,
                  SF_FORMAT_WAV);
}

TEST(compare_filters, compare_filters) {
    /*
    Logger::log_err("cpu: sizeof(CanonicalMemory): ",
                    sizeof(CanonicalMemory));
    Logger::log_err("cpu: sizeof(BiquadMemoryArray): ",
                    sizeof(BiquadMemoryArray));
    Logger::log_err("cpu: sizeof(CanonicalCoefficients): ",
                    sizeof(CanonicalCoefficients));
    Logger::log_err("cpu: sizeof(BiquadCoefficientsArray): ",
                    sizeof(BiquadCoefficientsArray));
    */

    auto test = [](auto&& biquad, auto&& filter) {
        for (auto i = 0; i != biquad.input.size(); ++i) {
            for (auto j = 0; j != biquad.input[i].size(); ++j) {
                ASSERT_EQ(biquad.input[i][j], filter.input[i][j]);
            }
        }

        auto buf_1 = biquad.run_kernel(biquad.program.get_filter_test_kernel());
        auto buf_2 =
            filter.run_kernel(filter.program.get_filter_test_2_kernel());

        auto diff = buf_1;
        std::transform(buf_1.begin(),
                       buf_1.end(),
                       buf_2.begin(),
                       diff.begin(),
                       [](auto i, auto j) { return std::abs(i - j); });

        auto div = buf_1;
        std::transform(buf_1.begin(),
                       buf_1.end(),
                       buf_2.begin(),
                       div.begin(),
                       [](auto i, auto j) {
                           if (i == 0 || j == 0)
                               return 0.0;
                           return std::abs(a2db(std::abs(i / j)));
                       });

        /*
        std::for_each(
            diff.begin(), diff.end(), [](auto i) { ASSERT_NEAR(i, 0, 0.001); });
        */

        // auto min_diff = *std::min_element(diff.begin(), diff.end());
        auto max_diff = *std::max_element(diff.begin(), diff.end());

        ASSERT_TRUE(max_diff < 0.001) << max_diff;

        /*
        auto min_div = *std::min_element(div.begin(), div.end());
        auto max_div = *std::max_element(div.begin(), div.end());

        Logger::log_err("min diff: ", min_diff);
        Logger::log_err("max diff: ", max_diff);
        Logger::log_err("min div / dB: ", min_div);
        Logger::log_err("max div / dB: ", max_div);
        */

        write_sndfile(
            "./buf_1.wav", {buf_1}, biquad.sr, SF_FORMAT_PCM_16, SF_FORMAT_WAV);

        write_sndfile(
            "./buf_2.wav", {buf_2}, filter.sr, SF_FORMAT_PCM_16, SF_FORMAT_WAV);
    };

    test(rk_biquad<ImpulseGenerator>{}, rk_filter<ImpulseGenerator>{});
    test(rk_biquad<NoiseGenerator>{}, rk_filter<NoiseGenerator>{});
}
