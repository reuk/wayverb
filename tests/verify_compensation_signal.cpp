#include "compressed_waveguide.h"

#include "common/progress_bar.h"

#include "waveguide/make_transparent.h"
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
    const auto proper_output  = run();
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
    waveguide::mesh_boundary boundary(scene_data);

    constexpr glm::vec3 centre{0, 0, 0};

    waveguide::waveguide waveguide(
            cc.get_context(), cc.get_device(), boundary, centre, 20000);

    auto receiver_index = waveguide.get_index_for_coordinate(centre);

    multitest([&] {
        constexpr auto steps = 100;
        std::atomic_bool keep_going{true};
        progress_bar pb(std::cout, steps);
        const auto output = waveguide::init_and_run(waveguide,
                                                    centre,
                                                    transparent,
                                                    receiver_index,
                                                    steps,
                                                    keep_going,
                                                    [&](auto) { pb += 1; });

        assert(output);

        aligned::vector<float> pressures;
        pressures.reserve(output->size());
        for (const auto& i : *output) {
            pressures.push_back(i.pressure);
        }

        return pressures;
    });
}
