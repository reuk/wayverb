#include "combined/engine.h"

#include "common/cl/common.h"
#include "common/geo/box.h"
#include "common/model/receiver_settings.h"
#include "common/scene_data.h"

#include "gtest/gtest.h"

TEST(engine, engine) {
    constexpr glm::vec3 min{0, 0, 0};
    constexpr glm::vec3 max{5.56, 3.97, 2.81};
    const geo::box box{min, max};
    constexpr glm::vec3 source{2.09, 2.12, 2.12};
    constexpr glm::vec3 mic{2.09, 3.08, 0.96};
    constexpr auto output_sample_rate{96000.0};
    constexpr auto surface{make_surface(0.9, 0.9)};

    const auto scene_data{geo::get_scene_data(box, surface)};

    constexpr auto waveguide_sample_rate{8000};
    //    constexpr auto waveguide_filter_frequency = 4000;

    constexpr auto rays{1 << 15};
    constexpr auto impulses{128};

    const compute_context cc{};

    const wayverb::engine e{
            cc, scene_data, source, mic, waveguide_sample_rate, rays, impulses};

    std::cout << "finished engine init" << std::endl;

    const auto callback{[](auto state, auto progress) {
        std::cout << '\r' << std::setw(30) << to_string(state) << std::setw(10)
                  << progress << std::flush;
    }};

    const auto intermediate{e.run(true, callback)};

    std::cout << "\nfinished engine run" << std::endl;

    if (intermediate == nullptr) {
        throw std::runtime_error("failed to generate intermediate results");
    }

    const auto result{intermediate->attenuate(
            cc, model::receiver_settings{}, output_sample_rate, 20, callback)};

    std::cout << "\nfinished engine attenuate" << std::endl;
}
