#include "combined/engine.h"

#include "core/cl/common.h"
#include "core/geo/box.h"
#include "core/scene_data.h"

#include "gtest/gtest.h"

using namespace wayverb::combined;
using namespace wayverb::raytracer;
using namespace wayverb::waveguide;
using namespace wayverb::core;

TEST(engine, engine) {
    constexpr auto min = glm::vec3{0, 0, 0};
    constexpr auto max = glm::vec3{5.56, 3.97, 2.81};
    const auto box = geo::box{min, max};
    constexpr auto source = glm::vec3{2.09, 2.12, 2.12},
                   receiver = glm::vec3{2.09, 3.08, 0.96};
    constexpr auto output_sample_rate = 96000.0;
    constexpr auto surface = make_surface<simulation_bands>(0.1, 0.1);

    const auto scene_data = geo::get_scene_data(box, surface);

    auto e = make_engine(compute_context{},
                         scene_data,
                         source,
                         receiver,
                         wayverb::core::environment{},
                         simulation_parameters{1 << 16, 5},
                         single_band_parameters{10000, 0.5});

    const engine_state_changed::scoped_connection connection =
            e.add_engine_state_changed_callback([](auto state, auto progress) {
                std::cout << '\r' << std::setw(30) << to_string(state)
                          << std::setw(10) << progress << std::flush;
            });

    const auto intermediate = e.run(true);

    if (intermediate == nullptr) {
        throw std::runtime_error{"failed to generate intermediate results"};
    }

    const auto result =
            intermediate->postprocess(attenuator::null{}, output_sample_rate);
}
