#include "combined/engine.h"

#include "gtest/gtest.h"

static auto to_string(WayverbEngine::State state) {
    switch (state) {
        case WayverbEngine::State::starting_raytracer:
            return "starting raytracer";
        case WayverbEngine::State::running_raytracer:
            return "running raytracer";
        case WayverbEngine::State::finishing_raytracer:
            return "finishing raytracer";
        case WayverbEngine::State::starting_waveguide:
            return "starting waveguide";
        case WayverbEngine::State::running_waveguide:
            return "running waveguide";
        case WayverbEngine::State::finishing_waveguide:
            return "finishing waveguide";
        case WayverbEngine::State::postprocessing:
            return "postprocessing";
    }
}

TEST(engine, engine) {
    CuboidBoundary cuboid_boundary(Vec3f(0, 0, 0), Vec3f(5.56, 3.97, 2.81));
    constexpr Vec3f source(2.09, 2.12, 2.12);
    constexpr Vec3f mic(2.09, 3.08, 0.96);
    constexpr auto output_sample_rate = 96000;
    constexpr auto v = 0.9;
    constexpr Surface surface{{{v, v, v, v, v, v, v, v}},
                              {{v, v, v, v, v, v, v, v}}};

    auto scene_data = cuboid_boundary.get_scene_data();
    scene_data.set_surfaces(surface);

    MeshBoundary mesh_boundary{scene_data};

    constexpr auto waveguide_sample_rate = 16000;
    //    constexpr auto waveguide_filter_frequency = 4000;

    constexpr auto rays = 1024 * 32;
    constexpr auto impulses = 128;

    ComputeContext compute_context{};

    WayverbEngine engine{compute_context,
                         mesh_boundary,
                         source,
                         mic,
                         waveguide_sample_rate,
                         rays,
                         impulses,
                         output_sample_rate};

    std::cout << "finished engine init" << std::endl;

    struct Callback {
        void operator()(WayverbEngine::State state, double progress) const {
            std::cout << std::setw(30) << to_string(state) << std::setw(10)
                      << progress << std::endl;
        }
    };

    Callback callback;

    auto intermediate = engine.run(callback);

    std::cout << "finished engine run" << std::endl;

    auto result = engine.attenuate(intermediate, callback);

    std::cout << "finished engine attenuate" << std::endl;
}
