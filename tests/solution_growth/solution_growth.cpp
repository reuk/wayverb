#include "waveguide/config.h"
#include "waveguide/microphone.h"
#include "waveguide/rectangular_waveguide.h"

#include "common/azimuth_elevation.h"
#include "common/cl_common.h"
#include "common/conversions.h"
#include "common/scene_data.h"

#include "common/serialize/boundaries.h"
#include "common/serialize/surface.h"

#include "common/filters_common.h"
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

std::vector<float> run_simulation(const ComputeContext& compute_context,
                                  const CuboidBoundary& boundary,
                                  const Surface& surface,
                                  double filter_frequency,
                                  double out_sr,
                                  const glm::vec3& source,
                                  const glm::vec3& receiver,
                                  const std::string& output_folder,
                                  const std::string& fname,
                                  int steps) {
    RectangularProgram waveguide_program(compute_context.context,
                                         compute_context.device);

    auto scene_data = boundary.get_scene_data();
    scene_data.set_surfaces(surface);

    RectangularWaveguide<BufferType::cl> waveguide(waveguide_program,
                                                   MeshBoundary(scene_data),
                                                   receiver,
                                                   filter_frequency * 4);

    auto receiver_index = waveguide.get_index_for_coordinate(receiver);
    auto source_index = waveguide.get_index_for_coordinate(source);

    if (!waveguide.inside(receiver_index)) {
        throw std::runtime_error("receiver is outside of mesh!");
    }
    if (!waveguide.inside(source_index)) {
        throw std::runtime_error("source is outside of mesh!");
    }

    auto corrected_source = waveguide.get_coordinate_for_index(source_index);
    auto corrected_mic = waveguide.get_coordinate_for_index(receiver_index);

    LOG(INFO) << "running simulation!";
    LOG(INFO) << "source pos: " << corrected_source;
    LOG(INFO) << "mic pos: " << corrected_mic;

    std::atomic_bool keep_going{true};
    ProgressBar pb(std::cout, steps);
    auto results = waveguide.init_and_run(corrected_source,
                                          std::vector<float>{1},
                                          receiver_index,
                                          steps,
                                          keep_going,
                                          [&pb] { pb += 1; });

    auto output = std::vector<float>(results.size());
    proc::transform(
            results, output.begin(), [](const auto& i) { return i.pressure; });

    return output;
}

int main(int argc, char** argv) {
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    try {
    } catch (const cl::Error& e) {
        LOG(INFO) << "critical cl error: " << e.what();
        return EXIT_FAILURE;
    } catch (const std::runtime_error& e) {
        LOG(INFO) << "critical runtime error: " << e.what();
        return EXIT_FAILURE;
    } catch (...) {
        LOG(INFO) << "unknown error";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
