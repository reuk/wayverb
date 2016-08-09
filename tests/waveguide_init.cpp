#include "waveguide/make_transparent.h"
#include "waveguide/waveguide.h"
#include "common/progress_bar.h"

#include "gtest/gtest.h"

#include <algorithm>

namespace {
auto uniform_surface(float r) {
    return surface{volume_type{{r, r, r, r, r, r, r, r}},
                   volume_type{{r, r, r, r, r, r, r, r}}};
}
}  // namespace

TEST(waveguide_init, waveguide_init) {
    compute_context cc;

    auto scene_data =
            geo::get_scene_data(geo::box(glm::vec3(-1), glm::vec3(1)));
    scene_data.set_surfaces(uniform_surface(0.999));

    mesh_boundary boundary(scene_data);

    constexpr glm::vec3 centre{0, 0, 0};

    const aligned::vector<float> input(20, 1);
    const auto transparent = waveguide::make_transparent(input);

    waveguide::waveguide a(
            cc.get_context(), cc.get_device(), boundary, centre, 20000);
    waveguide::waveguide b(
            cc.get_context(), cc.get_device(), boundary, centre, 20000);

    auto run = [&](auto& waveguide) {
        auto receiver_index = waveguide.get_index_for_coordinate(centre);
        constexpr auto steps = 100;
        std::atomic_bool keep_going{true};
        progress_bar pb(std::cout, steps);
        const auto output = waveguide::init_and_run(a,
                                                    centre,
                                                    transparent,
                                                    receiver_index,
                                                    steps,
                                                    keep_going,
                                                    [&](auto) { pb += 1; });
    };

    run(a);
    run(b);
}
