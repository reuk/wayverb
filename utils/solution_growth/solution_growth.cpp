#include "waveguide/attenuator/microphone.h"
#include "waveguide/boundary_adjust.h"
#include "waveguide/config.h"
#include "waveguide/make_transparent.h"
#include "waveguide/mesh.h"
#include "waveguide/surface_filters.h"
#include "waveguide/waveguide.h"

#include "common/azimuth_elevation.h"
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
#include "common/write_audio_file.h"

#include <gflags/gflags.h>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <random>

//  TODO try a butterworth-filtered impulse
//  TODO work out why dc-blocked version isn't working
//  TODO test a super-steep butterworth as a dc blocker
//      zero-phase via two-pass filtering

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
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    try {
        const glm::vec3 source{4.8, 2.18, 2.12};
        const glm::vec3 receiver{3.91, 1.89, 1.69};
        const auto sampling_frequency{5000.0};
   
        const auto surface{make_surface(0.2, 0)};

        const compute_context cc{};
        const geo::box boundary{glm::vec3{0}, glm::vec3{5.56, 3.97, 2.81}};

        auto scene_data{geo::get_scene_data(boundary)};
        scene_data.set_surfaces(surface);

        const auto rt60{sabine_reverb_time(scene_data, make_volume_type(0)).s[0]};

        const size_t simulation_length =
                std::pow(std::floor(std::log(sampling_frequency * rt60 * 2) /
                                    std::log(2)) +
                                 1,
                         2);

        const auto spacing{waveguide::config::grid_spacing(
                speed_of_sound, 1 / sampling_frequency)};

        const voxelised_scene_data voxelised(
                scene_data,
                5,
                waveguide::compute_adjusted_boundary(
                        scene_data.get_aabb(), receiver, spacing));
        auto model{waveguide::compute_mesh(
                cc, voxelised, spacing, speed_of_sound)};
        model.set_coefficients({waveguide::to_flat_coefficients(surface)});

        const auto receiver_index {
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

            progress_bar pb{std::cout, kernel.size()};
            const auto results{waveguide::run(cc,
                                              model,
                                              source_index,
                                              kernel,
                                              receiver_index,
                                              speed_of_sound,
                                              400,
                                              [&](auto) { pb += 1; })};

            auto output{map_to_vector(
                    results, [](const auto& i) { return i.pressure; })};

            {
                const auto fname{build_string(
                        "solution_growth.", i.name, ".transparent.wav")};

                snd::write(fname, {output}, sampling_frequency, 16);
            }

            {
                filter::extra_linear_dc_blocker u;
                filter::run_two_pass(u, output.begin(), output.end());

                const auto fname{build_string(
                        "solution_growth.", i.name, ".dc_blocked.wav")};
                snd::write(fname, {output}, sampling_frequency, 16);
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
