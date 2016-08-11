#include "compressed_waveguide.h"

#include "common/progress_bar.h"
#include "common/voxelised_scene_data.h"

#include "waveguide/make_transparent.h"
#include "waveguide/mesh/model.h"
#include "waveguide/postprocessor/microphone.h"
#include "waveguide/preprocessor/single_soft_source.h"
#include "waveguide/waveguide.h"

#include "gtest/gtest.h"

#include <cassert>
#include <cmath>

namespace {
auto uniform_surface(float r) {
    return surface{volume_type{{r, r, r, r, r, r, r, r}},
                   volume_type{{r, r, r, r, r, r, r, r}}};
}

template <typename T>
void multitest(T&& run) {
    constexpr auto iterations = 100;
    const auto proper_output = run();
    for (auto i = 0; i != iterations; ++i) {
        const auto output = run();
        ASSERT_EQ(output, proper_output);
    }
}
}  // namespace

TEST(verify_compensation_signal, verify_compensation_signal_compressed) {
    const aligned::vector<float> input{1, 2, 3, 4, 5, 4, 3, 2, 1};
    const auto transparent = waveguide::make_transparent(input);

    compute_context c;
    compressed_rectangular_waveguide waveguide(
            c.get_context(), c.get_device(), 100);

    multitest([&] {
        auto t = transparent;
        return waveguide.run_soft_source(std::move(t));
    });
}

TEST(verify_compensation_signal, verify_compensation_signal_normal) {
    const aligned::vector<float> input{1, 2, 3, 4, 5, 6, 5, 4, 3, 2, 1};
    const auto transparent = waveguide::make_transparent(input);

    compute_context cc;

    auto scene_data =
            geo::get_scene_data(geo::box(glm::vec3(-1), glm::vec3(1)));
    scene_data.set_surfaces(uniform_surface(0.5));
    const voxelised_scene_data voxelised(scene_data, 5, scene_data.get_aabb());

    const auto model = waveguide::mesh::compute_model(
            cc.get_context(), cc.get_device(), voxelised, 0.05);

    constexpr glm::vec3 centre{0, 0, 0};
    const auto receiver_index = compute_index(model.get_descriptor(), centre);

    multitest([&] {
        constexpr auto steps = 100;
        std::atomic_bool keep_going{true};
        progress_bar pb(std::cout, steps);

        const auto output = waveguide::run(cc.get_context(),
                                           cc.get_device(),
                                           model,
                                           receiver_index,
                                           input,
                                           receiver_index,
                                           [&](auto) { pb += 1; });

        assert(output.size() == steps);

        return map_to_vector(output, [](const auto& i) { return i.pressure; });
    });
}
