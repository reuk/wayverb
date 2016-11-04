#include "waveguide/make_transparent.h"
#include "waveguide/mesh.h"
#include "waveguide/postprocessor/node.h"
#include "waveguide/preprocessor/soft_source.h"
#include "waveguide/waveguide.h"

#include "compensation_signal/waveguide.h"

#include "core/callback_accumulator.h"
#include "core/spatial_division/voxelised_scene_data.h"

#include "utilities/progress_bar.h"

#include "gtest/gtest.h"

#include <cassert>
#include <cmath>

using namespace wayverb::waveguide;
using namespace wayverb::core;

namespace {
template <typename T, typename U>
void multitest(T run, U input) {
    constexpr auto iterations = 100;
    const auto proper_output = run(input);
    for (auto i = 0ul; i != iterations; ++i) {
        const auto output = run(input);
        ASSERT_EQ(output, proper_output);
    }
}
}  // namespace

TEST(verify_compensation_signal, verify_compensation_signal_compressed) {
    const std::vector<float> input{1, 2, 3, 4, 5, 4, 3, 2, 1};
    const auto transparent{
            make_transparent(input.data(), input.data() + input.size())};

    const auto steps{100};

    compressed_rectangular_waveguide waveguide{compute_context{}, steps};
    multitest(
            [&](const auto& input) {
                return waveguide.run_soft_source(
                        begin(input), end(input), [&](auto step) {});
            },
            transparent);
}

TEST(verify_compensation_signal, verify_compensation_signal_normal) {
    const std::vector<float> input{1, 2, 3, 4, 5, 6, 5, 4, 3, 2, 1};
    auto transparent{
            make_transparent(input.data(), input.data() + input.size())};
    transparent.resize(100);

    const compute_context cc{};

    auto scene_data =
            geo::get_scene_data(geo::box(glm::vec3(-1), glm::vec3(1)),
                                make_surface<simulation_bands>(0.5, 0));
    const auto voxelised = make_voxelised_scene_data(
            scene_data,
            5,
            padded(geo::compute_aabb(scene_data), glm::vec3{0.1}));

    constexpr auto speed_of_sound = 340.0;
    const auto model = compute_mesh(cc, voxelised, 0.05, speed_of_sound);

    constexpr glm::vec3 centre{0, 0, 0};
    const auto receiver_index = compute_index(model.get_descriptor(), centre);

    multitest(
            [&](const auto& input) {
                callback_accumulator<postprocessor::node> postprocessor{
                        receiver_index};

                run(cc,
                    model,
                    preprocessor::make_soft_source(
                            receiver_index, begin(input), end(input)),
                    [&](auto& queue, const auto& buffer, auto step) {
                        postprocessor(queue, buffer, step);
                    },
                    true);

                assert(postprocessor.get_output().size() == transparent.size());

                return postprocessor.get_output();
            },
            transparent);
}
