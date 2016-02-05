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
    return BiquadCoefficients{(1 + alpha * A) / a0,
                              (-2 * cw0) / a0,
                              (1 - alpha * A) / a0,
                              (-2 * cw0) / a0,
                              (1 - alpha / A) / a0};
}

BiquadCoefficientsArray get_notch_array(float sr) {
    auto engine = std::default_random_engine{std::random_device()()};
    auto range = std::uniform_real_distribution<cl_float>(-24, 6);
    auto centres = std::array<float, BiquadCoefficientsArray::BIQUAD_SECTIONS>{
        {45, 90, 180}};
    BiquadCoefficientsArray ret;
    std::transform(centres.begin(),
                   centres.end(),
                   std::begin(ret.array),
                   [&range, &engine, sr](auto i) {
                       return get_notch_coefficients(range(engine), i, 0.5, sr);
                   });
    return ret;
}

float biquad_step(float input,
                  BiquadMemory * bm,
                  const BiquadCoefficients * bc) {
    float out = input * bc->b0 + bm->z1;
    bm->z1 = input * bc->b1 - bc->a1 * out + bm->z2;
    bm->z2 = input * bc->b2 - bc->a2 * out;
    return out;
}

float biquad_cascade(float input,
                     BiquadMemoryArray * bm,
                     const BiquadCoefficientsArray * bc) {
    for (int i = 0; i != BiquadMemoryArray::BIQUAD_SECTIONS; ++i) {
        input = biquad_step(input, bm->array + i, bc->array + i);
    }
    return input;
}

TEST(rectangular_kernel, filtering) {
    auto context = get_context();
    auto device = get_device(context);
    auto queue = cl::CommandQueue(context, device);
    auto program = get_program<RectangularProgram>(context, device);
    auto kernel = program.get_filter_test_kernel();

    auto size = 1 << 13;

    auto memory = std::vector<BiquadMemoryArray>(size, BiquadMemoryArray{});
    auto coeffs = std::vector<BiquadCoefficientsArray>(size);

    auto sr = 2000;

    std::generate(
        coeffs.begin(), coeffs.end(), [sr] { return get_notch_array(sr); });

    auto cl_memory = cl::Buffer(context, memory.begin(), memory.end(), false);
    auto cl_coeffs = cl::Buffer(context, coeffs.begin(), coeffs.end(), false);

    auto input = std::vector<std::vector<cl_float>>(
        10000, std::vector<cl_float>(size, 0));
    auto ret = std::vector<std::vector<cl_float>>(
        input.size(), std::vector<cl_float>(size, 0));

    auto cl_input =
        cl::Buffer(context, CL_MEM_READ_WRITE, size * sizeof(cl_float));
    auto cl_output =
        cl::Buffer(context, CL_MEM_READ_WRITE, size * sizeof(cl_float));

    auto engine = std::default_random_engine{std::random_device()()};
    auto r = 0.25;
    auto range = std::uniform_real_distribution<cl_float>(-r, r);
    for (auto & i : input) {
        std::generate(
            i.begin(), i.end(), [&range, &engine] { return range(engine); });
    }

    for (auto i = 0u; i != input.size(); ++i) {
        cl::copy(queue, input[i].begin(), input[i].end(), cl_input);

        kernel(cl::EnqueueArgs(queue, cl::NDRange(size)),
               cl_input,
               cl_output,
               cl_memory,
               cl_coeffs);

        cl::copy(queue, cl_output, ret[i].begin(), ret[i].end());
    }

    auto buf = std::vector<cl_float>(ret.size());
    std::transform(ret.begin(),
                   ret.end(),
                   buf.begin(),
                   [](const auto & i) { return i.front(); });

    write_sndfile(
        "./filtered_noise.wav", {buf}, sr, SF_FORMAT_PCM_16, SF_FORMAT_WAV);

    auto bma = BiquadMemoryArray{};

    auto buf2 = std::vector<cl_float>(input.size());
    std::transform(input.begin(),
                   input.end(),
                   buf2.begin(),
                   [&bma, &coeffs](const auto & i) {
                       return biquad_cascade(i.front(), &bma, &coeffs.front());
                   });
}
