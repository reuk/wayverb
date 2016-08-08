#include "waveguide/attenuator/microphone.h"
#include "waveguide/config.h"
#include "waveguide/waveguide.h"

#include "common/azimuth_elevation.h"
#include "common/almost_equal.h"
#include "common/cl_common.h"
#include "common/conversions.h"
#include "common/map.h"
#include "common/progress_bar.h"
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

aligned::vector<float> run_simulation(const compute_context& cc,
                                      const box<3>& boundary,
                                      const surface& surface,
                                      double filter_frequency,
                                      double out_sr,
                                      const glm::vec3& source,
                                      const glm::vec3& receiver,
                                      const std::string& output_folder,
                                      const std::string& fname,
                                      int steps) {
    auto scene_data = get_scene_data(boundary);
    scene_data.set_surfaces(surface);

    waveguide::waveguide waveguide(cc.get_context(),
                                   cc.get_device(),
                                   mesh_boundary(scene_data),
                                   receiver,
                                   filter_frequency * 4);

    auto receiver_index = waveguide.get_index_for_coordinate(receiver);
    auto source_index   = waveguide.get_index_for_coordinate(source);

    if (!waveguide.inside(receiver_index)) {
        throw std::runtime_error("receiver is outside of mesh!");
    }
    if (!waveguide.inside(source_index)) {
        throw std::runtime_error("source is outside of mesh!");
    }

    auto corrected_source = waveguide.get_coordinate_for_index(source_index);
    auto corrected_mic    = waveguide.get_coordinate_for_index(receiver_index);

    LOG(INFO) << "running simulation!";
    LOG(INFO) << "source pos: " << corrected_source;
    LOG(INFO) << "mic pos: " << corrected_mic;

    std::atomic_bool keep_going{true};
    progress_bar pb(std::cout, steps);
    auto results = waveguide::init_and_run(waveguide,
                                           corrected_source,
                                           aligned::vector<float>{1},
                                           receiver_index,
                                           steps,
                                           keep_going,
                                           [&](auto) { pb += 1; });

#if 0
    auto output = Microphone::omni.process(results);
#else
    auto output =
            map_to_vector(*results, [](const auto& i) { return i.pressure; });
#endif

    // filter::LinkwitzRileySingleLopass lopass;
    // lopass.set_params(filter_frequency, out_sr);
    // lopass.filter(output);

    return output;
}

struct FullTestResults final {
    aligned::vector<float> windowed_free_field_signal;
    aligned::vector<float> windowed_reflection_signal;
};

bool operator==(const FullTestResults& a, const FullTestResults& b) {
    return std::tie(a.windowed_free_field_signal,
                    a.windowed_reflection_signal) ==
           std::tie(b.windowed_free_field_signal, b.windowed_reflection_signal);
}

bool operator!=(const FullTestResults& a, const FullTestResults& b) {
    return !(a == b);
}

aligned::vector<float> get_free_field_results(const compute_context& cc,
                                              const std::string& output_folder,
                                              double filter_frequency,
                                              double out_sr,
                                              float azimuth,
                                              float elevation,
                                              int dim,
                                              int steps) {
    //  set room size based on desired number of nodes
    auto desired_nodes = glm::ivec3(dim);
    auto total_desired_nodes =
            desired_nodes.x * desired_nodes.y * desired_nodes.z;

    auto total_possible_nodes = 1 << 30;
    if (total_desired_nodes >= total_possible_nodes) {
        LOG(INFO) << "total desired nodes: " << total_desired_nodes;
        LOG(INFO) << "however, total possible nodes: " << total_possible_nodes;
        throw std::runtime_error("too many nodes");
    }

    const float divisions = waveguide::config::grid_spacing(
            speed_of_sound, 1.0 / (filter_frequency * 4));

    //  generate two boundaries, one twice the size of the other
    const box<3> wall(glm::vec3(0, 0, 0), glm::vec3(desired_nodes) * divisions);
    const auto far     = wall.get_c1();
    const auto new_dim = glm::vec3(far.x * 2, far.y, far.z);
    const box<3> no_wall(glm::vec3(0, 0, 0), new_dim);

    //  place source and image in rooms based on distance in nodes from the wall
    auto source_dist_nodes = glm::length(glm::vec3(desired_nodes)) / 8;
    auto source_dist       = source_dist_nodes * divisions;

    const auto wall_centre = ::centre(no_wall);

    auto log_incorrect_distance = [&source_dist, &wall_centre](
            auto str, const auto& pos) {
        LOG(INFO) << str << " position: " << pos;
        auto dist = glm::distance(wall_centre, pos);
        if (!almost_equal(dist, source_dist, size_t{5})) {
            LOG(INFO) << "incorrect distance: " << str;
            LOG(INFO) << "distance: " << dist;
            LOG(INFO) << "desired distance: " << source_dist;
        }
    };

    auto source_offset =
            point_on_sphere(azimuth + M_PI, elevation) * source_dist;

    auto source_position = wall_centre + source_offset;
    log_incorrect_distance("source", source_position);

    auto image_position = wall_centre - source_offset;
    log_incorrect_distance("image", image_position);

    auto wrong_position = [source_dist, &no_wall](auto pos, auto c) {
        return std::abs(glm::distance(pos, c) - source_dist) > 1 ||
               !inside(no_wall, pos);
    };

    if (wrong_position(source_position, wall_centre)) {
        LOG(INFO) << "source is placed incorrectly";
        throw std::runtime_error("incorrect placement");
    }
    if (wrong_position(image_position, wall_centre)) {
        LOG(INFO) << "image is placed incorrectly";
        throw std::runtime_error("incorrect placement");
    }
    if (std::abs(glm::distance(source_position, image_position) -
                 source_dist * 2) > 1) {
        LOG(INFO) << "image is placed incorrectly";
        throw std::runtime_error("incorrect placement");
    }

    LOG(INFO) << "running for " << steps << " steps";

    auto image = run_simulation(cc,
                                no_wall,
                                surface{},
                                filter_frequency,
                                out_sr,
                                source_position,
                                image_position,
                                output_folder,
                                "image",
                                steps);

    auto windowed_free_field = right_hanning(image.size());
    elementwise_multiply(windowed_free_field, image);

    return windowed_free_field;
}

FullTestResults run_full_test(const std::string& test_name,
                              const compute_context& cc,
                              const std::string& output_folder,
                              double filter_frequency,
                              double out_sr,
                              float azimuth,
                              float elevation,
                              int dim,
                              int steps,
                              const surface& surface,
                              aligned::vector<float> windowed_free_field) {
    //  set room size based on desired number of nodes
    auto desired_nodes = glm::ivec3(dim);
    auto total_desired_nodes =
            desired_nodes.x * desired_nodes.y * desired_nodes.z;

    auto total_possible_nodes = 1 << 30;
    if (total_desired_nodes >= total_possible_nodes) {
        LOG(INFO) << "total desired nodes: " << total_desired_nodes;
        LOG(INFO) << "however, total possible nodes: " << total_possible_nodes;
        throw std::runtime_error("too many nodes");
    }

    const float divisions = waveguide::config::grid_spacing(
            speed_of_sound, 1.0 / (filter_frequency * 4));

    //  generate two boundaries, one twice the size of the other
    const box<3> wall(glm::vec3(0, 0, 0), glm::vec3(desired_nodes) * divisions);

    const auto far = wall.get_c1();
    const glm::vec3 new_dim(far.x * 2, far.y, far.z);

    const box<3> no_wall(glm::vec3(0, 0, 0), new_dim);

    //  place source and receiver in rooms based on distance in nodes from the
    //  wall
    auto source_dist_nodes = glm::length(glm::vec3(desired_nodes)) / 8;
    auto source_dist       = source_dist_nodes * divisions;

    const auto wall_centre = ::centre(no_wall);

    auto log_incorrect_distance = [&source_dist, &wall_centre](
            auto str, const auto& pos) {
        LOG(INFO) << str << " position: " << pos;
        auto dist = glm::distance(wall_centre, pos);
        if (!almost_equal(dist, source_dist, size_t{5})) {
            LOG(INFO) << "incorrect distance: " << str;
            LOG(INFO) << "distance: " << dist;
            LOG(INFO) << "desired distance: " << source_dist;
        }
    };

    auto source_offset =
            point_on_sphere(azimuth + M_PI, elevation) * source_dist;

    auto source_position = wall_centre + source_offset;
    log_incorrect_distance("source", source_position);

    auto receiver_position = wall_centre + source_offset * glm::vec3(1, -1, -1);
    log_incorrect_distance("receiver", receiver_position);

    auto wrong_position = [source_dist, &no_wall](auto pos, auto c) {
        return std::abs(glm::distance(pos, c) - source_dist) > 1 ||
               !::inside(no_wall, pos);
    };

    if (wrong_position(source_position, wall_centre)) {
        LOG(INFO) << "source is placed incorrectly";
        throw std::runtime_error("incorrect placement");
    }

    if (wrong_position(receiver_position, wall_centre)) {
        LOG(INFO) << "receiver is placed incorrectly";
        throw std::runtime_error("incorrect placement");
    }

    LOG(INFO) << "running for " << steps << " steps";

    auto reflected = run_simulation(cc,
                                    wall,
                                    surface,
                                    filter_frequency,
                                    out_sr,
                                    source_position,
                                    receiver_position,
                                    output_folder,
                                    "reflected",
                                    steps);
    auto direct = run_simulation(cc,
                                 no_wall,
                                 surface,
                                 filter_frequency,
                                 out_sr,
                                 source_position,
                                 receiver_position,
                                 output_folder,
                                 "direct",
                                 steps);

    auto subbed = reflected;
    proc::transform(reflected,
                    direct.begin(),
                    subbed.begin(),
                    [](const auto& i, const auto& j) { return j - i; });
    LOG(INFO) << "subbed max mag: " << max_mag(subbed);

    auto first_nonzero = [](const auto& i) {
        auto it = proc::find_if(i, [](auto j) { return j; });
        if (it == i.end())
            throw std::runtime_error("no non-zero values found");
        LOG(INFO) << "first nonzero value: " << *it;
        return it - i.begin();
    };

    auto first_nonzero_reflected = first_nonzero(reflected);
    auto first_nonzero_direct    = first_nonzero(direct);

    if (first_nonzero_reflected != first_nonzero_direct) {
        LOG(INFO) << "WARNING: direct and reflected should receive signal at "
                     "same time";
    }

    auto windowed_subbed = right_hanning(subbed.size());
    elementwise_multiply(windowed_subbed, subbed);

    auto norm_factor = 1.0 / std::max(max_mag(windowed_free_field),
                                      max_mag(windowed_subbed));
    mul(windowed_subbed, norm_factor);
    mul(windowed_free_field, norm_factor);

    auto param_string = build_string(
            std::setprecision(4), "_az_", azimuth, "_el_", elevation);

    const auto bit_depth = 16;

    snd::write(build_string(output_folder,
                            "/",
                            test_name,
                            param_string,
                            "_windowed_free_field.wav"),
               {windowed_free_field},
               out_sr,
               bit_depth);
    snd::write(build_string(output_folder,
                            "/",
                            test_name,
                            param_string,
                            "_windowed_subbed.wav"),
               {windowed_subbed},
               out_sr,
               bit_depth);

    return FullTestResults{windowed_free_field, windowed_subbed};
}

int main(int argc, char** argv) {
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    if (argc != 4) {
        LOG(INFO) << "expecting an output folder, an azimuth, and an elevation";

        LOG(INFO) << "actually found: ";
        for (auto i = 0u; i != argc; ++i) {
            LOG(INFO) << "arg " << i << ": " << argv[i];
        }

        return EXIT_FAILURE;
    }

    auto output_folder = std::string(argv[1]);

    auto azimuth           = std::stod(argv[2]);
    auto elevation         = std::stod(argv[3]);
    auto azimuth_elevation = std::make_pair(azimuth, elevation);

    compute_context cc;
    auto filter_frequency = 2000;
    auto out_sr           = 44100;

    //  set room size based on desired number of nodes
    auto dim = 300;

    try {
        struct surface_package {
            std::string name;
            surface surface;
        };

        auto steps = dim * 1.4;

        auto windowed_free_field =
                get_free_field_results(cc,
                                       output_folder,
                                       filter_frequency,
                                       out_sr,
                                       azimuth_elevation.first,
                                       azimuth_elevation.second,
                                       dim,
                                       steps);

        cl_float lo = 0.01;
        cl_float hi = 0.9;

        auto surface_set = {
                surface_package{"anechoic",
                                surface{{{lo, lo, lo, lo, lo, lo, lo, lo}},
                                        {{lo, lo, lo, lo, lo, lo, lo, lo}}}},
                surface_package{"filtered_1",
                                surface{{{hi, lo, lo, lo, lo, lo, lo, lo}},
                                        {{hi, lo, lo, lo, lo, lo, lo, lo}}}},
                surface_package{"filtered_2",
                                surface{{{lo, hi, lo, lo, lo, lo, lo, lo}},
                                        {{lo, hi, lo, lo, lo, lo, lo, lo}}}},
                surface_package{"filtered_3",
                                surface{{{lo, lo, hi, lo, lo, lo, lo, lo}},
                                        {{lo, lo, hi, lo, lo, lo, lo, lo}}}},
                surface_package{
                        "filtered_4",
                        surface{{{0.4, 0.3, 0.5, 0.8, hi, hi, hi, hi}},
                                {{0.4, 0.3, 0.5, 0.8, hi, hi, hi, hi}}}},
                surface_package{"filtered_5",
                                surface{{{lo, hi, hi, hi, hi, hi, hi, hi}},
                                        {{lo, hi, hi, hi, hi, hi, hi, hi}}}},
                surface_package{"filtered_6",
                                surface{{{hi, lo, hi, hi, hi, hi, hi, hi}},
                                        {{hi, lo, hi, hi, hi, hi, hi, hi}}}},
                surface_package{"filtered_7",
                                surface{{{hi, hi, lo, hi, hi, hi, hi, hi}},
                                        {{hi, hi, lo, hi, hi, hi, hi, hi}}}},
                surface_package{"flat",
                                surface{{{hi, hi, hi, hi, hi, hi, hi, hi}},
                                        {{hi, hi, hi, hi, hi, hi, hi, hi}}}},
        };

        const auto all_test_results = map_to_vector(surface_set, [&](auto i) {
            return run_full_test(i.name,
                                 cc,
                                 output_folder,
                                 filter_frequency,
                                 out_sr,
                                 azimuth_elevation.first,
                                 azimuth_elevation.second,
                                 dim,
                                 steps,
                                 i.surface,
                                 windowed_free_field);
        });

        if (all_test_results.front() == all_test_results.back()) {
            LOG(INFO) << "somehow both test results are the same even though "
                         "they use different boundary coefficients";
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
