#include "waveguide/attenuator/microphone.h"
#include "waveguide/boundary_adjust.h"
#include "waveguide/config.h"
#include "waveguide/make_transparent.h"
#include "waveguide/mesh.h"
#include "waveguide/postprocessor/single_node.h"
#include "waveguide/preprocessor/single_soft_source.h"
#include "waveguide/surface_filters.h"
#include "waveguide/waveguide.h"

#include "common/almost_equal.h"
#include "common/azimuth_elevation.h"
#include "common/cl/common.h"
#include "common/conversions.h"
#include "common/map_to_vector.h"
#include "common/progress_bar.h"
#include "common/scene_data.h"
#include "common/spatial_division/voxelised_scene_data.h"

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

struct FullTestResults final {
    aligned::vector<float> windowed_free_field_signal;
    aligned::vector<float> windowed_reflection_signal;
};

auto to_tuple(const FullTestResults& x) {
    return std::tie(x.windowed_free_field_signal, x.windowed_reflection_signal);
}

bool operator==(const FullTestResults& a, const FullTestResults& b) {
    return to_tuple(a) == to_tuple(b);
}

bool operator!=(const FullTestResults& a, const FullTestResults& b) {
    return !(a == b);
}

class boundary_test final {
public:
    boundary_test(const std::string& output_folder,
                  float azimuth,
                  float elevation)
            : output_folder_{output_folder}
            , azimuth_{azimuth}
            , elevation_{elevation} {}

private:
    aligned::vector<aligned::vector<float>> run_simulation(
            const geo::box& boundary,
            const aligned::vector<glm::vec3>& receivers,
            const coefficients_canonical& coefficients) const {
        const auto scene_data{geo::get_scene_data(boundary)};
        const auto spacing{waveguide::config::grid_spacing(
                speed_of_sound, 1 / (filter_frequency_ * 4))};

        auto mesh{waveguide::compute_mesh(
                cc_,
                voxelised_scene_data(scene_data,
                                     5,
                                     waveguide::compute_adjusted_boundary(
                                             scene_data.get_aabb(),
                                             source_position_,
                                             spacing)),
                spacing,
                speed_of_sound)};
        mesh.set_coefficients({coefficients});

        const auto source_index{
                compute_index(mesh.get_descriptor(), source_position_)};

        auto input{waveguide::make_transparent({1.0f})};
        input.resize(steps_, 0);
        waveguide::preprocessor::single_soft_source preprocessor{source_index,
                                                                 input};

        class output_holder final {
        public:
            output_holder(size_t node)
                    : node_{node} {}

            waveguide::step_postprocessor get_postprocessor() const {
                return waveguide::postprocessor::node{
                        node_, [&](auto i) { output_.emplace_back(i); }};
            }

            const auto& get_output() const { return output_; }

        private:
            mutable aligned::vector<float> output_;
            size_t node_;
        };

        aligned::vector<output_holder> output_holders;
        output_holders.reserve(receivers.size());
        for (const auto& receiver : receivers) {
            const auto receiver_index{
                    compute_index(mesh.get_descriptor(), receiver)};
            if (!waveguide::is_inside(mesh, receiver_index)) {
                throw std::runtime_error{"receiver is outside of mesh!"};
            }
            output_holders.emplace_back(receiver_index);
        }

        progress_bar pb{std::cout, steps_};
        waveguide::run(
                cc_,
                mesh,
                input.size(),
                preprocessor,
                map_to_vector(output_holders,
                              [](auto& i) { return i.get_postprocessor(); }),
                [&](auto i) { pb += 1; },
                true);

        return map_to_vector(output_holders,
                             [](const auto& i) { return i.get_output(); });
    }

    struct free_field_results final {
        aligned::vector<float> image;
        aligned::vector<float> direct;
    };

    free_field_results get_free_field_results() const {
        auto results{run_simulation(no_wall_,
                                    {image_position_, receiver_position_},
                                    coefficients_canonical{})};

        const auto window{right_hanning(results.front().size())};
        for (auto& i : results) {
            elementwise_multiply(i, window);
        }

        return {std::move(results[0]), std::move(results[1])};
    }

public:
    aligned::vector<float> run_full_test(
            const std::string& test_name,
            const coefficients_canonical& coefficients) const {
        auto reflected{
                run_simulation(wall_, {receiver_position_}, coefficients)
                        .front()};

        const auto window{right_hanning(reflected.size())};
        elementwise_multiply(reflected, window);

        auto subbed{reflected};
        proc::transform(reflected,
                        free_field_.direct.begin(),
                        subbed.begin(),
                        [](const auto& i, const auto& j) { return j - i; });

        const auto param_string{build_string(
                std::setprecision(4), "_az_", azimuth_, "_el_", elevation_)};

        snd::write(build_string(output_folder_,
                                "/",
                                test_name,
                                param_string,
                                "_windowed_free_field.wav"),
                   {free_field_.image},
                   out_sr_,
                   bit_depth_);
        snd::write(build_string(output_folder_,
                                "/",
                                test_name,
                                param_string,
                                "_windowed_subbed.wav"),
                   {subbed},
                   out_sr_,
                   bit_depth_);

        return subbed;
    }

    auto get_waveguide_sample_rate() const {return waveguide_sample_rate_;}

private:
    static constexpr auto speed_of_sound{340.0};

    const compute_context cc_{};
    const float filter_frequency_{2000.0};
    const float waveguide_sample_rate_{filter_frequency_ / 0.196f};
    const float out_sr_{44100.0};
    const size_t bit_depth_{16};
    const size_t dim_{300};
    const size_t steps_ = dim_ * 1.4;

    const size_t desired_nodes_{dim_ * dim_ * dim_};
    const float divisions_ = waveguide::config::grid_spacing(
            speed_of_sound, 1 / waveguide_sample_rate_);

    const geo::box wall_{
            glm::vec3{0, 0, 0},
            glm::vec3{static_cast<float>(desired_nodes_)} * divisions_};
    const glm::vec3 far_{wall_.get_max()};
    const glm::vec3 new_dim_{far_.x * 2, far_.y, far_.z};
    const geo::box no_wall_{glm::vec3{0}, new_dim_};

    const float source_dist_nodes_{glm::length(glm::vec3(desired_nodes_)) / 8};
    const float source_dist_{source_dist_nodes_ * divisions_};

    const glm::vec3 wall_centre_{centre(no_wall_)};

    const std::string output_folder_;
    const float azimuth_;
    const float elevation_;

    const glm::vec3 source_offset_{
            point_on_sphere(azimuth_ + M_PI, elevation_) * source_dist_};

    const glm::vec3 source_position_{wall_centre_ + source_offset_};
    const glm::vec3 image_position_{wall_centre_ - source_offset_};
    const glm::vec3 receiver_position_{wall_centre_ +
                                       source_offset_ * glm::vec3{1, -1, -1}};

    const free_field_results free_field_{get_free_field_results()};
};

int main(int argc, char** argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    if (argc != 4) {
        std::cerr
                << "expecting an output folder, an azimuth, and an elevation\n";
        std::cerr << "actually found: \n";
        for (auto i{0u}; i != argc; ++i) {
            std::cerr << "arg " << i << ": " << argv[i] << '\n';
        }

        return EXIT_FAILURE;
    }

    const auto output_folder{std::string(argv[1])};

    const auto azimuth{std::stof(argv[2])};
    const auto elevation{std::stof(argv[3])};

    const boundary_test test{output_folder, azimuth, elevation};

    try {
        struct surface_package final {
            std::string name;
            surface surface;
        };

        const cl_float lo{0.01};
        const cl_float hi{0.9};

        const auto surface_set = {
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

        const auto all_test_results{map_to_vector(surface_set, [&](auto i) {
            const auto coefficients{waveguide::to_filter_coefficients(
                    i.surface, test.get_waveguide_sample_rate())};
            return test.run_full_test(i.name, coefficients);
        })};

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
