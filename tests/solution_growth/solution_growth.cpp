#include "waveguide/attenuator/microphone.h"
#include "waveguide/config.h"
#include "waveguide/make_transparent.h"
#include "waveguide/waveguide.h"

#include "common/azimuth_elevation.h"
#include "common/cl_common.h"
#include "common/conversions.h"
#include "common/dc_blocker.h"
#include "common/filters_common.h"
#include "common/kernel.h"
#include "common/map.h"
#include "common/progress_bar.h"
#include "common/scene_data.h"
#include "common/serialize/boundaries.h"
#include "common/serialize/surface.h"
#include "common/sinc.h"
#include "common/write_audio_file.h"

#include "samplerate.h"
#include "sndfile.hh"

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
}  // namespace

int main(int argc, char** argv) {
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    try {
        glm::vec3 source{0, 0, 0};
        glm::vec3 receiver{0.5, 0, 0};
        const auto sampling_frequency = 20000.0;

        compute_context cc;
        const box<3> boundary(glm::vec3(-1), glm::vec3(1));

        auto scene_data = get_scene_data(boundary);
        scene_data.set_surfaces(uniform_surface(0.999));

        waveguide::waveguide waveguide(cc.get_context(),
                                       cc.get_device(),
                                       mesh_boundary(scene_data),
                                       receiver,
                                       sampling_frequency);

        const auto receiver_index =
                waveguide.get_index_for_coordinate(receiver);
        const auto source_index = waveguide.get_index_for_coordinate(source);

        const auto corrected_source =
                waveguide.get_coordinate_for_index(source_index);

        struct signal {
            std::string name;
            aligned::vector<float> kernel;
        };

        aligned::vector<signal> signals{
                signal{"dirac", aligned::vector<float>{1.0}},
                signal{"gauss", kernels::gaussian_kernel(sampling_frequency)},
                signal{"sinmod_gauss",
                       kernels::sin_modulated_gaussian_kernel(
                               sampling_frequency)},
                signal{"ricker", kernels::ricker_kernel(sampling_frequency)},
                signal{"hi_sinc",
                       hipass_sinc_kernel(sampling_frequency, 100, 401)},
                signal{"band_sinc",
                       bandpass_sinc_kernel(sampling_frequency,
                                            100,
                                            sampling_frequency * 0.25,
                                            401)},
        };

        for (const auto& i : signals) {
            const auto steps = 10000;

            const auto kernel = waveguide::make_transparent(i.kernel);

            std::atomic_bool keep_going{true};
            progress_bar pb(std::cout, steps);
            const auto results =
                    waveguide::init_and_run(waveguide,
                                            corrected_source,
                                            kernel,
                                            receiver_index,
                                            steps,
                                            keep_going,
                                            [&](auto) { pb += 1; });

            const auto output = map_to_vector(
                    *results, [](const auto& i) { return i.pressure; });

            {
                const auto fname = build_string(
                        "solution_growth.", i.name, ".transparent.wav");

                snd::write(fname, {output}, sampling_frequency, 16);
            }

            {
                filter::extra_linear_dc_blocker u;
                const auto dc_removed =
                        filter::run_two_pass(u, output.begin(), output.end());

                const auto fname = build_string(
                        "solution_growth.", i.name, ".dc_blocked.wav");
                snd::write(fname, {dc_removed}, sampling_frequency, 16);
            }
        }
    } catch (const std::runtime_error& e) {
        LOG(INFO) << "critical runtime error: " << e.what();
        return EXIT_FAILURE;
    } catch (...) {
        LOG(INFO) << "unknown error";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
