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
                const std::string& output_folder,
                const std::string& fname,
                const std::vector<float>& output) {
    auto output_file = build_string(output_folder, "/", fname, ".wav");
    Logger::log_err("writing file: ", output_file);

    auto format = get_file_format(output_file);
    auto depth = get_file_depth(config.get_bit_depth());

    write_sndfile(
        output_file, {output}, config.get_output_sample_rate(), depth, format);
    //    auto normalized = output;
    //    normalize(normalized);
    //
    //    auto norm_output_file =
    //        build_string(output_folder, "/normalized.", fname, ".wav");
    //    Logger::log_err("writing file: ", norm_output_file);
    //    write_sndfile(norm_output_file,
    //                  {normalized},
    //                  config.get_output_sample_rate(),
    //                  depth,
    //                  format);
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
                                  const CuboidBoundary& boundary,
                                  const WaveguideConfig& config,
                                  const Vec3f& source,
                                  const Vec3f& receiver,
                                  const std::string& output_folder,
                                  const std::string& fname,
                                  int steps) {
    auto waveguide_program = get_program<RectangularProgram>(context, device);

    RectangularWaveguide waveguide(waveguide_program,
                                   queue,
                                   boundary,
                                   config.get_divisions(),
                                   receiver,
                                   config.get_waveguide_sample_rate());

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

    Logger::log_err("running simulation!");
    Logger::log_err("source pos: ", corrected_source);
    Logger::log_err("mic pos: ", corrected_mic);

    auto results = waveguide.run_basic(corrected_source,
                                       receiver_index,
                                       steps,
                                       config.get_waveguide_sample_rate());

#if 0
    auto output = Microphone::omni.process(results);
#else
    auto output = std::vector<float>(results.size());
    std::transform(results.begin(),
                   results.end(),
                   output.begin(),
                   [](const auto& i) { return i.pressure; });
#endif

    LinkwitzRiley lopass;
    lopass.setParams(
        1, config.get_filter_frequency(), config.get_output_sample_rate());
    //    lopass.filter(output);

    return output;
}

struct FullTestResults {
    std::vector<float> windowed_free_field_signal;
    std::vector<float> windowed_reflection_signal;

    bool operator==(const FullTestResults& rhs) const {
        return windowed_free_field_signal == rhs.windowed_free_field_signal &&
               windowed_reflection_signal == rhs.windowed_reflection_signal;
    }
};

std::vector<float> get_free_field_results(const cl::Context& context,
                                          cl::Device& device,
                                          cl::CommandQueue& queue,
                                          const std::string& output_folder,
                                          const WaveguideConfig& config,
                                          float azimuth,
                                          float elevation,
                                          int dim,
                                          int steps) {
    //  set room size based on desired number of nodes
    auto desired_nodes = Vec3<uint32_t>(dim, dim, dim);
    auto total_desired_nodes = desired_nodes.product();

    auto total_possible_nodes = 1 << 30;
    if (total_desired_nodes >= total_possible_nodes) {
        Logger::log_err("total desired nodes: ", total_desired_nodes);
        Logger::log_err("however, total possible nodes: ",
                        total_possible_nodes);
        throw std::runtime_error("too many nodes");
    }

    //  generate two boundaries, one twice the size of the other
    auto wall =
        CuboidBoundary(Vec3f(0, 0, 0), desired_nodes * config.get_divisions());
    auto far = wall.get_c1();
    auto new_dim = Vec3f(far.x * 2, far.y, far.z);
    auto no_wall = CuboidBoundary(Vec3f(0, 0, 0), new_dim);

    //  place source and image in rooms based on distance in nodes from the wall
    auto source_dist_nodes = desired_nodes.mag() / 8;
    auto source_dist = source_dist_nodes * config.get_divisions();

    auto wall_centre = no_wall.get_centre();

    auto log_incorrect_distance = [&source_dist, &wall_centre](
        auto str, const auto& pos) {
        Logger::log_err(str, " position: ", pos);
        auto dist = (wall_centre - pos).mag();
        if (!almost_equal(dist, source_dist, 5)) {
            Logger::log_err("incorrect distance: ", str);
            Logger::log_err("distance: ", dist);
            Logger::log_err("desired distance: ", source_dist);
        }
    };

    auto source_offset =
        point_on_sphere(azimuth + M_PI, elevation) * source_dist;

    auto source_position = wall_centre + source_offset;
    log_incorrect_distance("source", source_position);

    auto image_position = wall_centre - source_offset;
    log_incorrect_distance("image", image_position);

    auto wrong_position = [source_dist, &no_wall](auto pos, auto c) {
        return std::abs((pos - c).mag() - source_dist) > 1 ||
               !no_wall.inside(pos);
    };

    if (wrong_position(source_position, wall_centre)) {
        Logger::log_err("source is placed incorrectly");
        throw std::runtime_error("incorrect placement");
    }

    if (wrong_position(image_position, wall_centre)) {
        Logger::log_err("image is placed incorrectly");
        throw std::runtime_error("incorrect placement");
    }

    if (std::abs((source_position - image_position).mag() - source_dist * 2) >
        1) {
        Logger::log_err("image is placed incorrectly");
        throw std::runtime_error("incorrect placement");
    }

    Logger::log_err("running for ", steps, " steps");

    auto image = run_simulation(context,
                                device,
                                queue,
                                no_wall,
                                config,
                                source_position,
                                image_position,
                                output_folder,
                                "image",
                                steps);
    auto h = right_hanning(image.size());

    auto windowed_free_field = h;

    auto window = [&h](const auto& in, auto& out) {
        std::transform(in.begin(),
                       in.end(),
                       h.begin(),
                       out.begin(),
                       [](auto i, auto j) { return i * j; });
    };

    window(image, windowed_free_field);

    return windowed_free_field;
}

FullTestResults run_full_test(const std::string& test_name,
                              const cl::Context& context,
                              cl::Device& device,
                              cl::CommandQueue& queue,
                              const std::string& output_folder,
                              const WaveguideConfig& config,
                              float azimuth,
                              float elevation,
                              int dim,
                              int steps,
                              const Surface& surface,
                              std::vector<float> windowed_free_field) {
    //  set room size based on desired number of nodes
    auto desired_nodes = Vec3<uint32_t>(dim, dim, dim);
    auto total_desired_nodes = desired_nodes.product();

    auto total_possible_nodes = 1 << 30;
    if (total_desired_nodes >= total_possible_nodes) {
        Logger::log_err("total desired nodes: ", total_desired_nodes);
        Logger::log_err("however, total possible nodes: ",
                        total_possible_nodes);
        throw std::runtime_error("too many nodes");
    }

    //  generate two boundaries, one twice the size of the other
    auto wall = CuboidBoundary(
        Vec3f(0, 0, 0), desired_nodes * config.get_divisions(), {surface});

    Logger::log_err("boundary: ", wall);

    auto far = wall.get_c1();
    auto new_dim = Vec3f(far.x * 2, far.y, far.z);

    auto no_wall = CuboidBoundary(Vec3f(0, 0, 0), new_dim, {surface});

    //  place source and receiver in rooms based on distance in nodes from the
    //  wall
    auto source_dist_nodes = desired_nodes.mag() / 8;
    auto source_dist = source_dist_nodes * config.get_divisions();

    auto wall_centre = no_wall.get_centre();

    auto log_incorrect_distance = [&source_dist, &wall_centre](
        auto str, const auto& pos) {
        Logger::log_err(str, " position: ", pos);
        auto dist = (wall_centre - pos).mag();
        if (!almost_equal(dist, source_dist, 5)) {
            Logger::log_err("incorrect distance: ", str);
            Logger::log_err("distance: ", dist);
            Logger::log_err("desired distance: ", source_dist);
        }
    };

    auto source_offset =
        point_on_sphere(azimuth + M_PI, elevation) * source_dist;

    auto source_position = wall_centre + source_offset;
    log_incorrect_distance("source", source_position);

    auto receiver_position = wall_centre + source_offset * Vec3f(1, -1, -1);
    log_incorrect_distance("receiver", receiver_position);

    auto wrong_position = [source_dist, &no_wall](auto pos, auto c) {
        return std::abs((pos - c).mag() - source_dist) > 1 ||
               !no_wall.inside(pos);
    };

    if (wrong_position(source_position, wall_centre)) {
        Logger::log_err("source is placed incorrectly");
        throw std::runtime_error("incorrect placement");
    }

    if (wrong_position(receiver_position, wall_centre)) {
        Logger::log_err("receiver is placed incorrectly");
        throw std::runtime_error("incorrect placement");
    }

    Logger::log_err("running for ", steps, " steps");

    auto reflected = run_simulation(context,
                                    device,
                                    queue,
                                    wall,
                                    config,
                                    source_position,
                                    receiver_position,
                                    output_folder,
                                    "reflected",
                                    steps);
    auto direct = run_simulation(context,
                                 device,
                                 queue,
                                 no_wall,
                                 config,
                                 source_position,
                                 receiver_position,
                                 output_folder,
                                 "direct",
                                 steps);

    auto subbed = reflected;
    std::transform(reflected.begin(),
                   reflected.end(),
                   direct.begin(),
                   subbed.begin(),
                   [](const auto& i, const auto& j) { return j - i; });
    Logger::log_err("subbed max mag: ", max_mag(subbed));

    auto first_nonzero = [](const auto& i) {
        auto it = std::find_if(i.begin(), i.end(), [](auto j) { return j; });
        if (it == i.end())
            throw std::runtime_error("no non-zero values found");
        Logger::log_err("first nonzero value: ", *it);
        return it - i.begin();
    };

    auto first_nonzero_reflected = first_nonzero(reflected);
    auto first_nonzero_direct = first_nonzero(direct);

    if (first_nonzero_reflected != first_nonzero_direct) {
        Logger::log_err(
            "WARNING: direct and reflected should receive signal at same "
            "time");
    }

    auto h = right_hanning(subbed.size());

    auto windowed_subbed = h;

    auto window = [&h](const auto& in, auto& out) {
        std::transform(in.begin(),
                       in.end(),
                       h.begin(),
                       out.begin(),
                       [](auto i, auto j) { return i * j; });
    };

    window(subbed, windowed_subbed);

    auto norm_factor =
        1.0 / std::max(max_mag(windowed_free_field), max_mag(windowed_subbed));
    mul(windowed_subbed, norm_factor);
    mul(windowed_free_field, norm_factor);

    auto param_string =
        build_string(std::setprecision(4), "_az_", azimuth, "_el_", elevation);

    write_file(config,
               output_folder,
               test_name + param_string + "_windowed_free_field",
               windowed_free_field);
    write_file(config,
               output_folder,
               test_name + param_string + "_windowed_subbed",
               windowed_subbed);

    return FullTestResults{windowed_free_field, windowed_subbed};
}

int main(int argc, char** argv) {
    Logger::restart();
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    if (argc != 2) {
        Logger::log_err("expecting an output folder");

        Logger::log_err("actually found: ");
        for (auto i = 0u; i != argc; ++i) {
            Logger::log_err("arg ", i, ": ", argv[i]);
        }

        return EXIT_FAILURE;
    }

    auto output_folder = std::string(argv[1]);

    auto config = WaveguideConfig();
    config.get_filter_frequency() = 2000;
    config.get_oversample_ratio() = 1;

    Logger::log_err("waveguide sampling rate: ",
                    config.get_waveguide_sample_rate());

    auto context = get_context();
    auto device = get_device(context);

    auto available = device.getInfo<CL_DEVICE_AVAILABLE>();
    if (!available) {
        Logger::log_err("opencl device is not available!");
    }

    auto queue = cl::CommandQueue(context, device);

    //  set room size based on desired number of nodes
    auto dim = 250;

    // auto azimuth_elevation = std::make_pair(M_PI / 4, M_PI / 4);
    // auto azimuth_elevation = std::make_pair(M_PI / 3, M_PI / 3);
    // auto azimuth_elevation = std::make_pair(0, 0);
    auto azimuth_elevation = std::make_pair(M_PI / 4, 0);

    try {
        struct SurfacePackage {
            std::string name;
            Surface surface;
        };

        auto steps = dim * 1.4;

        auto windowed_free_field =
            get_free_field_results(context,
                                   device,
                                   queue,
                                   output_folder,
                                   config,
                                   azimuth_elevation.first,
                                   azimuth_elevation.second,
                                   dim,
                                   steps);

        cl_float lo = 0.01;
        cl_float hi = 0.9;

        auto surface_set = {
            SurfacePackage{"anechoic",
                           Surface{{{lo, lo, lo, lo, lo, lo, lo, lo}},
                                   {{lo, lo, lo, lo, lo, lo, lo, lo}}}},
            SurfacePackage{"filtered_1",
                           Surface{{{hi, lo, lo, lo, lo, lo, lo, lo}},
                                   {{hi, lo, lo, lo, lo, lo, lo, lo}}}},
            SurfacePackage{"filtered_2",
                           Surface{{{lo, hi, lo, lo, lo, lo, lo, lo}},
                                   {{lo, hi, lo, lo, lo, lo, lo, lo}}}},
            SurfacePackage{"filtered_3",
                           Surface{{{lo, lo, hi, lo, lo, lo, lo, lo}},
                                   {{lo, lo, hi, lo, lo, lo, lo, lo}}}},
            SurfacePackage{"filtered_4",
                           Surface{{{0.4, 0.3, 0.5, 0.8, hi, hi, hi, hi}},
                                   {{0.4, 0.3, 0.5, 0.8, hi, hi, hi, hi}}}},
            SurfacePackage{"filtered_5",
                           Surface{{{lo, hi, hi, hi, hi, hi, hi, hi}},
                                   {{lo, hi, hi, hi, hi, hi, hi, hi}}}},
            SurfacePackage{"filtered_6",
                           Surface{{{hi, lo, hi, hi, hi, hi, hi, hi}},
                                   {{hi, lo, hi, hi, hi, hi, hi, hi}}}},
            SurfacePackage{"filtered_7",
                           Surface{{{hi, hi, lo, hi, hi, hi, hi, hi}},
                                   {{hi, hi, lo, hi, hi, hi, hi, hi}}}},
            SurfacePackage{"flat",
                           Surface{{{hi, hi, hi, hi, hi, hi, hi, hi}},
                                   {{hi, hi, hi, hi, hi, hi, hi, hi}}}},
        };

        std::vector<FullTestResults> all_test_results(surface_set.size());
        std::transform(surface_set.begin(),
                       surface_set.end(),
                       all_test_results.begin(),
                       [&](auto i) {
                           return run_full_test(i.name,
                                                context,
                                                device,
                                                queue,
                                                output_folder,
                                                config,
                                                azimuth_elevation.first,
                                                azimuth_elevation.second,
                                                dim,
                                                steps,
                                                i.surface,
                                                windowed_free_field);
                       });

        if (all_test_results.front() == all_test_results.back()) {
            Logger::log_err(
                "somehow both test results are the same even though they use "
                "different boundary coefficients");
        }
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
