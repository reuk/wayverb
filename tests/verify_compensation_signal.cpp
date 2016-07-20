#include "compressed_waveguide.h"

#include "waveguide/make_transparent.h"
#include "waveguide/rectangular_waveguide.h"

#include "gtest/gtest.h"

#include <cassert>
#include <cmath>

namespace {
auto uniform_surface(float r) {
    return Surface{VolumeType{{r, r, r, r, r, r, r, r}},
                   VolumeType{{r, r, r, r, r, r, r, r}}};
}

template <typename T>
void multitest(T&& run) {
    const auto proper_output = run();
    for (auto i = 0; i != 100; ++i) {
        const auto output = run();
        ASSERT_EQ(output, proper_output);
    }
}
}  // namespace

TEST(verify_compensation_signal, verify_compensation_signal_compressed) {
    const std::vector<float> input{1, 2, 3, 4, 5, 4, 3, 2, 1};
    const auto transparent = make_transparent(input);

    compute_context c;
    compressed_rectangular_waveguide waveguide(
            c.get_context(), c.get_device(), 100);

    multitest([&] {
        auto t = transparent;
        return waveguide.run_soft_source(std::move(t));
    });
}

TEST(verify_compensation_signal, verify_compensation_signal_normal) {
    const std::vector<float> input(20, 1);
    const auto transparent = make_transparent(input);

    compute_context cc;
    CuboidBoundary cuboid_boundary(glm::vec3(-1), glm::vec3(1));

    rectangular_program waveguide_program(cc.get_context(), cc.get_device());

    auto scene_data = cuboid_boundary.get_scene_data();
    scene_data.set_surfaces(uniform_surface(0.999));

    constexpr glm::vec3 centre{0, 0, 0};

    RectangularWaveguide waveguide(cc.get_context(),
                                   cc.get_device(),
                                   MeshBoundary(scene_data),
                                   centre,
                                   20000);

    auto receiver_index = waveguide.get_index_for_coordinate(centre);

    multitest([&] {
        constexpr auto steps = 100;
        std::atomic_bool keep_going{true};
        ProgressBar pb(std::cout, steps);
        const auto output = waveguide.init_and_run(
                centre, transparent, receiver_index, steps, keep_going, [&pb] {
                    pb += 1;
                });

        std::vector<float> pressures;
        pressures.reserve(output.size());
        for (const auto& i : output) {
            pressures.push_back(i.pressure);
        }

        return pressures;
    });
}
