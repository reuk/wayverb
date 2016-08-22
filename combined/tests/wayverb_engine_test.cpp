#include "combined/engine.h"

#include "common/cl/common.h"
#include "common/geo/box.h"
#include "common/model/receiver_settings.h"
#include "common/scene_data.h"

#include "gtest/gtest.h"

TEST(engine, engine) {
    geo::box box(glm::vec3(0, 0, 0), glm::vec3(5.56, 3.97, 2.81));
    constexpr glm::vec3 source(2.09, 2.12, 2.12);
    constexpr glm::vec3 mic(2.09, 3.08, 0.96);
    constexpr auto output_sample_rate = 96000;
    constexpr auto v                  = 0.9;
    constexpr surface surface{volume_type{{v, v, v, v, v, v, v, v}},
                              volume_type{{v, v, v, v, v, v, v, v}}};

    auto scene_data = geo::get_scene_data(box);
    scene_data.set_surfaces(surface);

    constexpr auto waveguide_sample_rate = 8000;
    //    constexpr auto waveguide_filter_frequency = 4000;

    constexpr auto rays     = 1024 * 32;
    constexpr auto impulses = 128;

    compute_context cc;

    wayverb::engine e(
            cc, scene_data, source, mic, waveguide_sample_rate, rays, impulses);

    std::cout << "finished engine init" << std::endl;

    auto callback = [](auto state, auto progress) {
        std::cout << '\r' << std::setw(30) << to_string(state) << std::setw(10)
                  << progress << std::flush;
    };

    std::atomic_bool keep_going{true};
    auto intermediate = e.run(keep_going, callback);

    std::cout << "\nfinished engine run" << std::endl;

    if (intermediate == nullptr) {
        throw std::runtime_error("failed to generate intermediate results");
    }

    auto result = intermediate->attenuate(
            cc, model::ReceiverSettings{}, output_sample_rate, callback);

    std::cout << "\nfinished engine attenuate" << std::endl;
}
