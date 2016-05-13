#include "waveguide/config.h"
#include "waveguide/rectangular_program.h"
#include "waveguide/waveguide.h"

#include "common/cl_common.h"
#include "common/sinc.h"

#include "glog/logging.h"
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
    config.filter_frequency = 1000;
    config.source = source;
    config.mic = receiver;
    config.sample_rate = samplerate;

    //  init simulation parameters
    CuboidBoundary boundary(box.get_c0(), box.get_c1());

    auto scene_data = boundary.get_scene_data();
    scene_data.set_surfaces(surface);

    //  get a waveguide
    RectangularWaveguide waveguide(waveguide_program,
                                   context_info.queue,
                                   MeshBoundary(scene_data),
                                   config.get_divisions(),
                                   config.mic,
                                   config.get_waveguide_sample_rate());

    auto source_index = waveguide.get_index_for_coordinate(config.source);
    auto receiver_index = waveguide.get_index_for_coordinate(config.mic);

    CHECK(waveguide.inside(source_index)) << "source is outside of mesh!";
    CHECK(waveguide.inside(receiver_index)) << "receiver is outside of mesh!";

    auto corrected_source = waveguide.get_coordinate_for_index(source_index);

    ProgressBar pb(std::cout, steps);
    auto results = waveguide.init_and_run(
        corrected_source, std::vector<float>{1}, receiver_index, steps, [&pb] {
            pb += 1;
        });

    auto output = std::vector<float>(results.size());
    proc::transform(
        results, output.begin(), [](const auto& i) { return i.pressure; });

    auto max_amp = max_mag(output);
    std::cout << "max_mag: " << max_amp << std::endl;
}
