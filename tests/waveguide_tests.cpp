#include "cl_common.h"
#include "rectangular_program.h"
#include "sinc.h"
#include "waveguide.h"
#include "waveguide_config.h"

#include "gtest/gtest.h"

#include <random>

static constexpr auto samplerate{44100.0};

TEST(peak_filter_coefficients, peak_filter_coefficients) {
    static std::default_random_engine engine{std::random_device()()};
    static std::uniform_real_distribution<cl_float> range{0, samplerate / 2};
    for (auto i = 0; i != 10; ++i) {
        auto descriptor =
            RectangularProgram::FilterDescriptor{0, range(engine), 1.414};
        auto coefficients =
            RectangularProgram::get_peak_coefficients(descriptor, samplerate);

        ASSERT_TRUE(proc::equal(coefficients.b, std::begin(coefficients.a)));
    }
}

TEST(filter_coefficients, filter_coefficients) {
    constexpr auto g = 0.99;
    constexpr auto surface =
        Surface{{{g, g, g, g, g, g, g, g}}, {{g, g, g, g, g, g, g, g}}};
    static_assert(validate_surface(surface), "invalid surface");
}

TEST(run_waveguide, run_waveguide) {
    auto steps = 64000;

    ComputeContext context_info;

    //  get opencl program
    auto waveguide_program = get_program<RectangularProgram>(
        context_info.context, context_info.device);

    constexpr Box box(Vec3f(0, 0, 0), Vec3f(4, 3, 6));
    constexpr Vec3f source(1, 1, 1);
    constexpr Vec3f receiver(2, 1, 5);
    constexpr auto v = 0.5;
    constexpr Surface surface{{{v, v, v, v, v, v, v, v}},
                              {{v, v, v, v, v, v, v, v}}};

    config::Waveguide config;
    config.get_filter_frequency() = 1000;
    config.get_source() = source;
    config.get_mic() = receiver;
    config.get_output_sample_rate() = samplerate;

    //  init simulation parameters
    CuboidBoundary boundary(box.get_c0(), box.get_c1(), {surface});

    //  get a waveguide
    RectangularWaveguide waveguide(waveguide_program,
                                   context_info.queue,
                                   boundary,
                                   config.get_divisions(),
                                   config.get_mic(),
                                   config.get_waveguide_sample_rate());

    auto source_index = waveguide.get_index_for_coordinate(config.get_source());
    auto receiver_index = waveguide.get_index_for_coordinate(config.get_mic());

    CHECK(waveguide.inside(source_index)) << "source is outside of mesh!";
    CHECK(waveguide.inside(receiver_index)) << "receiver is outside of mesh!";

    auto corrected_source = waveguide.get_coordinate_for_index(source_index);

    ProgressBar pb(std::cout, steps);
    auto results = waveguide.run_basic(corrected_source,
                                       receiver_index,
                                       steps,
                                       config.get_waveguide_sample_rate(),
                                       [&pb] { pb += 1; });

    auto output = std::vector<float>(results.size());
    proc::transform(
        results, output.begin(), [](const auto& i) { return i.pressure; });

    auto max_amp = max_mag(output);
    std::cout << "max_mag: " << max_amp << std::endl;
}
