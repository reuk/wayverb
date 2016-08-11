#include "waveguide/attenuator/microphone.h"
#include "waveguide/config.h"
#include "waveguide/mesh/boundary_adjust.h"
#include "waveguide/mesh/model.h"
#include "waveguide/waveguide.h"

#include "common/almost_equal.h"
#include "common/azimuth_elevation.h"
#include "common/cl_common.h"
#include "common/conversions.h"
#include "common/map_to_vector.h"
#include "common/progress_bar.h"
#include "common/scene_data.h"
#include "common/voxelised_scene_data.h"

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
                                      const geo::box& boundary,
                                      const surface& surface,
                                      double filter_frequency,
                                      double out_sr,
                                      const glm::vec3& source,
                                      const glm::vec3& receiver,
                                      const std::string& output_folder,
                                      const std::string& fname,
                                      int steps) {
    auto scene_data = geo::get_scene_data(boundary);
    scene_data.set_surfaces(surface);

    const auto spacing = waveguide::config::grid_spacing(
            speed_of_sound, 1 / (filter_frequency * 4));

    const auto model = waveguide::mesh::compute_model(
            cc.get_context(),
            cc.get_device(),
            voxelised_scene_data(
                    scene_data,
                    5,
                    waveguide::compute_adjusted_boundary(
                            scene_data.get_aabb(), receiver, spacing)),
            spacing);

    const auto receiver_index = compute_index(model.get_descriptor(), receiver);
    const auto source_index = compute_index(model.get_descriptor(), source);

    if (!waveguide::mesh::setup::is_inside(
                model.get_structure().get_condensed_nodes()[receiver_index])) {
        throw std::runtime_error("receiver is outside of mesh!");
    }
    if (!waveguide::mesh::setup::is_inside(
                model.get_structure().get_condensed_nodes()[source_index])) {
        throw std::runtime_error("source is outside of mesh!");
    }
    aligned::vector<float> input{1};
    input.resize(steps);

    progress_bar pb(std::cout, steps);
    const auto results = waveguide::run(cc.get_context(),
                                        cc.get_device(),
                                        model,
                                        source_index,
                                        input,
                                        receiver_index,
                                        [&](auto) { pb += 1; });

#if 0
    auto output = Microphone::omni.process(results);
#else
    auto output =
            map_to_vector(results, [](const auto& i) { return i.pressure; });
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
        std::cerr << "total desired nodes: " << total_desired_nodes << '\n';
        std::cerr << "however, total possible nodes: " << total_possible_nodes
                  << '\n';
        throw std::runtime_error("too many nodes");
    }

    const float divisions = waveguide::config::grid_spacing(
            speed_of_sound, 1.0 / (filter_frequency * 4));

    //  generate two boundaries, one twice the size of the other
    const geo::box wall(glm::vec3(0, 0, 0),
                        glm::vec3(desired_nodes) * divisions);
    const auto far = wall.get_max();
    const auto new_dim = glm::vec3(far.x * 2, far.y, far.z);
    const geo::box no_wall(glm::vec3(0, 0, 0), new_dim);

    //  place source and image in rooms based on distance in nodes from the wall
    auto source_dist_nodes = glm::length(glm::vec3(desired_nodes)) / 8;
    auto source_dist = source_dist_nodes * divisions;

    const auto wall_centre = centre(no_wall);

    auto log_incorrect_distance = [&source_dist, &wall_centre](
            auto str, const auto& pos) {
        auto dist = glm::distance(wall_centre, pos);
        if (!almost_equal(dist, source_dist, size_t{5})) {
            std::cerr << "incorrect distance: " << str << '\n';
            std::cerr << "distance: " << dist << '\n';
            std::cerr << "desired distance: " << source_dist << '\n';
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
        throw std::runtime_error("incorrect placement");
    }
    if (wrong_position(image_position, wall_centre)) {
        throw std::runtime_error("incorrect placement");
    }
    if (std::abs(glm::distance(source_position, image_position) -
                 source_dist * 2) > 1) {
        throw std::runtime_error("incorrect placement");
    }

    std::cerr << "running for " << steps << " steps\n";

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
        std::cerr << "total desired nodes: " << total_desired_nodes << '\n';
        std::cerr << "however, total possible nodes: " << total_possible_nodes
                  << '\n';
        throw std::runtime_error("too many nodes");
    }

    const float divisions = waveguide::config::grid_spacing(
            speed_of_sound, 1.0 / (filter_frequency * 4));

    //  generate two boundaries, one twice the size of the other
    const geo::box wall(glm::vec3(0, 0, 0),
                        glm::vec3(desired_nodes) * divisions);

    const auto far = wall.get_max();
    const glm::vec3 new_dim(far.x * 2, far.y, far.z);

    const geo::box no_wall(glm::vec3(0, 0, 0), new_dim);

    //  place source and receiver in rooms based on distance in nodes from the
    //  wall
    auto source_dist_nodes = glm::length(glm::vec3(desired_nodes)) / 8;
    auto source_dist = source_dist_nodes * divisions;

    const auto wall_centre = centre(no_wall);

    auto log_incorrect_distance = [&source_dist, &wall_centre](
            auto str, const auto& pos) {
        auto dist = glm::distance(wall_centre, pos);
        if (!almost_equal(dist, source_dist, size_t{5})) {
            std::cerr << "incorrect distance: " << str << '\n';
            std::cerr << "distance: " << dist << '\n';
            std::cerr << "desired distance: " << source_dist << '\n';
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
               !inside(no_wall, pos);
    };

    if (wrong_position(source_position, wall_centre)) {
        throw std::runtime_error("incorrect placement");
    }

    if (wrong_position(receiver_position, wall_centre)) {
        throw std::runtime_error("incorrect placement");
    }

    std::cerr << "running for " << steps << " steps\n";

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
    std::cerr << "subbed max mag: " << max_mag(subbed) << '\n';

    auto first_nonzero = [](const auto& i) {
        auto it = proc::find_if(i, [](auto j) { return j; });
        if (it == i.end())
            throw std::runtime_error("no non-zero values found");
        std::cerr << "first nonzero value: " << *it << '\n';
        return it - i.begin();
    };

    auto first_nonzero_reflected = first_nonzero(reflected);
    auto first_nonzero_direct = first_nonzero(direct);

    if (first_nonzero_reflected != first_nonzero_direct) {
        std::cerr << "WARNING: direct and reflected should receive signal at "
                     "same time\n";
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
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    if (argc != 4) {
        std::cerr
                << "expecting an output folder, an azimuth, and an elevation\n";
        std::cerr << "actually found: \n";
        for (auto i = 0u; i != argc; ++i) {
            std::cerr << "arg " << i << ": " << argv[i] << '\n';
        }

        return EXIT_FAILURE;
    }

    auto output_folder = std::string(argv[1]);

    auto azimuth = std::stod(argv[2]);
    auto elevation = std::stod(argv[3]);
    auto azimuth_elevation = std::make_pair(azimuth, elevation);

    compute_context cc;
    auto filter_frequency = 2000;
    auto out_sr = 44100;

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
            std::cerr << "somehow both test results are the same even though "
                         "they use different boundary coefficients";
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "critical runtime error: " << e.what();
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "unknown error";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
