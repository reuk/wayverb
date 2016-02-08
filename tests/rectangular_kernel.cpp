#include "rectangular_program.h"
#include "cl_common.h"

#include "write_audio_file.h"

#include "gtest/gtest.h"

#include <random>
#include <array>

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

template <typename T, unsigned long I>
auto head(const std::array<T, I>& a) {
    return a.front();
}

template <typename T, unsigned long I>
auto tail(const std::array<T, I>& a) {
    auto ret = std::array<T, I - 1>{};
    std::copy(a.begin() + 1, a.end(), ret.begin());
    return ret;
}

template <typename Func, typename A, typename T>
A array_reduce(const A& a, const std::array<T, 0>& t, const Func& f = Func()) {
    return a;
}

template <typename Func, typename A, typename T, unsigned long I>
auto array_reduce(const A& a,
                  const std::array<T, I>& b,
                  const Func& f = Func()) {
    return array_reduce(f(a, head(b)), tail(b), f);
}

auto convolve(const BiquadCoefficientsArray& a) {
    std::array<BiquadCoefficients, BiquadCoefficientsArray::BIQUAD_SECTIONS> t;
    std::copy(std::begin(a.array), std::end(a.array), t.begin());
    return array_reduce(
        head(t),
        tail(t),
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

template <typename Memory, typename Coeffs>
class rectangular_kernel : public ::testing::Test {
public:
    virtual ~rectangular_kernel() noexcept = default;

    std::vector<std::vector<cl_float>> compute_input() {
        auto ret = std::vector<std::vector<cl_float>>{
            10000, std::vector<cl_float>(size)};
        for (auto& i : ret)
            std::generate(i.begin(), i.end(), [this] { return range(engine); });
        return ret;
    }

    template <typename Kernel>
    auto run_kernel(Kernel&& k) {
        auto kernel = std::move(k);

        auto tick = std::chrono::steady_clock::now();
        for (auto i = 0u; i != input.size(); ++i) {
            cl::copy(queue, input[i].begin(), input[i].end(), cl_input);

            kernel(cl::EnqueueArgs(queue, cl::NDRange(size)),
                   cl_input,
                   cl_output,
                   cl_memory,
                   cl_coeffs);

            cl::copy(queue, cl_output, ret[i].begin(), ret[i].end());
        }
        auto tock = std::chrono::steady_clock::now();
        std::cout << "filtering took: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(
                         tock - tick)
                         .count()
                  << " ms" << std::endl;

        auto buf = std::vector<cl_float>(ret.size());
        std::transform(ret.begin(),
                       ret.end(),
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
    std::default_random_engine engine{std::random_device()()};
    static constexpr float r{0.25};
    std::uniform_real_distribution<cl_float> range{-r, r};
    std::vector<std::vector<cl_float>> input{compute_input()};
    std::vector<std::vector<cl_float>> ret{input.size(),
                                           std::vector<cl_float>(size, 0)};
    cl::Buffer cl_input{context, CL_MEM_READ_WRITE, size * sizeof(cl_float)};
    cl::Buffer cl_output{context, CL_MEM_READ_WRITE, size * sizeof(cl_float)};
};

using rk_biquad =
    rectangular_kernel<BiquadMemoryArray, BiquadCoefficientsArray>;
TEST_F(rk_biquad, filtering) {
    write_sndfile("./filtered_noise.wav",
                  {run_kernel(program.get_filter_test_kernel())},
                  sr,
                  SF_FORMAT_PCM_16,
                  SF_FORMAT_WAV);
}

using rk_filter = rectangular_kernel<CanonicalMemory, CanonicalCoefficients>;
TEST_F(rk_filter, filtering_2) {
    write_sndfile("./filtered_noise_2.wav",
                  {run_kernel(program.get_filter_test_2_kernel())},
                  sr,
                  SF_FORMAT_PCM_16,
                  SF_FORMAT_WAV);
}
