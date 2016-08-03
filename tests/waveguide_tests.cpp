#include "waveguide/config.h"
#include "waveguide/program.h"
#include "waveguide/waveguide.h"

#include "common/cl_common.h"
#include "common/progress_bar.h"
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
                waveguide::program::FilterDescriptor{0, range(engine), 1.414};
        auto coefficients = waveguide::program::get_peak_coefficients(
                descriptor, samplerate);

        ASSERT_TRUE(proc::equal(coefficients.b, std::begin(coefficients.a)));
    }
}

TEST(run_waveguide, run_waveguide) {
    auto steps = 64000;

    compute_context cc;

    //  get opencl program
    waveguide::program waveguide_program(cc.get_context(), cc.get_device());

    box box(glm::vec3(0, 0, 0), glm::vec3(4, 3, 6));
    constexpr glm::vec3 source(1, 1, 1);
    constexpr glm::vec3 receiver(2, 1, 5);
    constexpr auto v = 0.5;
    constexpr surface surface{volume_type{{v, v, v, v, v, v, v, v}},
                              volume_type{{v, v, v, v, v, v, v, v}}};

    //  init simulation parameters

    auto scene_data = get_scene_data(box);
    scene_data.set_surfaces(surface);

    //  get a waveguide
    waveguide::waveguide waveguide(cc.get_context(),
                                   cc.get_device(),
                                   mesh_boundary(scene_data),
                                   receiver,
                                   4000);

    auto source_index   = waveguide.get_index_for_coordinate(source);
    auto receiver_index = waveguide.get_index_for_coordinate(receiver);

    CHECK(waveguide.inside(source_index)) << "source is outside of mesh!";
    CHECK(waveguide.inside(receiver_index)) << "receiver is outside of mesh!";

    auto corrected_source = waveguide.get_coordinate_for_index(source_index);

    std::atomic_bool keep_going{true};
    progress_bar pb(std::cout, steps);
    auto callback_counter{0};
    auto results = waveguide::init_and_run(waveguide,
                                           corrected_source,
                                           aligned::vector<float>{1},
                                           receiver_index,
                                           steps,
                                           keep_going,
                                           [&](auto i) {
                                               pb += 1;
                                               ASSERT_EQ(i, callback_counter++);
                                           });

    ASSERT_TRUE(results);

    auto output = aligned::vector<float>(results->size());
    proc::transform(
            *results, output.begin(), [](const auto& i) { return i.pressure; });

    auto max_amp = max_mag(output);
    std::cout << "max_mag: " << max_amp << std::endl;
}
