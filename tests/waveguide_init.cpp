#include "waveguide/make_transparent.h"
#include "waveguide/rectangular_waveguide.h"
#include "common/progress_bar.h"

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

    auto scene_data = get_scene_data(box(glm::vec3(-1), glm::vec3(1)));
    scene_data.set_surfaces(uniform_surface(0.999));

    MeshBoundary boundary(scene_data);

    constexpr glm::vec3 centre{0, 0, 0};

    const aligned::vector<float> input(20, 1);
    const auto transparent = make_transparent(input);

    rectangular_waveguide a(
            cc.get_context(), cc.get_device(), boundary, centre, 20000);
    rectangular_waveguide b(
            cc.get_context(), cc.get_device(), boundary, centre, 20000);

    ASSERT_TRUE(a == b);

    auto run = [&](auto& waveguide) {
        auto receiver_index = waveguide.get_index_for_coordinate(centre);
        constexpr auto steps = 100;
        std::atomic_bool keep_going{true};
        progress_bar pb(std::cout, steps);
        const auto output = a.init_and_run(
                centre, transparent, receiver_index, steps, keep_going, [&pb] {
                    pb += 1;
                });
    };

    run(a);
    run(b);
}
