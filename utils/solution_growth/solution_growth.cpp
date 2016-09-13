#include "waveguide/attenuator/microphone.h"
#include "waveguide/boundary_adjust.h"
#include "waveguide/config.h"
#include "waveguide/make_transparent.h"
#include "waveguide/mesh.h"
#include "waveguide/waveguide.h"

#include "common/azimuth_elevation.h"
#include "common/cl/common.h"
#include "common/conversions.h"
#include "common/dc_blocker.h"
#include "common/filters_common.h"
#include "common/kernel.h"
#include "common/map_to_vector.h"
#include "common/progress_bar.h"
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

namespace {
auto uniform_surface(float r) {
    return surface{volume_type{{r, r, r, r, r, r, r, r}},
                   volume_type{{r, r, r, r, r, r, r, r}}};
}

constexpr auto speed_of_sound{340.0};
}  // namespace

int main(int argc, char** argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    try {
        glm::vec3 source{0, 0, 0};
        glm::vec3 receiver{0.5, 0, 0};
        const auto sampling_frequency = 20000.0;

        compute_context cc;
        const geo::box boundary(glm::vec3(-1), glm::vec3(1));

        auto scene_data = geo::get_scene_data(boundary);
        scene_data.set_surfaces(uniform_surface(0.999));

        const auto spacing = waveguide::config::grid_spacing(
                speed_of_sound, 1 / sampling_frequency);

        const voxelised_scene_data voxelised(
                scene_data,
                5,
                waveguide::compute_adjusted_boundary(
                        scene_data.get_aabb(), receiver, spacing));

        const auto model{waveguide::compute_mesh(
                cc, voxelised, spacing, speed_of_sound)};

        const auto receiver_index =
                compute_index(model.get_descriptor(), receiver);
        const auto source_index = compute_index(model.get_descriptor(), source);

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

        const auto valid_portion{0.196};

        aligned::vector<signal> signals{
                signal{"dirac", aligned::vector<float>{1.0}},
                signal{"gauss",
                       kernels::gaussian_kernel(sampling_frequency,
                                                valid_portion)},
                signal{"sinmod_gauss",
                       kernels::sin_modulated_gaussian_kernel(
                               sampling_frequency, valid_portion)},
                signal{"ricker",
                       kernels::ricker_kernel(sampling_frequency,
                                              valid_portion)},
                signal{"hi_sinc",
                       hipass_sinc_kernel(sampling_frequency, 100, 401)},
                signal{"band_sinc",
                       bandpass_sinc_kernel(sampling_frequency,
                                            100,
                                            sampling_frequency * valid_portion,
                                            401)},
        };

        for (const auto& i : signals) {
            const auto steps = 10000;

            auto kernel = waveguide::make_transparent(i.kernel);
            kernel.resize(steps);

            progress_bar pb{std::cout, steps};
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
                const auto fname = build_string(
                        "solution_growth.", i.name, ".transparent.wav");

                snd::write(fname, {output}, sampling_frequency, 16);
            }

            {
                filter::extra_linear_dc_blocker u;
                filter::run_two_pass(u, output.begin(), output.end());

                const auto fname = build_string(
                        "solution_growth.", i.name, ".dc_blocked.wav");
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
