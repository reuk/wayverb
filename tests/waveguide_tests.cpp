#include "waveguide/config.h"
#include "waveguide/mesh/model.h"
#include "waveguide/mesh/setup.h"
#include "waveguide/program.h"
#include "waveguide/waveguide.h"
#include "waveguide/postprocessor/microphone.h"
#include "waveguide/preprocessor/single_soft_source.h"

#include "common/cl_common.h"
#include "common/progress_bar.h"
#include "common/sinc.h"
#include "common/voxelised_scene_data.h"

#include "glog/logging.h"
#include "gtest/gtest.h"

#include <random>

static constexpr auto samplerate{44100.0};

TEST(peak_filter_coefficients, peak_filter_coefficients) {
    static std::default_random_engine engine{std::random_device()()};
    static std::uniform_real_distribution<cl_float> range{0, samplerate / 2};
    for (auto i = 0; i != 10; ++i) {
        const auto descriptor =
                waveguide::filters::descriptor{0, range(engine), 1.414};
        const auto coefficients = waveguide::filters::get_peak_coefficients(
                descriptor, samplerate);

        ASSERT_TRUE(proc::equal(coefficients.b, std::begin(coefficients.a)));
    }
}

TEST(run_waveguide, run_waveguide) {
    auto steps = 64000;

    compute_context cc;

    //  get opencl program
    waveguide::program waveguide_program(cc.get_context(), cc.get_device());

    const geo::box box(glm::vec3(0, 0, 0), glm::vec3(4, 3, 6));
    constexpr glm::vec3 source(1, 1, 1);
    constexpr glm::vec3 receiver(2, 1, 5);
    constexpr auto v = 0.5;
    constexpr surface surface{volume_type{{v, v, v, v, v, v, v, v}},
                              volume_type{{v, v, v, v, v, v, v, v}}};

    //  init simulation parameters

    auto scene_data = geo::get_scene_data(box);
    scene_data.set_surfaces(surface);

    const voxelised_scene_data voxelised(scene_data, 5, scene_data.get_aabb());

    const auto model = waveguide::mesh::compute_model(
            cc.get_context(), cc.get_device(), voxelised, 0.03);

    //  get a waveguide

    const auto source_index = compute_index(model.get_descriptor(), source);
    const auto receiver_index = compute_index(model.get_descriptor(), receiver);

    CHECK(waveguide::mesh::setup::is_inside(
            model.get_structure().get_condensed_nodes()[source_index]))
            << "source is outside of mesh!";
    CHECK(waveguide::mesh::setup::is_inside(
            model.get_structure().get_condensed_nodes()[receiver_index]))
            << "receiver is outside of mesh!";

    std::atomic_bool keep_going{true};
    progress_bar pb(std::cout, steps);
    auto callback_counter{0};

    aligned::vector<float> input(1000);
    input[0] = 1;

    const auto results = waveguide::run(cc.get_context(),
                                        cc.get_device(),
                                        model,
                                        source_index,
                                        input,
                                        receiver_index,
                                        [&](auto i) {
                                            pb += 1;
                                            ASSERT_EQ(i, callback_counter++);
                                        });

    ASSERT_FALSE(results.empty());

    const auto output = map_to_vector(results, [](auto i) {return i.pressure;});

    const auto max_amp = max_mag(output);
    std::cout << "max_mag: " << max_amp << std::endl;
}
