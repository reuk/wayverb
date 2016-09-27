#include "compressed_waveguide.h"

#include "waveguide/make_transparent.h"
#include "waveguide/mesh.h"
#include "waveguide/postprocessor/node.h"
#include "waveguide/preprocessor/soft_source.h"
#include "waveguide/waveguide.h"

#include "common/progress_bar.h"
#include "common/spatial_division/voxelised_scene_data.h"
#include "common/callback_accumulator.h"

#include "gtest/gtest.h"

#include <cassert>
#include <cmath>

constexpr auto speed_of_sound{340.0};

namespace {
template <typename T>
void multitest(T&& run) {
    constexpr auto iterations{100};
    const auto proper_output{run()};
    for (auto i{0ul}; i != iterations; ++i) {
        const auto output{run()};
        ASSERT_EQ(output, proper_output);
    }
}
}  // namespace

TEST(verify_compensation_signal, verify_compensation_signal_compressed) {
    const aligned::vector<float> input{1, 2, 3, 4, 5, 4, 3, 2, 1};
    const auto transparent{waveguide::make_transparent(input)};

    compute_context c;
    compressed_rectangular_waveguide waveguide(c, 100);

    multitest([&] {
        auto t{transparent};
        return waveguide.run_soft_source(std::move(t));
    });
}

TEST(verify_compensation_signal, verify_compensation_signal_normal) {
    const aligned::vector<float> input{1, 2, 3, 4, 5, 6, 5, 4, 3, 2, 1};
    auto transparent{waveguide::make_transparent(input)};
    transparent.resize(100);

    const compute_context cc{};

    auto scene_data{geo::get_scene_data(geo::box(glm::vec3(-1), glm::vec3(1)),
                                        make_surface(0.5, 0))};
    const auto voxelised{make_voxelised_scene_data(
            scene_data,
            5,
            util::padded(geo::get_aabb(scene_data), glm::vec3{0.1}))};

    const auto model{
            waveguide::compute_mesh(cc, voxelised, 0.05, speed_of_sound)};

    constexpr glm::vec3 centre{0, 0, 0};
    const auto receiver_index{compute_index(model.get_descriptor(), centre)};

    multitest([&] {
        auto prep{waveguide::preprocessor::make_soft_source(
                receiver_index, transparent.begin(), transparent.end())};

        callback_accumulator<waveguide::postprocessor::node> postprocessor{
                receiver_index};

        progress_bar pb(std::cout, transparent.size());
        waveguide::run(cc,
                       model,
                       prep,
                       [&](auto& queue, const auto& buffer, auto step) {
                           postprocessor(queue, buffer, step);
                           pb += 1;
                       },
                       true);

        assert(postprocessor.get_output().size() == transparent.size());

        return postprocessor.get_output();
    });
}
