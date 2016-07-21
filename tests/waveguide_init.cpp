#include "waveguide/make_transparent.h"
#include "waveguide/rectangular_waveguide.h"

#include "gtest/gtest.h"

#include <algorithm>

namespace {
auto uniform_surface(float r) {
    return Surface{VolumeType{{r, r, r, r, r, r, r, r}},
                   VolumeType{{r, r, r, r, r, r, r, r}}};
}
}  // namespace

TEST(waveguide_init, waveguide_init) {
    compute_context cc;
    CuboidBoundary cuboid_boundary(glm::vec3(-1), glm::vec3(1));

    auto scene_data = cuboid_boundary.get_scene_data();
    scene_data.set_surfaces(uniform_surface(0.999));

    MeshBoundary boundary(scene_data);

    constexpr glm::vec3 centre{0, 0, 0};

    const aligned::vector<float> input(20, 1);
    const auto transparent = make_transparent(input);

    RectangularWaveguide a(
            cc.get_context(), cc.get_device(), boundary, centre, 20000);
    RectangularWaveguide b(
            cc.get_context(), cc.get_device(), boundary, centre, 20000);

    ASSERT_TRUE(a == b);

    auto run = [&](auto& waveguide) {
        auto receiver_index = waveguide.get_index_for_coordinate(centre);
        constexpr auto steps = 100;
        std::atomic_bool keep_going{true};
        ProgressBar pb(std::cout, steps);
        const auto output = a.init_and_run(
                centre, transparent, receiver_index, steps, keep_going, [&pb] {
                    pb += 1;
                });
    };

    run(a);
    run(b);
}
