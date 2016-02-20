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

std::ostream& operator<<(std::ostream& os, const RunStepResult& rsr) {
    Bracketer bracketer(os);
    return to_stream(os, rsr.pressure);
}

void write_file(const WaveguideConfig& config,
                const std::string& fname,
                const std::vector<float>& output) {
    auto output_folder = ".";
    auto output_file = build_string(output_folder, "/", fname, ".wav");
    Logger::log_err("writing file: ", output_file);

    auto format = get_file_format(output_file);
    auto depth = get_file_depth(config.get_bit_depth());

    write_sndfile(
        output_file, {output}, config.get_output_sample_rate(), depth, format);
}

float hanning_point(float f) {
    return 0.5 - 0.5 * cos(2 * M_PI * f);
}

std::vector<float> right_hanning(int length) {
    std::vector<float> ret(length);
    for (auto i = 0; i != length; ++i) {
        ret[i] = hanning_point(0.5 + (i / (2 * (length - 1.0))));
    }
    return ret;
}

std::vector<float> run_simulation(const cl::Context& context,
                                  cl::Device& device,
                                  cl::CommandQueue& queue,
                                  const Boundary& boundary,
                                  const WaveguideConfig& config,
                                  const Vec3f& source,
                                  const Vec3f& receiver,
                                  const std::string& fname) {
    auto waveguide_program =
        get_program<RectangularProgram>(context, device);
    RectangularWaveguide waveguide(waveguide_program,
                                   queue,
                                   boundary,
                                   config.get_divisions(),
                                   receiver);

    auto receiver_index = waveguide.get_index_for_coordinate(receiver);
    auto source_index = waveguide.get_index_for_coordinate(source);

    if (! waveguide.inside(receiver_index)) {
        throw std::runtime_error("receiver is outside of mesh!");
    }
    if (! waveguide.inside(source_index)) {
        throw std::runtime_error("source is outside of mesh!");
    }

    auto corrected_source = waveguide.get_coordinate_for_index(source_index);
    auto corrected_mic = waveguide.get_coordinate_for_index(receiver_index);

    Logger::log_err("running simulation!");
    Logger::log_err("source pos: ", corrected_source);
    Logger::log_err("mic pos: ", corrected_mic);

    auto steps = 10000;

    auto results = waveguide.run_basic(corrected_source,
                                       receiver_index,
                                       steps,
                                       config.get_waveguide_sample_rate());

    auto output = Microphone::omni.process(results);

    LinkwitzRiley lopass;
    lopass.setParams(1,
                     config.get_filter_frequency(),
                     config.get_output_sample_rate());
    lopass.filter(output);

    write_file(config, fname, output);

    return output;
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
    auto desired_nodes = Vec3<uint32_t>(200, 200, 200);
    auto total_desired_nodes = desired_nodes.product();

    auto total_possible_nodes = 1 << 30;
    if (total_desired_nodes >= total_possible_nodes) {
        Logger::log_err("total desired nodes: ", total_desired_nodes);
        Logger::log_err("however, total possible nodes: ",
                        total_possible_nodes);
        return EXIT_FAILURE;
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
    auto source_dist_nodes = desired_nodes.mag() / 4;
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

    try {
        auto reflected = run_simulation(context,
                                        device,
                                        queue,
                                        wall,
                                        config,
                                        source_position,
                                        receiver_position,
                                        "reflected");
        auto free_field = run_simulation(context,
                                        device,
                                        queue,
                                        no_wall,
                                        config,
                                        source_position,
                                        image_position,
                                        "free_field");
        auto direct = run_simulation(context,
                                     device,
                                     queue,
                                     no_wall,
                                     config,
                                     source_position,
                                     receiver_position,
                                     "direct");

        auto subbed = reflected;
        std::transform(reflected.begin(),
                       reflected.end(),
                       direct.begin(),
                       subbed.begin(),
                       [](const auto& i, const auto& j) { return i - j; });

        write_file(config, "isolated_reflection", subbed);

        auto h = right_hanning(subbed.size());

        auto windowed_free_field = h;
        auto windowed_subbed = h;

        std::transform(free_field.begin(),
                       free_field.end(),
                       h.begin(),
                       windowed_free_field.begin(),
                       [](auto i, auto j) { return i * j; });
        std::transform(subbed.begin(),
                       subbed.end(),
                       h.begin(),
                       windowed_subbed.begin(),
                       [](auto i, auto j) { return i * j; });

        auto norm_factor = 1.0 / std::max(max_mag(windowed_free_field), max_mag(windowed_subbed));
        mul(windowed_free_field, norm_factor);
        mul(windowed_subbed, norm_factor);

        write_file(config, "windowed_free_field", windowed_free_field);
        write_file(config, "windowed_subbed", windowed_subbed);

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
