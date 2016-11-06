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

    auto complete = complete_engine{};

    const auto connection = complete.add_scoped_engine_state_changed_callback(
            [](auto state, auto progress) {
                std::cout << '\r' << std::setw(30) << to_string(state)
                          << std::setw(10) << progress << std::flush;
            });

    std::vector<capsule_info> capsules;
    capsules.emplace_back(capsule_info{
            "the_capsule", make_capsule_ptr(attenuator::microphone{})});

    std::vector<receiver_info> receivers;
    receivers.emplace_back(
            receiver_info{"the_receiver", receiver, std::move(capsules)});

    complete.run(compute_context{},
                 scene_data,
                 {source_info{"the_source", source}},
                 std::move(receivers),
                 wayverb::core::environment{},
                 simulation_parameters{1 << 16, 5},
                 single_band_parameters{10000, 0.5},
                 output_info{".", "threaded_test", 44100, 16},
                 true);
}
