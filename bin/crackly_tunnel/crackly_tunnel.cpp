#include "audio_file/audio_file.h"

#include "utilities/progress_bar.h"
#include "utilities/string_builder.h"

#include "core/attenuator/null.h"
#include "core/cl/common.h"
#include "core/dsp_vector_ops.h"
#include "core/environment.h"
#include "core/reverb_time.h"
#include "core/scene_data_loader.h"

#include "raytracer/hit_rate.h"
#include "raytracer/simulation_parameters.h"

#include "waveguide/simulation_parameters.h"

#include "combined/engine.h"
#include "combined/waveguide_base.h"

#include <iomanip>

#ifndef OBJ_PATH_TUNNEL
#define OBJ_PATH_TUNNEL ""
#endif

template <typename T>
constexpr T power(T t, size_t i) {
    return i == 0 ? 1 : t * power(t, i - 1);
}

int main(int /*argc*/, char** /*argv*/) {
    auto scene_data = wayverb::core::scene_with_extracted_surfaces(
            *wayverb::core::scene_data_loader{OBJ_PATH_TUNNEL}.get_scene_data(),
            util::aligned::unordered_map<
                    std::string,
                    wayverb::core::surface<wayverb::core::simulation_bands>>{});

    scene_data.set_surfaces(
            wayverb::core::make_surface<wayverb::core::simulation_bands>(0.1,
                                                                         0.1));

    const wayverb::core::environment environment{};

    const auto raytracer_params =
            wayverb::raytracer::make_simulation_parameters(
                    1,
                    0.1,
                    environment.speed_of_sound,
                    1000,
                    estimate_room_volume(scene_data),
                    3);

    std::cout << "required rays: " << raytracer_params.rays << '\n';

    wayverb::combined::engine engine{
            wayverb::core::compute_context{},
            scene_data,
            glm::vec3{0, 0, -1},
            glm::vec3{0, 0, 1},
            environment,
            raytracer_params,
            wayverb::combined::make_waveguide_ptr(
                    wayverb::waveguide::single_band_parameters{500, 0.6})};

    util::progress_bar pb{std::cout};

    engine.connect_engine_state_changed([&](auto state, auto progress) {
        std::cout << '\r' << std::setw(40) << to_string(state) << std::setw(40)
                  << progress;
    });

    const auto intermediate = engine.run(true);

    constexpr auto output_sr = 44100.0;
    auto results = intermediate->postprocess(wayverb::core::attenuator::null{},
                                             output_sr);

    const auto max_magnitude = wayverb::core::max_mag(results);
    const auto norm_factor = 1.0 / max_magnitude;

    wayverb::core::mul(results, norm_factor);

    write(util::build_string("crackly_", raytracer_params.rays, ".aif").c_str(),
          results,
          output_sr,
          audio_file::format::aif,
          audio_file::bit_depth::pcm24);
}
