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

#include "hrtf/multiband.h"

#include "core/almost_equal.h"
#include "core/az_el.h"
#include "core/azimuth_elevation.h"
#include "core/callback_accumulator.h"
#include "core/cl/common.h"
#include "core/conversions.h"
#include "core/filters_common.h"
#include "core/scene_data.h"
#include "core/serialize/az_el.h"
#include "core/serialize/surface.h"
#include "core/sinc.h"
#include "core/spatial_division/voxelised_scene_data.h"
#include "core/surfaces.h"

#include "utilities/map.h"
#include "utilities/map_to_vector.h"
#include "utilities/named_value.h"
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
    auto run_simulation(const wayverb::core::geo::box& boundary,
                        const util::aligned::vector<glm::vec3>& receivers,
                        const wayverb::waveguide::coefficients_canonical&
                                coefficients) const {
        const auto scene_data = wayverb::core::geo::get_scene_data(
                boundary,
                wayverb::core::make_surface<wayverb::core::simulation_bands>(
                        0, 0));

        auto mesh = wayverb::waveguide::compute_mesh(
                cc_,
                make_voxelised_scene_data(
                        scene_data,
                        5,
                        wayverb::waveguide::compute_adjusted_boundary(
                                wayverb::core::geo::compute_aabb(
                                        scene_data.get_vertices()),
                                source_position_,
                                divisions_)),
                divisions_,
                speed_of_sound);

        mesh.set_coefficients({coefficients});

        const auto source_index =
                compute_index(mesh.get_descriptor(), source_position_);

#if 1
        const auto input = [&] {
            const util::aligned::vector<float> raw_input{1000.0f};
            auto ret = wayverb::waveguide::make_transparent(
                    raw_input.data(), raw_input.data() + raw_input.size());
            ret.resize(steps, 0);
            return ret;
        }();
        auto prep = wayverb::waveguide::preprocessor::make_soft_source(
                source_index, input.begin(), input.end());

#else
        const auto input = [&] {
            util::aligned::vector<float> raw_input(steps, 0);
            raw_input[0] = 1000.0f;
            return raw_input;
        }();
        auto prep = wayverb::waveguide::preprocessor::make_hard_source(
                source_index, input.begin(), input.end());
#endif

        auto output_holders = util::map_to_vector(
                begin(receivers), end(receivers), [&](auto i) {
                    const auto receiver_index{
                            compute_index(mesh.get_descriptor(), i)};
                    if (!wayverb::waveguide::is_inside(mesh, receiver_index)) {
                        throw std::runtime_error{
                                "receiver is outside of mesh!"};
                    }
                    return wayverb::core::callback_accumulator<
                            wayverb::waveguide::postprocessor::node>{
                            receiver_index};
                });

        util::progress_bar pb{};
        wayverb::waveguide::run(
                cc_,
                mesh,
                prep,
                [&](auto& queue, const auto& buffer, auto step) {
                    for (auto& i : output_holders) {
                        i(queue, buffer, step);
                    }
                    set_progress(pb, step, steps);
                },
                true);

        return util::map_to_vector(
                begin(output_holders), end(output_holders), [](const auto& i) {
                    return i.get_output();
                });
    }

    struct free_field_results final {
        std::vector<float> image;
        std::vector<float> direct;
    };

    free_field_results get_free_field_results() const {
        auto results = run_simulation(
                no_wall_,
                {image_position_, receiver_position_},
                wayverb::waveguide::coefficients_canonical{{1}, {1}});

        const auto window =
                wayverb::core::right_hanning(results.front().size());
        for (auto& i : results) {
            wayverb::core::elementwise_multiply(i, window);
        }

        return {std::vector<float>(results[0].begin(), results[0].end()),
                std::vector<float>(results[1].begin(), results[1].end())};
    }

public:
    util::aligned::vector<float> run_full_test(
            const std::string& test_name,
            const wayverb::waveguide::coefficients_canonical& coefficients)
            const {
        auto reflected =
                run_simulation(wall_, {receiver_position_}, coefficients)
                        .front();

        const auto window = wayverb::core::right_hanning(reflected.size());
        wayverb::core::elementwise_multiply(reflected, window);

        auto subbed = reflected;
        std::transform(begin(reflected),
                       end(reflected),
                       free_field_.direct.begin(),
                       subbed.begin(),
                       [](const auto& i, const auto& j) { return j - i; });

        write(util::build_string(output_folder_,
                                 "/",
                                 test_name,
                                 "_windowed_free_field.wav")
                      .c_str(),
              free_field_.image,
              out_sr,
              audio_file::format::wav,
              bit_depth);
        write(util::build_string(
                      output_folder_, "/", test_name, "_windowed_subbed.wav")
                      .c_str(),
              subbed,
              out_sr,
              audio_file::format::wav,
              bit_depth);

        return subbed;
    }

    static constexpr auto speed_of_sound = 340.0;
    static constexpr auto out_sr = 44100;
    static constexpr auto bit_depth = audio_file::bit_depth::pcm16;
    static constexpr auto dim = 300;
    static constexpr auto steps = dim * 1.4;

private:
    const wayverb::core::compute_context cc_;

    const std::string output_folder_;
    const float waveguide_sample_rate_;
    const float azimuth_;
    const float elevation_;

    const float divisions_ = wayverb::waveguide::config::grid_spacing(
            speed_of_sound, 1 / waveguide_sample_rate_);

    const wayverb::core::geo::box wall_{
            glm::vec3{0, 0, 0},
            glm::vec3{static_cast<float>(dim)} * divisions_};
    const glm::vec3 far_ = wall_.get_max();
    const glm::vec3 new_dim_{far_.x * 2, far_.y, far_.z};
    const wayverb::core::geo::box no_wall_{glm::vec3{0}, new_dim_};

    const glm::vec3 wall_centre_ = centre(no_wall_);

    const float source_dist_nodes_ = glm::length(glm::vec3{dim}) / 8;
    const float source_dist_ = source_dist_nodes_ * divisions_;

    const glm::vec3 source_offset_ =
            wayverb::core::point_on_sphere(azimuth_ + M_PI, elevation_) *
            source_dist_;

    const glm::vec3 source_position_ = wall_centre_ + source_offset_;
    const glm::vec3 image_position_ = wall_centre_ - source_offset_;
    const glm::vec3 receiver_position_ =
            wall_centre_ + source_offset_ * glm::vec3{1, -1, -1};

    const free_field_results free_field_ = get_free_field_results();
};

struct file_item final {
    wayverb::core::az_el az_el;
    std::string material;
    std::string test;
    wayverb::waveguide::coefficients_canonical reflectance;
    wayverb::waveguide::coefficients_canonical impedance;
};

namespace cereal {

template <typename T>
void serialize(T& archive,
               wayverb::waveguide::coefficients_canonical& coefficients) {
    archive(cereal::make_nvp("b", coefficients.b),
            cereal::make_nvp("a", coefficients.a));
}

template <typename T>
void serialize(T& archive, file_item& i) {
    archive(cereal::make_nvp("az_el", i.az_el),
            cereal::make_nvp("material", i.material),
            cereal::make_nvp("test", i.test),
            cereal::make_nvp("reflectance", i.reflectance),
            cereal::make_nvp("impedance", i.impedance));
}

}  // namespace cereal

int main(int argc, char** argv) {
    //  arguments  /////////////////////////////////////////////////////////////

    if (argc != 2) {
        std::cerr << "expecting an output folder\n";
        std::cerr << "actually found: \n";
        for (auto i = 0; i != argc; ++i) {
            std::cerr << "arg " << i << ": " << argv[i] << '\n';
        }

        return EXIT_FAILURE;
    }

    const std::string output_folder{argv[1]};

    const auto angles = std::array<wayverb::core::az_el, 3>{
            {{0, 0}, {M_PI / 6, M_PI / 6}, {M_PI / 3, M_PI / 3}}};

    constexpr auto waveguide_sample_rate = 8000.0;

    {
        const auto centres = hrtf_data::hrtf_band_centres_hz();
        for (auto i : centres) {
            std::cout << i << '\n';
        }
    }

    const auto make_reflectance_coefficients = [&](const auto& absorption) {
        return wayverb::waveguide::compute_reflectance_filter_coefficients(
                absorption, waveguide_sample_rate);
    };

    //  Coefficients taken from
    //  Frequency-Dependent Absorbing Boundary Implementations in 3D Finite
    //  Different Time Domain Room Acoustics Simulations
    const auto coefficients = std::array<
            util::named_value<wayverb::waveguide::coefficients_canonical>,
            3>{{
            {"plaster",
             make_reflectance_coefficients(
                     std::array<double, wayverb::core::simulation_bands>{
                             {0.08, 0.08, 0.2, 0.5, 0.4, 0.4, 0.36}})},
            {"wood",
             make_reflectance_coefficients(
                     std::array<double, wayverb::core::simulation_bands>{
                             {0.15, 0.15, 0.11, 0.1, 0.07, 0.06, 0.06}})},
            {"concrete",
             make_reflectance_coefficients(
                     std::array<double, wayverb::core::simulation_bands>{
                             {0.02, 0.02, 0.03, 0.03, 0.03, 0.04, 0.07}})},
    }};

    //  fitted boundaries  /////////////////////////////////////////////////////

    std::ofstream file{util::build_string(output_folder, "/coefficients.txt")};
    cereal::JSONOutputArchive archive{file};

    for (const auto azel : angles) {
        //  Precomputes anechoic signal, so it's faster to group calculations
        //  by azimuth/elevation, even though outputs should be grouped by
        //  material.
        const boundary_test test{output_folder,
                                 waveguide_sample_rate,
                                 azel.azimuth,
                                 azel.elevation};

        for (const auto coeffs : coefficients) {
            const auto impedance_coeffs =
                    to_impedance_coefficients(coeffs.value);

            const auto test_name = util::build_string("az_",
                                                      azel.azimuth,
                                                      "_el_",
                                                      azel.elevation,
                                                      '_',
                                                      coeffs.name);

            test.run_full_test(test_name, impedance_coeffs);

            archive(file_item{azel,
                              coeffs.name,
                              test_name,
                              coeffs.value,
                              impedance_coeffs});
        }
    }

    return EXIT_SUCCESS;
}
