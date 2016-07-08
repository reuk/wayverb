#include "compressed_waveguide.h"

#include "waveguide/make_transparent.h"
#include "waveguide/rectangular_waveguide.h"

#include "gtest/gtest.h"

#include <cassert>
#include <cmath>

TEST(verify_compensation_signal, verify_compensation_signal_compressed) {
    ComputeContext c;
    compressed_rectangular_waveguide_program program(c.context, c.device);
    compressed_rectangular_waveguide waveguide(program, 100);

    std::vector<float> input{1, 2, 3, 4, 5, 4, 3, 2, 1};
    auto output = waveguide.run_soft_source(make_transparent(input));

    auto lim = std::max(input.size(), output.size());
    for (auto i = 0u; i != lim; ++i) {
        auto in = i < input.size() ? input[i] : 0.0;
        auto out = i < output.size() ? output[i] : 0.0;
        ASSERT_TRUE(std::fabs(in - out) < 0.001f);
    }
}

namespace {
auto uniform_surface(float r) {
    return Surface{VolumeType{{r, r, r, r, r, r, r, r}},
                   VolumeType{{r, r, r, r, r, r, r, r}}};
}
}  // namespace

TEST(verify_compensation_signal, verify_compensation_signal_normal) {
    ComputeContext compute_context;
    CuboidBoundary cuboid_boundary(glm::vec3(-1), glm::vec3(1));

    RectangularProgram waveguide_program(compute_context.context,
                                         compute_context.device);

    auto scene_data = cuboid_boundary.get_scene_data();
    scene_data.set_surfaces(uniform_surface(0.999));

    glm::vec3 centre{0, 0, 0};

    RectangularWaveguide<BufferType::cl> waveguide(waveguide_program,
                                                   MeshBoundary(scene_data),
                                                   centre,
                                                   20000);

    auto receiver_index = waveguide.get_index_for_coordinate(centre);

    std::vector<float> input{1, 2, 3, 4, 5, 4, 3, 2, 1};
    std::atomic_bool keep_going{true};
    auto steps = 100;
    ProgressBar pb(std::cout, steps);
    auto output = waveguide.init_and_run(centre,
                                         make_transparent(input),
                                         receiver_index,
                                         steps,
                                         keep_going,
                                         [&pb] { pb += 1; });

    std::vector<float> pressures;
    pressures.reserve(output.size());
    for (const auto& i : output) {
        pressures.push_back(i.pressure);
    }

    auto lim = std::max(input.size(), output.size());
    for (auto i = 0u; i != lim; ++i) {
        auto in = i < input.size() ? input[i] : 0.0;
        auto out = i < pressures.size() ? pressures[i] : 0.0;
        ASSERT_TRUE(std::fabs(in - out) < 0.001f);
    }
}
