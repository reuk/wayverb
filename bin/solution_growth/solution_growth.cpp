#include "waveguide/boundary_adjust.h"
#include "waveguide/config.h"
#include "waveguide/fitted_boundary.h"
#include "waveguide/make_transparent.h"
#include "waveguide/mesh.h"
#include "waveguide/postprocessor/node.h"
#include "waveguide/preprocessor/soft_source.h"
#include "waveguide/waveguide.h"

#include "core/azimuth_elevation.h"
#include "core/callback_accumulator.h"
#include "core/cl/common.h"
#include "core/conversions.h"
#include "core/dc_blocker.h"
#include "core/filters_common.h"
#include "core/kernel.h"
#include "core/maximum_length_sequence.h"
#include "core/reverb_time.h"
#include "core/scene_data.h"
#include "core/serialize/surface.h"
#include "core/sinc.h"
#include "core/spatial_division/voxelised_scene_data.h"

#include "utilities/map_to_vector.h"
#include "utilities/progress_bar.h"

#include "audio_file/audio_file.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <random>

constexpr auto speed_of_sound = 340.0;

util::aligned::vector<float> make_mls(size_t length) {
    const size_t order = std::floor(std::log(length) / std::log(2)) + 1;
    util::aligned::vector<float> ret;
    ret.reserve(std::pow(2, order));
    wayverb::core::generate_maximum_length_sequence(
            order, [&](auto value, auto step) { ret.emplace_back(value); });
    return ret;
}

int main(int argc, char** argv) {
    try {
        const glm::vec3 source{4.8, 2.18, 2.12};
        const glm::vec3 receiver{3.91, 1.89, 1.69};
        const auto sampling_frequency = 8000.0;

        constexpr auto absorption = 0.2;
        const auto surface =
                wayverb::core::make_surface<wayverb::core::simulation_bands>(
                        absorption, 0);

        const wayverb::core::compute_context cc{};
        const wayverb::core::geo::box boundary{glm::vec3{0},
                                               glm::vec3{5.56, 3.97, 2.81}};

        const auto scene_data =
                wayverb::core::geo::get_scene_data(boundary, surface);

        const auto rt60 = max_element(eyring_reverb_time(scene_data, 0));

        const size_t simulation_length =
                std::pow(std::floor(std::log(sampling_frequency * rt60 * 2) /
                                    std::log(2)) +
                                 1,
                         2);

        const auto spacing = wayverb::waveguide::config::grid_spacing(
                speed_of_sound, 1 / sampling_frequency);

        const auto voxelised = make_voxelised_scene_data(
                scene_data,
                5,
                wayverb::waveguide::compute_adjusted_boundary(
                        wayverb::core::geo::compute_aabb(
                                scene_data.get_vertices()),
                        receiver,
                        spacing));
        auto model = wayverb::waveguide::compute_mesh(
                cc, voxelised, spacing, speed_of_sound);
        model.set_coefficients(
                wayverb::waveguide::to_flat_coefficients(absorption));

        const auto receiver_index =
                compute_index(model.get_descriptor(), receiver);
        const auto source_index = compute_index(model.get_descriptor(), source);

        if (!wayverb::waveguide::is_inside(model, receiver_index)) {
            throw std::runtime_error("receiver is outside of mesh!");
        }
        if (!wayverb::waveguide::is_inside(model, source_index)) {
            throw std::runtime_error("source is outside of mesh!");
        }

        struct signal final {
            std::string name;
            util::aligned::vector<float> kernel;
        };

        const auto valid_portion = 0.15;

        util::aligned::vector<signal> signals{
                {"dirac", util::aligned::vector<float>{1.0}},
                {"gauss",
                 wayverb::core::kernels::gaussian_kernel(sampling_frequency,
                                                         valid_portion)},
                {"sinmod_gauss",
                 wayverb::core::kernels::sin_modulated_gaussian_kernel(
                         sampling_frequency, valid_portion)},
                {"ricker",
                 wayverb::core::kernels::ricker_kernel(sampling_frequency,
                                                       valid_portion)},
                {"hi_sinc",
                 wayverb::core::hipass_sinc_kernel(
                         sampling_frequency, 100, 401)},
                {"band_sinc",
                 wayverb::core::bandpass_sinc_kernel(
                         sampling_frequency,
                         100,
                         sampling_frequency * valid_portion,
                         401)},
                {"mls", make_mls(sampling_frequency * rt60)},
        };

        for (const auto& i : signals) {
            auto kernel = wayverb::waveguide::make_transparent(
                    i.kernel.data(), i.kernel.data() + i.kernel.size());
            kernel.resize(simulation_length);

            auto prep = wayverb::waveguide::preprocessor::make_soft_source(
                    receiver_index, kernel.begin(), kernel.end());

            wayverb::core::callback_accumulator<
                    wayverb::waveguide::postprocessor::node>
                    postprocessor{receiver_index};

            util::progress_bar pb;
            wayverb::waveguide::run(cc,
                                    model,
                                    prep,
                                    [&](auto& a, const auto& b, auto c) {
                                        postprocessor(a, b, c);
                                        set_progress(pb, c, kernel.size());
                                    },
                                    true);

            {
                write(util::build_string(
                              "solution_growth.", i.name, ".transparent.wav"),
                      audio_file::make_audio_file(postprocessor.get_output(),
                                                  sampling_frequency),
                      16);
            }

            {
                auto copy = postprocessor.get_output();
                wayverb::core::filter::extra_linear_dc_blocker u;
                wayverb::core::filter::run_two_pass(
                        u, copy.begin(), copy.end());
                write(util::build_string(
                              "solution_growth.", i.name, ".dc_blocked.wav"),
                      audio_file::make_audio_file(copy, sampling_frequency),
                      16);
            }

            {
                auto copy = postprocessor.get_output();
                wayverb::core::filter::block_dc(
                        copy.begin(), copy.end(), sampling_frequency);
                write(util::build_string("solution_growth.",
                                         i.name,
                                         ".butterworth_blocked.wav"),
                      audio_file::make_audio_file(copy, sampling_frequency),
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
