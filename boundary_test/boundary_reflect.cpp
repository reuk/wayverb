#include "waveguide.h"
#include "waveguide_config.h"
#include "scene_data.h"
#include "test_flag.h"
#include "conversions.h"
#include "microphone.h"
#include "azimuth_elevation.h"

#include "cl_common.h"

//  dependency
#include "logger.h"
#include "filters_common.h"
#include "sinc.h"
#include "write_audio_file.h"

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

#include "sndfile.hh"
#include "samplerate.h"

#include <gflags/gflags.h>

//  stdlib
#include <random>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <map>

std::ostream& operator<<(std::ostream& os, const CuboidBoundary& cb) {
    Bracketer bracketer(os);
    return to_stream(os, cb.get_c0(), "  ", cb.get_c1(), "  ");
}

int main(int argc, char** argv) {
    Logger::restart();
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    //    auto output_folder = std::string(argv[1]);

    auto config = WaveguideConfig();
    config.get_filter_frequency() = 11025;
    config.get_oversample_ratio() = 1;

    auto context = get_context();
    auto device = get_device(context);
    auto queue = cl::CommandQueue(context, device);

    //  set room size based on desired number of nodes
    auto desired_nodes = Vec3<uint32_t>(1000, 1000, 1000);
    auto total_desired_nodes = desired_nodes.product();

    auto total_possible_nodes = 1 << 30;
    if (total_desired_nodes >= total_possible_nodes) {
        Logger::log_err("total desired nodes: ", total_desired_nodes);
        Logger::log_err("however, total possible nodes: ",
                        total_possible_nodes);
        return 0;
    }

    //  generate two boundaries, one twice the size of the other
    auto wall =
        CuboidBoundary(Vec3f(0, 0, 0), desired_nodes * config.get_divisions());
    Logger::log_err("boundary: ", wall);

    auto far = wall.get_c1();
    auto new_dim = Vec3f(far.x * 2, far.y, far.z);

    auto no_wall = CuboidBoundary(Vec3f(0, 0, 0), new_dim);

    //  place source, receiver (and image) in rooms based on distance in nodes
    //      from the wall
    auto source_dist_nodes = 400;
    auto source_dist = source_dist_nodes * config.get_divisions();

    auto azimuth = M_PI / 4;
    auto elevation = M_PI / 6;

    auto wall_centre = no_wall.get_centre();

    auto source_position =
        wall_centre + point_on_sphere(azimuth + M_PI, elevation) * source_dist;
    Logger::log_err("source position: ", source_position);

    auto receiver_position =
        wall_centre +
        point_on_sphere(-azimuth + M_PI, -elevation) * source_dist;
    Logger::log_err("receiver position: ", receiver_position);

    auto image_position =
        wall_centre + point_on_sphere(azimuth, -elevation) * source_dist;
    Logger::log_err("image position: ", image_position);

    auto steps = 2000;

    try {
        //  run two simulations
        auto waveguide_program =
            get_program<RectangularProgram>(context, device);
        RectangularWaveguide wall_waveguide(waveguide_program,
                                       queue,
                                       wall,
                                       config.get_divisions(),
                                       receiver_position);

        auto mic_index = wall_waveguide.get_index_for_coordinate(receiver_position);
        auto source_index =
            wall_waveguide.get_index_for_coordinate(source_position);

        auto corrected_source =
            wall_waveguide.get_coordinate_for_index(source_index);
        auto corrected_mic = wall_waveguide.get_coordinate_for_index(mic_index);

        Logger::log_err("source pos: ", corrected_source);
        Logger::log_err("mic pos: ", corrected_mic);

        auto wall_results =
            wall_waveguide.run_basic(corrected_source,
                                mic_index,
                                steps,
                                config.get_waveguide_sample_rate());

        auto omni = Microphone(Vec3f(0, 0, 0), 0);
        auto output = omni.process(wall_results);

        std::vector<float> out_signal(
            config.get_output_sample_rate() * output.size() /
            config.get_waveguide_sample_rate());

        LinkwitzRiley lopass;
        lopass.setParams(1,
                         config.get_filter_frequency(),
                         config.get_output_sample_rate());
        lopass.filter(out_signal);

        auto output_folder = ".";

        auto output_file = build_string(output_folder, "/reflect.wav");

        unsigned long format, depth;

        try {
            format = get_file_format(output_file);
            depth = get_file_depth(config.get_bit_depth());
        } catch (const std::runtime_error& e) {
            Logger::log_err("critical runtime error: ", e.what());
            return EXIT_FAILURE;
        }

        write_sndfile(output_file,
                      {out_signal},
                      config.get_output_sample_rate(),
                      depth,
                      format);

        //  do some nonsense with the outputs
    } catch (const cl::Error& e) {
        Logger::log_err("critical cl error: ", e.what());
        return EXIT_FAILURE;
    } catch (const std::runtime_error& e) {
        Logger::log_err("critical runtime error: ", e.what());
        return EXIT_FAILURE;
    } catch (...) {
        Logger::log_err("unknown error");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
