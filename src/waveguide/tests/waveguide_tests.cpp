#include "waveguide/config.h"
#include "waveguide/filters.h"
#include "waveguide/fitted_boundary.h"
#include "waveguide/make_transparent.h"
#include "waveguide/mesh.h"
#include "waveguide/postprocessor/node.h"
#include "waveguide/preprocessor/soft_source.h"
#include "waveguide/program.h"
#include "waveguide/setup.h"
#include "waveguide/waveguide.h"

#include "core/callback_accumulator.h"
#include "core/cl/common.h"
#include "core/sinc.h"
#include "core/spatial_division/voxelised_scene_data.h"

#include "utilities/progress_bar.h"

#include "audio_file/audio_file.h"

#include "gtest/gtest.h"

#include <random>

using namespace wayverb::waveguide;
using namespace wayverb::core;

static constexpr auto samplerate = 44100.0;

TEST(peak_filter_coefficients, peak_filter_coefficients) {
    static std::default_random_engine engine{std::random_device()()};
    static std::uniform_real_distribution<cl_float> range{0, 0.5};
    for (auto i = 0; i != 10; ++i) {
        const auto descriptor = filter_descriptor{0, range(engine), 1.414};
        const auto coefficients = get_peak_coefficients(descriptor);

        ASSERT_TRUE(std::equal(std::begin(coefficients.b),
                               std::end(coefficients.b),
                               std::begin(coefficients.a)));
    }
}

TEST(run_waveguide, run_waveguide) {
    const auto steps = 700;

    const compute_context cc{};

    const geo::box box{glm::vec3{0, 0, 0}, glm::vec3{4, 3, 6}};
    constexpr glm::vec3 source{2, 1.5, 1};

    util::aligned::vector<glm::vec3> receivers{
            {2, 1.5, 2}, {2, 1.5, 3}, {2, 1.5, 4}, {2, 1.5, 5}};

    //  init simulation parameters

    const auto scene_data =
            geo::get_scene_data(box, make_surface<simulation_bands>(0.01, 0));

    constexpr auto speed_of_sound = 340.0;
    auto voxels_and_mesh = compute_voxels_and_mesh(
            cc, scene_data, source, samplerate, speed_of_sound);

    voxels_and_mesh.mesh.set_coefficients(to_flat_coefficients(0.01));

    //  get a waveguide

    const auto source_index =
            compute_index(voxels_and_mesh.mesh.get_descriptor(), source);

    if (!is_inside(voxels_and_mesh.mesh, source_index)) {
        throw std::runtime_error("Source is outside of mesh.");
    }

    const util::aligned::vector<float> raw_input{1.0f};
    auto input = make_transparent(raw_input.data(),
                                  raw_input.data() + raw_input.size());
    input.resize(steps);

    auto prep = preprocessor::make_soft_source(
            source_index, input.begin(), input.end());

    auto output_holders =
            util::map_to_vector(begin(receivers), end(receivers), [&](auto i) {
                const auto receiver_index =
                        compute_index(voxels_and_mesh.mesh.get_descriptor(), i);
                if (!is_inside(voxels_and_mesh.mesh, receiver_index)) {
                    throw std::runtime_error("Receiver is outside of mesh.");
                }
                return callback_accumulator<postprocessor::node>{
                        receiver_index};
            });

    util::progress_bar pb;
    auto callback_counter = 0;
    run(cc,
        voxels_and_mesh.mesh,
        [&](auto& queue, auto& buffer, auto step) {
            return prep(queue, buffer, step);
        },
        [&](auto& queue, const auto& buffer, auto step) {
            for (auto& i : output_holders) {
                i(queue, buffer, step);
            }
            set_progress(pb, step, steps);
            ASSERT_EQ(step, callback_counter++);
        },
        true);

    auto count = 0ul;
    for (const auto& output_holder : output_holders) {
        write(util::build_string("waveguide_receiver_", count++, ".wav")
                      .c_str(),
              output_holder.get_output(),
              samplerate,
              audio_file::format::wav,
              audio_file::bit_depth::pcm16);
    }

    const auto max_values = util::map_to_vector(
            begin(output_holders),
            end(output_holders),
            [](const auto& output_holder) {
                const auto output = output_holder.get_output();
                const auto begin = std::find_if(
                        std::begin(output), std::end(output), [](auto samp) {
                            return 0.0001 < std::abs(samp);
                        });
                const auto end =
                        begin +
                        std::min(std::distance(begin, output.end()), 100l);
                if (begin == end) {
                    return 0.0f;
                }
                return *std::max_element(begin, end);
            });

    for (auto val : max_values) {
        std::cout << "value: " << val << '\n';
    }
}
