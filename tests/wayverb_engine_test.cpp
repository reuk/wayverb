#include "combined/engine.h"

#include "gtest/gtest.h"

using Engine = engine::WayverbEngine<BufferType::cl>;

TEST(engine, engine) {
    CuboidBoundary cuboid_boundary(glm::vec3(0, 0, 0),
                                   glm::vec3(5.56, 3.97, 2.81));
    constexpr glm::vec3 source(2.09, 2.12, 2.12);
    constexpr glm::vec3 mic(2.09, 3.08, 0.96);
    constexpr auto output_sample_rate = 96000;
    constexpr auto v = 0.9;
    constexpr Surface surface{{{v, v, v, v, v, v, v, v}},
                              {{v, v, v, v, v, v, v, v}}};

    auto scene_data = cuboid_boundary.get_scene_data();
    scene_data.set_surfaces(surface);

    constexpr auto waveguide_sample_rate = 8000;
    //    constexpr auto waveguide_filter_frequency = 4000;

    constexpr auto rays = 1024 * 32;
    constexpr auto impulses = 128;

    ComputeContext compute_context{};

    Engine engine{compute_context,
                  scene_data,
                  source,
                  mic,
                  waveguide_sample_rate,
                  rays,
                  impulses,
                  output_sample_rate};

    std::cout << "finished engine init" << std::endl;

    struct Callback {
        void operator()(engine::State state, double progress) const {
            std::cout << '\r' << std::setw(30) << to_string(state)
                      << std::setw(10) << progress << std::flush;
        }
    };

    std::atomic_bool keep_going{true};
    Callback callback;
    auto intermediate = engine.run(keep_going, callback);

    std::cout << std::endl;

    std::cout << "finished engine run" << std::endl;

    auto result = engine.attenuate(intermediate, callback);

    std::cout << "finished engine attenuate" << std::endl;
}
