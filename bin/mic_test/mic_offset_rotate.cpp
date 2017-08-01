#include "box/img_src.h"

#include "raytracer/image_source/postprocess.h"

#include "waveguide/canonical.h"
#include "waveguide/postprocess.h"

#include "core/attenuator/hrtf.h"
#include "core/attenuator/microphone.h"

#include "frequency_domain/multiband_filter.h"

#include "utilities/aligned/map.h"
#include "utilities/map.h"
#include "utilities/progress_bar.h"
#include "utilities/range.h"

#include "cereal/archives/json.hpp"
#include "cereal/cereal.hpp"
#include "cereal/types/array.hpp"
#include "cereal/types/vector.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <random>

template <size_t bands>
struct angle_info final {
    double angle;
    std::array<double, bands> energy;
};

template <size_t bands>
constexpr auto make_angle_info(double angle,
                               const std::array<double, bands>& energy) {
    return angle_info<bands>{angle, energy};
}

template <typename T, size_t bands>
void serialize(T& archive, angle_info<bands>& info) {
    archive(cereal::make_nvp("angle", info.angle),
            cereal::make_nvp("energy", info.energy));
}

template <size_t num, typename T>
constexpr auto generate_range(util::range<T> r) {
    std::array<T, num> ret;
    for (auto i = 0; i != ret.size(); ++i) {
        ret[i] = map(i, util::make_range(size_t{0}, num), r);
    }
    return ret;
}

template <size_t bands, typename Callback>
auto run_single_angle(float angle,
                      const glm::vec3& receiver,
                      const Callback& callback) {
    constexpr auto distance = 1.0;
    const glm::vec3 source{receiver + glm::vec3{distance * std::sin(angle),
                                                0,
                                                distance * std::cos(angle)}};
    constexpr util::range<double> valid_frequency_range{0.002, 0.16};
    const auto params = frequency_domain::compute_multiband_params<8>(
            valid_frequency_range, 1);

    std::cout << "band edges:\n"; 
    for (const auto& i : params.edges) {
        std::cout << i << '\n';
    }

    const auto signal = callback(source, receiver);
    return make_angle_info(angle,
                           frequency_domain::per_band_energy(
                                   signal.begin(), signal.end(), params));
}

template <size_t bands, typename Callback>
auto run_multiple_angles(const glm::vec3& receiver, const Callback& callback) {
    constexpr auto test_locations = 16;
    const auto angles =
            generate_range<test_locations>(util::range<double>{0, 2 * M_PI});
    return util::map_to_vector(begin(angles), end(angles), [&](auto angle) {
        return run_single_angle<bands>(angle, receiver, callback);
    });
}

int main(int argc, char** argv) {
    //  arguments ------------------------------------------------------------//

    if (argc != 3) {
        throw std::runtime_error{
                "expected an output folder and a polar pattern"};
    }

    std::string output_folder{argv[1]};
    std::string polar_string{argv[2]};

    const util::aligned::map<std::string, float> polar_pattern_map{
            {"omnidirectional", 0.0f},
            {"cardioid", 0.5f},
            {"bidirectional", 1.0f}};

    const auto it = polar_pattern_map.find(polar_string);
    if (it == polar_pattern_map.end()) {
        throw std::runtime_error{"unrecognised polar pattern"};
    }
    const auto directionality = it->second;

    //  constants ------------------------------------------------------------//

    const auto s = 1.5f;
    const wayverb::core::geo::box box{glm::vec3{-s}, glm::vec3{s}};

    const auto filter_frequency = 8000.0;
    const auto oversample_ratio = 1.0;
    const auto sample_rate = filter_frequency * oversample_ratio * (1 / 0.16);

    const glm::vec3 receiver{0, 0, 0};
    const wayverb::core::attenuator::microphone mic{
            wayverb::core::orientation{{0, 0, 1}}, directionality};

    constexpr auto absorption = 0.001f;
    constexpr auto scattering = 0.0f;

    const wayverb::core::compute_context cc;

    constexpr wayverb::core::environment env{};

    const auto scene_data = wayverb::core::geo::get_scene_data(
            box,
            wayverb::core::make_surface<wayverb::core::simulation_bands>(
                    absorption, scattering));

    const auto voxelised = wayverb::waveguide::compute_voxels_and_mesh(
            cc, scene_data, receiver, sample_rate, env.speed_of_sound);

    //  simulations ----------------------------------------------------------//

    const auto run = [&](const auto& name, const auto& callback) {
        const auto output =
                run_multiple_angles<wayverb::core::simulation_bands>(receiver,
                                                                     callback);
        std::ofstream file{output_folder + "/" + name + ".txt"};
        cereal::JSONOutputArchive archive{file};
        archive(cereal::make_nvp("directionality", directionality),
                cereal::make_nvp("energies", output));
    };

    run("waveguide", [&](const auto& source, const auto& receiver) {
        util::progress_bar pb;
        auto raw = *wayverb::waveguide::canonical(
                cc,
                voxelised,
                source,
                receiver,
                env,
                wayverb::waveguide::single_band_parameters{sample_rate, 0.6},
                2 / env.speed_of_sound,
                true,
                [&](auto&, const auto&, auto step, auto steps) {
                    set_progress(pb, step, steps);
                });
        return wayverb::waveguide::postprocess(
                raw, mic, env.acoustic_impedance, sample_rate);
    });

    /*
    run("img_src", [&](const auto& source, const auto& receiver) {
        const auto simulation_time = 2 / env.speed_of_sound;
        const auto raw = run_exact_img_src(
                box, absorption, source, receiver, env, simulation_time);
        return wayverb::raytracer::image_source::postprocess(begin(raw),
                                                             end(raw),
                                                             mic,
                                                             receiver,
                                                             env.speed_of_sound,
                                                             sample_rate);
    });
    */

    return EXIT_SUCCESS;
}
