#include "waveguide/boundary_adjust.h"
#include "waveguide/config.h"
#include "waveguide/make_transparent.h"
#include "waveguide/mesh.h"
#include "waveguide/postprocessor/node.h"
#include "waveguide/preprocessor/soft_source.h"
#include "waveguide/surface_filters.h"
#include "waveguide/waveguide.h"

#include "common/azimuth_elevation.h"
#include "common/callback_accumulator.h"
#include "common/cl/common.h"
#include "common/conversions.h"
#include "common/dc_blocker.h"
#include "common/filters_common.h"
#include "common/kernel.h"
#include "common/map_to_vector.h"
#include "common/maximum_length_sequence.h"
#include "common/progress_bar.h"
#include "common/reverb_time.h"
#include "common/scene_data.h"
#include "common/serialize/surface.h"
#include "common/sinc.h"
#include "common/spatial_division/voxelised_scene_data.h"

#include "audio_file/audio_file.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <random>

constexpr auto speed_of_sound{340.0};

aligned::vector<float> make_mls(size_t length) {
    const size_t order = std::floor(std::log(length) / std::log(2)) + 1;
    aligned::vector<float> ret;
    ret.reserve(std::pow(2, order));
    generate_maximum_length_sequence(
            order, [&](auto value, auto step) { ret.emplace_back(value); });
    return ret;
}

int main(int argc, char** argv) {
    try {
        const glm::vec3 source{4.8, 2.18, 2.12};
        const glm::vec3 receiver{3.91, 1.89, 1.69};
        const auto sampling_frequency{8000.0};

        const auto surface{make_surface(0.2, 0)};

        const compute_context cc{};
        const geo::box boundary{glm::vec3{0}, glm::vec3{5.56, 3.97, 2.81}};

        const auto scene_data{geo::get_scene_data(boundary, surface)};

        const auto rt60{
                sabine_reverb_time(scene_data, make_volume_type(0)).s[0]};

        const size_t simulation_length =
                std::pow(std::floor(std::log(sampling_frequency * rt60 * 2) /
                                    std::log(2)) +
                                 1,
                         2);

        const auto spacing{waveguide::config::grid_spacing(
                speed_of_sound, 1 / sampling_frequency)};

        const auto voxelised{make_voxelised_scene_data(
                scene_data,
                5,
                waveguide::compute_adjusted_boundary(
                        geo::get_aabb(scene_data), receiver, spacing))};
        auto model{waveguide::compute_mesh(
                cc, voxelised, spacing, speed_of_sound)};
        model.set_coefficients({waveguide::to_flat_coefficients(surface)});

        const auto receiver_index{
                compute_index(model.get_descriptor(), receiver)};
        const auto source_index{compute_index(model.get_descriptor(), source)};

        if (!waveguide::is_inside(model, receiver_index)) {
            throw std::runtime_error("receiver is outside of mesh!");
        }
        if (!waveguide::is_inside(model, source_index)) {
            throw std::runtime_error("source is outside of mesh!");
        }

        struct signal final {
            std::string name;
            aligned::vector<float> kernel;
        };

        const auto valid_portion{0.15};

        aligned::vector<signal> signals{
                {"dirac", aligned::vector<float>{1.0}},
                {"gauss",
                 kernels::gaussian_kernel(sampling_frequency, valid_portion)},
                {"sinmod_gauss",
                 kernels::sin_modulated_gaussian_kernel(sampling_frequency,
                                                        valid_portion)},
                {"ricker",
                 kernels::ricker_kernel(sampling_frequency, valid_portion)},
                {"hi_sinc", hipass_sinc_kernel(sampling_frequency, 100, 401)},
                {"band_sinc",
                 bandpass_sinc_kernel(sampling_frequency,
                                      100,
                                      sampling_frequency * valid_portion,
                                      401)},
                {"mls", make_mls(sampling_frequency * rt60)},
        };

        for (const auto& i : signals) {
            auto kernel{waveguide::make_transparent(i.kernel)};
            kernel.resize(simulation_length);

            auto prep{waveguide::preprocessor::make_soft_source(
                    receiver_index, kernel.begin(), kernel.end())};

            callback_accumulator<waveguide::postprocessor::node> postprocessor{
                    receiver_index};

            progress_bar pb{std::cout, kernel.size()};
            waveguide::run(cc,
                           model,
                           prep,
                           [&](auto& a, const auto& b, auto c) {
                               postprocessor(a, b, c);
                               pb += 1;
                           },
                           true);

            {
                write(build_string(
                              "solution_growth.", i.name, ".transparent.wav"),
                      make_audio_file(postprocessor.get_output(),
                                      sampling_frequency),
                      16);
            }

            {
                auto copy{postprocessor.get_output()};
                filter::extra_linear_dc_blocker u;
                filter::run_two_pass(u, copy.begin(), copy.end());
                write(build_string(
                              "solution_growth.", i.name, ".dc_blocked.wav"),
                      make_audio_file(copy, sampling_frequency),
                      16);
            }

            {
                auto copy{postprocessor.get_output()};
                filter::block_dc(copy.begin(), copy.end(), sampling_frequency);
                write(build_string("solution_growth.",
                                   i.name,
                                   ".butterworth_blocked.wav"),
                      make_audio_file(copy, sampling_frequency),
                      16);
            }
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "critical runtime error: " << e.what() << '\n';
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "unknown error\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
