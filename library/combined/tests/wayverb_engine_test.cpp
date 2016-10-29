#include "combined/engine.h"

#include "common/cl/common.h"
#include "common/geo/box.h"
#include "common/model/receiver.h"
#include "common/scene_data.h"

#include "gtest/gtest.h"

TEST(engine, engine) {
    constexpr auto min = glm::vec3{0, 0, 0};
    constexpr auto max = glm::vec3{5.56, 3.97, 2.81};
    const auto box = geo::box{min, max};
    constexpr auto source = glm::vec3{2.09, 2.12, 2.12};
    constexpr auto mic = glm::vec3{2.09, 3.08, 0.96};
    constexpr auto output_sample_rate = 96000.0;
    constexpr auto surface = make_surface<simulation_bands>(0.1, 0.1);

    const auto scene_data = geo::get_scene_data(box, surface);

    constexpr auto waveguide_sample_rate = 8000;
    //    constexpr auto waveguide_filter_frequency = 4000;

    constexpr auto rays = 1 << 15;

    const compute_context cc{};

    const wayverb::engine e{
            cc, scene_data, source, mic, waveguide_sample_rate, rays};

    std::cout << "finished engine init" << '\n';

    const auto callback = [](auto state, auto progress) {
        std::cout << '\r' << std::setw(30) << to_string(state) << std::setw(10)
                  << progress << std::flush;
    };

    const auto intermediate = e.run(true, callback);

    std::cout << "\nfinished engine run" << '\n';

    if (intermediate == nullptr) {
        throw std::runtime_error("failed to generate intermediate results");
    }

    const auto result =
            intermediate->attenuate(model::receiver{}, output_sample_rate);

    std::cout << "\nfinished engine attenuate" << '\n';
}
