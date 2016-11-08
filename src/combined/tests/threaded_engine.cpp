#include "combined/threaded_engine.h"

#include "gtest/gtest.h"

using namespace wayverb::combined;
using namespace wayverb::raytracer;
using namespace wayverb::waveguide;
using namespace wayverb::core;

TEST(threaded_engine, threaded_engine) {
    constexpr auto min = glm::vec3{0, 0, 0};
    constexpr auto max = glm::vec3{5.56, 3.97, 2.81};
    const auto box = geo::box{min, max};
    constexpr auto source = glm::vec3{2.09, 2.12, 2.12},
                   receiver = glm::vec3{2.09, 3.08, 0.96};
    constexpr auto surface = make_surface<simulation_bands>(0.1, 0.1);

    const auto scene_data = geo::get_scene_data(box, surface);

    model::scene model_scene{geo::compute_aabb(scene_data)};
    model_scene.sources[0].set_position(source);
    model_scene.receivers[0].set_position(receiver);

    complete_engine complete{};

    const engine_state_changed::scoped_connection connection{
            complete.add_engine_state_changed_callback(
                    [](auto state, auto progress) {
                        std::cout << '\r' << std::setw(30) << to_string(state)
                                  << std::setw(10) << progress << std::flush;
                    })};

    complete.run(compute_context{}, scene_data, model_scene);
}
