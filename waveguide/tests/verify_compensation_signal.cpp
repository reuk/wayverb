#include "compressed_waveguide.h"

#include "common/progress_bar.h"
#include "common/spatial_division/voxelised_scene_data.h"

#include "waveguide/make_transparent.h"
#include "waveguide/mesh.h"
#include "waveguide/postprocessor/microphone.h"
#include "waveguide/preprocessor/single_soft_source.h"
#include "waveguide/waveguide.h"

#include "gtest/gtest.h"

#include <cassert>
#include <cmath>

constexpr auto speed_of_sound{340.0};
constexpr auto acoustic_impedance{400.0};

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
    compressed_rectangular_waveguide waveguide(c, 100);

    multitest([&] {
        auto t = transparent;
        return waveguide.run_soft_source(std::move(t));
    });
}

TEST(verify_compensation_signal, verify_compensation_signal_normal) {
    const aligned::vector<float> input{1, 2, 3, 4, 5, 6, 5, 4, 3, 2, 1};
    auto transparent{waveguide::make_transparent(input)};
    transparent.resize(100);

    const compute_context cc{};

    auto scene_data{geo::get_scene_data(geo::box(glm::vec3(-1), glm::vec3(1)))};
    scene_data.set_surfaces(uniform_surface(0.5));
    const voxelised_scene_data voxelised{
            scene_data, 5, util::padded(scene_data.get_aabb(), glm::vec3{0.1})};

    const auto model{
            waveguide::compute_mesh(cc, voxelised, 0.05, speed_of_sound)};

    constexpr glm::vec3 centre{0, 0, 0};
    const auto receiver_index{compute_index(model.get_descriptor(), centre)};

    multitest([&] {
        progress_bar pb(std::cout, transparent.size());
        const auto output{waveguide::run(cc,
                                         model,
                                         receiver_index,
                                         transparent,
                                         receiver_index,
                                         speed_of_sound,
                                         acoustic_impedance,
                                         [&](auto) { pb += 1; })};

        assert(output.size() == transparent.size());

        return map_to_vector(output, [](const auto& i) { return i.pressure; });
    });
}
