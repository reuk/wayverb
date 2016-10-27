#include "waveguide/mesh.h"
#include "waveguide/postprocessor/directional_receiver.h"
#include "waveguide/preprocessor/gaussian.h"
#include "waveguide/waveguide.h"

#include "common/callback_accumulator.h"

#include "utilities/progress_bar.h"

#include "gtest/gtest.h"

TEST(nan_in_waveguide, nan_in_waveguide) {
    const auto filter_frequency = 11025.0;
    const auto oversample_ratio = 1.0;
    const auto waveguide_sr = filter_frequency * oversample_ratio * (1 / 0.196);

    const compute_context cc{device_type::gpu};

    const glm::vec3 mic{0, 0, 0};

    const auto s = 1.5f;
    const geo::box box{glm::vec3{-s}, glm::vec3{s}};
    const auto r = 0.9f;
    auto scene_data =
            geo::get_scene_data(box, make_surface<simulation_bands>(r, r));

    constexpr auto speed_of_sound = 340.0;
    constexpr auto acoustic_impedance = 400.0;

    const auto voxels_and_mesh = waveguide::compute_voxels_and_mesh(
            cc, scene_data, mic, waveguide_sr, speed_of_sound);

    const auto& model{voxels_and_mesh.mesh};

    const auto angle = M_PI;

    const glm::vec3 source{std::sin(angle), 0, std::cos(angle)};
    const auto dist = glm::distance(source, mic);
    const auto time_between_source_receiver = dist / speed_of_sound;
    const size_t required_steps = time_between_source_receiver * waveguide_sr;

    const auto receiver_index = compute_index(model.get_descriptor(), mic);

    const auto steps = 2 * required_steps;

    //  hacıhabiboglu2010 the pulse had a variance of 4 spatial samples
    const auto variance = 4 * model.get_descriptor().spacing;
    //  standard deviation is the sqrt of the variance
    const waveguide::preprocessor::gaussian generator{
            model.get_descriptor(), source, std::sqrt(variance), steps};

    callback_accumulator<waveguide::postprocessor::directional_receiver>
            postprocessor{model.get_descriptor(),
                          waveguide_sr,
                          acoustic_impedance / speed_of_sound,
                          receiver_index};

    std::cout << "running " << steps << " steps" << std::endl;

    progress_bar pb{std::cout};
    waveguide::run(cc,
                   model,
                   generator,
                   [&](auto& queue, const auto& buffer, auto step) {
                       postprocessor(queue, buffer, step);
                       set_progress(pb, step, steps);
                   },
                   true);
}
