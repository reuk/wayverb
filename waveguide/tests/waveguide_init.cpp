#include "waveguide/make_transparent.h"
#include "waveguide/mesh.h"
#include "waveguide/postprocessor/node.h"
#include "waveguide/preprocessor/soft_source.h"
#include "waveguide/waveguide.h"

#include "common/callback_accumulator.h"
#include "common/progress_bar.h"
#include "common/spatial_division/voxelised_scene_data.h"

#include "gtest/gtest.h"

#include <algorithm>

TEST(waveguide_init, waveguide_init) {
    const compute_context cc{};

    auto scene_data{geo::get_scene_data(geo::box{glm::vec3{-1}, glm::vec3{1}},
                                        make_surface(0.001, 0))};

    const auto voxelised{make_voxelised_scene_data(scene_data, 5, 0.1f)};

    constexpr glm::vec3 centre{0, 0, 0};

    const aligned::vector<float> input(20, 1);
    auto transparent{waveguide::make_transparent(input)};

    constexpr auto steps{100};
    transparent.resize(steps, 0);

    constexpr auto speed_of_sound{340.0};

    const auto model{
            waveguide::compute_mesh(cc, voxelised, 0.04, speed_of_sound)};
    const auto receiver_index{compute_index(model.get_descriptor(), centre)};

    auto prep{waveguide::preprocessor::make_soft_source(
            receiver_index, transparent.begin(), transparent.end())};

    callback_accumulator<waveguide::postprocessor::node> postprocessor{
            receiver_index};

    progress_bar pb{std::cout, steps};
    waveguide::run(cc,
                   model,
                   prep,
                   [&](auto& queue, const auto& buffer, auto step) {
                       postprocessor(queue, buffer, step);
                       pb += 1;
                   },
                   true);

    ASSERT_EQ(transparent.size(), postprocessor.get_output().size());

    for (auto i{0u}; i != input.size(); ++i) {
        ASSERT_NEAR(postprocessor.get_output()[i], input[i], 0.00001);
    }
}
