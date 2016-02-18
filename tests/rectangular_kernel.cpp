#include "rectangular_program.h"
#include "cl_common.h"
#include "timed_scope.h"
#include "db.h"

#include "write_audio_file.h"

#include "gtest/gtest.h"

#include <random>
#include <array>
#include <type_traits>

BiquadCoefficients get_notch_coefficients(float gain,
                                          float centre,
                                          float Q,
                                          float sr) {
    const float A = pow(10.0f, gain / 40.0f);
    const float w0 = 2.0f * M_PI * centre / sr;
    const float cw0 = cos(w0);
    const float sw0 = sin(w0);
    const float alpha = sw0 / 2.0f * Q;
    const float a0 = 1 + alpha / A;
    return BiquadCoefficients{
        {(1 + alpha * A) / a0, (-2 * cw0) / a0, (1 - alpha * A) / a0},
        {1, (-2 * cw0) / a0, (1 - alpha / A) / a0}};
}

BiquadCoefficientsArray get_notch_biquads_array(float sr) {
    auto engine = std::default_random_engine{std::random_device()()};
    auto range = std::uniform_real_distribution<cl_float>(-24, -24);
    auto centres = std::array<float, BiquadCoefficientsArray::BIQUAD_SECTIONS>{
        {45, 90, 180}};
    BiquadCoefficientsArray ret;
    std::transform(centres.begin(),
                   centres.end(),
                   std::begin(ret.array),
                   [&range, &engine, sr](auto i) {
                       return get_notch_coefficients(range(engine), i, 1, sr);
                   });
    return ret;
}

template <int A, int B>
FilterCoefficients<A + B> convolve(const FilterCoefficients<A>& a,
                                   const FilterCoefficients<B>& b) {
    auto ret = FilterCoefficients<A + B>{};
    for (auto i = 0; i != A + 1; ++i) {
        for (auto j = 0; j != B + 1; ++j) {
            ret.b[i + j] += a.b[i] * b.b[j];
            ret.a[i + j] += a.a[i] * b.a[j];
        }
    }
    return ret;
}

template <size_t I>
struct Indexer : std::integral_constant<decltype(I), I> {
    using Next = Indexer<I - 1>;
};

template <typename Func, typename A, typename T>
auto reduce(const A& a, const T&, Indexer<0>, const Func& = Func{}) {
    return a;
}

template <typename Func, typename A, typename T, typename Ind>
auto reduce(const A& a, const T& data, Ind i = Ind{}, const Func& f = Func{}) {
    return reduce(f(a, std::get<i - 1>(data)), data, typename Ind::Next{}, f);
}

template <typename Func, typename T>
auto reduce(const T& data, const Func& f = Func()) {
    return reduce(data.back(), data, Indexer<std::tuple_size<T>() - 1>{}, f);
}

auto convolve(const BiquadCoefficientsArray& a) {
    std::array<BiquadCoefficients, BiquadCoefficientsArray::BIQUAD_SECTIONS> t;
    std::copy(std::begin(a.array), std::end(a.array), t.begin());
    return reduce(t,
                  [](const auto& i, const auto& j) { return convolve(i, j); });
}

auto get_notch_filter_array(float sr) {
    return convolve(get_notch_biquads_array(sr));
}

template <typename T>
std::vector<T> compute_coeffs(int size, float sr) {
    return std::vector<T>(size);
}

template <>
std::vector<BiquadCoefficientsArray> compute_coeffs(int size, float sr) {
    auto ret = std::vector<BiquadCoefficientsArray>(size);
    std::generate(
        ret.begin(), ret.end(), [sr] { return get_notch_biquads_array(sr); });
    return ret;
}

using FC = FilterCoefficients<BiquadCoefficientsArray::BIQUAD_SECTIONS *
                              BiquadCoefficients::ORDER>;

template <>
std::vector<FC> compute_coeffs(int size, float sr) {
    auto ret = std::vector<FC>(size);
    std::generate(
        ret.begin(), ret.end(), [sr] { return get_notch_filter_array(sr); });
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

namespace std {
template <typename T>
ostream& operator<<(ostream& os, const vector<T>& t) {
    Bracketer bracketer(os);
    copy(t.begin(), t.end(), ostream_iterator<T>(os, "  "));
    return os;
}
}

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
                Logger::log(output[i]);
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
    static constexpr int sr{2000};
    std::vector<Memory> memory{size, Memory{}};
    std::vector<Coeffs> coeffs{compute_coeffs<Coeffs>(size, sr)};
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
    rectangular_kernel<BiquadMemoryArray, BiquadCoefficientsArray, Generator>;

template <typename Generator>
using rk_filter =
    rectangular_kernel<CanonicalMemory, CanonicalCoefficients, Generator>;

class testing_rk_biquad : public rk_biquad<NoiseGenerator>,
                          public ::testing::Test {};
class testing_rk_filter : public rk_filter<NoiseGenerator>,
                          public ::testing::Test {};

TEST_F(testing_rk_biquad, filtering) {
    write_sndfile("./filtered_noise.wav",
                  {run_kernel(program.get_filter_test_kernel())},
                  sr,
                  SF_FORMAT_PCM_16,
                  SF_FORMAT_WAV);
}

TEST_F(testing_rk_filter, filtering_2) {
    write_sndfile("./filtered_noise_2.wav",
                  {run_kernel(program.get_filter_test_2_kernel())},
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

        std::for_each(
            diff.begin(), diff.end(), [](auto i) { ASSERT_NEAR(i, 0, 0.001); });

        std::for_each(
            div.begin(), div.end(), [](auto i) { ASSERT_NEAR(i, 0, 24); });

        auto min_diff = *std::min_element(diff.begin(), diff.end());
        auto max_diff = *std::max_element(diff.begin(), diff.end());

        auto min_div = *std::min_element(div.begin(), div.end());
        auto max_div = *std::max_element(div.begin(), div.end());

        std::cout << "min diff: " << min_diff << std::endl;
        std::cout << "max diff: " << max_diff << std::endl;
        std::cout << "min div / dB: " << min_div << std::endl;
        std::cout << "max div / dB: " << max_div << std::endl;

        write_sndfile(
            "./buf_1.wav", {buf_1}, biquad.sr, SF_FORMAT_PCM_16, SF_FORMAT_WAV);

        write_sndfile(
            "./buf_2.wav", {buf_2}, filter.sr, SF_FORMAT_PCM_16, SF_FORMAT_WAV);
    };

    test(rk_biquad<ImpulseGenerator>{}, rk_filter<ImpulseGenerator>{});
    test(rk_biquad<NoiseGenerator>{}, rk_filter<NoiseGenerator>{});
}
