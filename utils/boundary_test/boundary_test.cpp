#include "waveguide/boundary_adjust.h"
#include "waveguide/config.h"
#include "waveguide/fitted_boundary.h"
#include "waveguide/make_transparent.h"
#include "waveguide/mesh.h"
#include "waveguide/postprocess.h"
#include "waveguide/postprocessor/node.h"
#include "waveguide/preprocessor/soft_source.h"
#include "waveguide/stable.h"
#include "waveguide/waveguide.h"

#include "common/almost_equal.h"
#include "common/azimuth_elevation.h"
#include "common/callback_accumulator.h"
#include "common/cl/common.h"
#include "common/conversions.h"
#include "common/filters_common.h"
#include "common/scene_data.h"
#include "common/serialize/surface.h"
#include "common/sinc.h"
#include "common/spatial_division/voxelised_scene_data.h"
#include "common/surfaces.h"

#include "utilities/map.h"
#include "utilities/map_to_vector.h"
#include "utilities/progress_bar.h"

#include "audio_file/audio_file.h"

#include "cereal/archives/json.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <random>

struct FullTestResults final {
    util::aligned::vector<float> windowed_free_field_signal;
    util::aligned::vector<float> windowed_reflection_signal;
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
                  float waveguide_sample_rate,
                  float azimuth,
                  float elevation)
            : output_folder_{output_folder}
            , waveguide_sample_rate_{waveguide_sample_rate}
            , azimuth_{azimuth}
            , elevation_{elevation} {}

private:
    auto run_simulation(const geo::box& boundary,
                        const util::aligned::vector<glm::vec3>& receivers,
                        const coefficients_canonical& coefficients) const {
        const auto scene_data = geo::get_scene_data(
                boundary, make_surface<simulation_bands>(0, 0));

        auto mesh = waveguide::compute_mesh(
                cc_,
                make_voxelised_scene_data(scene_data,
                                          5,
                                          waveguide::compute_adjusted_boundary(
                                                  geo::get_aabb(scene_data),
                                                  source_position_,
                                                  divisions_)),
                divisions_,
                speed_of_sound);

        mesh.set_coefficients({coefficients});

        const auto source_index =
                compute_index(mesh.get_descriptor(), source_position_);

        const auto input = [&] {
            const util::aligned::vector<float> raw_input{1.0f};
            auto ret = waveguide::make_transparent(
                    raw_input.data(), raw_input.data() + raw_input.size());
            ret.resize(steps, 0);
            return ret;
        }();

        auto prep = waveguide::preprocessor::make_soft_source(
                source_index, input.begin(), input.end());

        auto output_holders =
                util::map_to_vector(begin(receivers), end(receivers), [&](auto i) {
                    const auto receiver_index{
                            compute_index(mesh.get_descriptor(), i)};
                    if (!waveguide::is_inside(mesh, receiver_index)) {
                        throw std::runtime_error{
                                "receiver is outside of mesh!"};
                    }
                    return callback_accumulator<waveguide::postprocessor::node>{
                            receiver_index};
                });

        util::progress_bar pb{};
        waveguide::run(cc_,
                       mesh,
                       prep,
                       [&](auto& queue, const auto& buffer, auto step) {
                           for (auto& i : output_holders) {
                               i(queue, buffer, step);
                           }
                           set_progress(pb, step, steps);
                       },
                       true);

        return util::map_to_vector(begin(output_holders),
                             end(output_holders),
                             [](const auto& i) { return i.get_output(); });
    }

    struct free_field_results final {
        std::vector<float> image;
        std::vector<float> direct;
    };

    free_field_results get_free_field_results() const {
        auto results = run_simulation(no_wall_,
                                      {image_position_, receiver_position_},
                                      coefficients_canonical{{1}, {1}});

        const auto window = right_hanning(results.front().size());
        for (auto& i : results) {
            elementwise_multiply(i, window);
        }

        return {std::vector<float>(results[0].begin(), results[0].end()),
                std::vector<float>(results[1].begin(), results[1].end())};
    }

public:
    util::aligned::vector<float> run_full_test(
            const std::string& test_name,
            const coefficients_canonical& coefficients) const {
        auto reflected =
                run_simulation(wall_, {receiver_position_}, coefficients)
                        .front();

        const auto window = right_hanning(reflected.size());
        elementwise_multiply(reflected, window);

        auto subbed = reflected;
        std::transform(begin(reflected),
                       end(reflected),
                       free_field_.direct.begin(),
                       subbed.begin(),
                       [](const auto& i, const auto& j) { return j - i; });

        write(util::build_string(output_folder_,
                           "/",
                           test_name,
                           "_windowed_free_field.wav"),
              audio_file::make_audio_file(free_field_.image, out_sr),
              bit_depth);
        write(util::build_string(
                      output_folder_, "/", test_name, "_windowed_subbed.wav"),
              audio_file::make_audio_file(subbed, out_sr),
              bit_depth);

        return subbed;
    }

    static constexpr auto speed_of_sound = 340.0;
    static constexpr auto out_sr = 44100;
    static constexpr auto bit_depth = 16;
    static constexpr auto dim = 300;
    static constexpr auto steps = dim * 1.4;

private:
    const compute_context cc_;

    const std::string output_folder_;
    const float waveguide_sample_rate_;
    const float azimuth_;
    const float elevation_;

    const float divisions_ = waveguide::config::grid_spacing(
            speed_of_sound, 1 / waveguide_sample_rate_);

    const geo::box wall_{glm::vec3{0, 0, 0},
                         glm::vec3{static_cast<float>(dim)} * divisions_};
    const glm::vec3 far_ = wall_.get_max();
    const glm::vec3 new_dim_{far_.x * 2, far_.y, far_.z};
    const geo::box no_wall_{glm::vec3{0}, new_dim_};

    const glm::vec3 wall_centre_ = centre(no_wall_);

    const float source_dist_nodes_ = glm::length(glm::vec3{dim}) / 8;
    const float source_dist_ = source_dist_nodes_ * divisions_;

    const glm::vec3 source_offset_ =
            point_on_sphere(azimuth_ + M_PI, elevation_) * source_dist_;

    const glm::vec3 source_position_ = wall_centre_ + source_offset_;
    const glm::vec3 image_position_ = wall_centre_ - source_offset_;
    const glm::vec3 receiver_position_ =
            wall_centre_ + source_offset_ * glm::vec3{1, -1, -1};

    const free_field_results free_field_ = get_free_field_results();
};

template <typename T>
void serialize(T& archive, coefficients_canonical& coefficients) {
    archive(cereal::make_nvp("b", coefficients.b),
            cereal::make_nvp("a", coefficients.a));
}

struct coefficient_package final {
    std::string name;
    coefficients_canonical reflectance_coefficients;
    coefficients_canonical impedance_coefficients;
};

template <typename T>
void serialize(T& archive, coefficient_package& c) {
    archive(cereal::make_nvp("name", c.name),
            cereal::make_nvp("reflectance_coefficients",
                             c.reflectance_coefficients),
            cereal::make_nvp("impedance_coefficients",
                             c.impedance_coefficients));
}

int main(int argc, char** argv) {
    //  arguments  /////////////////////////////////////////////////////////////

    if (argc != 4) {
        std::cerr
                << "expecting an output folder, an azimuth, and an elevation\n";
        std::cerr << "actually found: \n";
        for (auto i = 0u; i != argc; ++i) {
            std::cerr << "arg " << i << ": " << argv[i] << '\n';
        }

        return EXIT_FAILURE;
    }

    const std::string output_folder{argv[1]};

    const auto azimuth = std::stof(argv[2]);
    const auto elevation = std::stof(argv[3]);

    //  fitted boundaries  /////////////////////////////////////////////////////

    constexpr auto waveguide_sample_rate = 8000.0;

    const auto make_reflectance_coefficients = [&](const auto& absorption) {
        return waveguide::compute_reflectance_filter_coefficients(
                absorption, waveguide_sample_rate);
    };

    try {
        const boundary_test test{
                output_folder, waveguide_sample_rate, azimuth, elevation};

        const util::aligned::vector<std::tuple<const char*, coefficients_canonical>>
                raw_tests{
                        //  {"flat_0",
                        //   waveguide::to_flat_coefficients(
                        //           make_surface<simulation_bands>(0.0199,
                        //           0))},
                        //  {"flat_1",
                        //   waveguide::to_flat_coefficients(
                        //           make_surface<simulation_bands>(0.19, 0))},
                        //  {"flat_2",
                        //   waveguide::to_flat_coefficients(
                        //           make_surface<simulation_bands>(0.36, 0))},

                        //  {"flat_fitted_0",
                        //   make_filter_coefficients(make_bands_type(0.0199).s)},
                        //  {"flat_fitted_1",
                        //   make_filter_coefficients(make_bands_type(0.19).s)},
                        //  {"flat_fitted_2",
                        //   make_filter_coefficients(make_bands_type(0.36).s)},

                        {"sloping_fitted_0",
                         make_reflectance_coefficients(
                                 std::array<double, simulation_bands>{{0.0,
                                                                       0.05,
                                                                       0.1,
                                                                       0.15,
                                                                       0.2,
                                                                       0.25,
                                                                       0.3,
                                                                       0.35}})},
                        {"sloping_fitted_1",
                         make_reflectance_coefficients(
                                 std::array<double, simulation_bands>{{0.35,
                                                                       0.3,
                                                                       0.25,
                                                                       0.2,
                                                                       0.15,
                                                                       0.1,
                                                                       0.05,
                                                                       0.0}})},

                        {"sudden",
                         make_reflectance_coefficients(
                                 std::array<double, simulation_bands>{
                                         {0, 1, 0, 1, 0, 1, 0, 1}})},
                };

        const auto coefficients_set = util::map_to_vector(
                begin(raw_tests), end(raw_tests), [](const auto& tup) {
                    return coefficient_package{
                            std::get<0>(tup),
                            std::get<1>(tup),
                            waveguide::to_impedance_coefficients(
                                    std::get<1>(tup))};
                });

        {
            //  Write coefficients to file.
            std::ofstream file{
                util::build_string(output_folder, "/coefficients.txt")};
            cereal::JSONOutputArchive{file}(
                    cereal::make_nvp("coefficients", coefficients_set));
        }

        const auto all_test_results = util::map_to_vector(
                begin(coefficients_set), end(coefficients_set), [&](auto i) {
                    return test.run_full_test(i.name, i.impedance_coefficients);
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
