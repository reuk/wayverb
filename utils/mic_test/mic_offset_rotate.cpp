#include "box/img_src.h"
#include "box/waveguide.h"

#include "common/attenuator/hrtf.h"
#include "common/attenuator/microphone.h"

#include "frequency_domain/multiband_filter.h"

#include "utilities/aligned/map.h"
#include "utilities/map.h"
#include "utilities/range.h"

#include "cereal/archives/JSON.hpp"
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
constexpr auto generate_range(range<T> r) {
    std::array<T, num> ret;
    for (auto i = 0ul; i != ret.size(); ++i) {
        ret[i] = map(i, range<T>{0, num}, r);
    }
    return ret;
}

template <size_t bands, typename Callback>
auto run_single_angle(float angle,
                      const glm::vec3& receiver,
                      double sample_rate,
                      const Callback& callback) {
    constexpr auto distance = 1.0;
    const glm::vec3 source{receiver + glm::vec3{distance * std::sin(angle),
                                                0,
                                                distance * std::cos(angle)}};
    constexpr range<double> valid_frequency_range{0.01, 0.16};
    const auto params = frequency_domain::band_edges_and_widths<8>(
            valid_frequency_range, 1);
    const auto signal = callback(source, receiver);
    return make_angle_info(angle,
                           frequency_domain::per_band_energy(
                                   signal.begin(), signal.end(), params));
}

template <size_t bands, typename Callback>
auto run_multiple_angles(const glm::vec3& receiver,
                         float sample_rate,
                         const Callback& callback) {
    constexpr auto test_locations = 16;
    const auto angles =
            generate_range<test_locations>(range<double>{0, 2 * M_PI});
    return map_to_vector(begin(angles), end(angles), [&](auto angle) {
        return run_single_angle<bands>(angle, receiver, sample_rate, callback);
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

    const aligned::map<std::string, float> polar_pattern_map{
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
    const geo::box box{glm::vec3{-s}, glm::vec3{s}};

    const auto filter_frequency = 8000.0;
    const auto oversample_ratio = 1.0;
    const auto sample_rate = filter_frequency * oversample_ratio * (1 / 0.16);

    const glm::vec3 receiver{0, 0, 0};
    const attenuator::microphone mic{glm::vec3{0, 0, 1}, directionality};

    constexpr auto bands = 8;

    constexpr auto absorption = 0.001f;

    //  simulations ----------------------------------------------------------//

    const auto run = [&](const auto& name, const auto& callback) {
        const auto output =
                run_multiple_angles<bands>(receiver, sample_rate, callback);
        std::ofstream file{output_folder + "/" + name + ".txt"};
        cereal::JSONOutputArchive archive{file};
        archive(cereal::make_nvp("directionality", directionality),
                cereal::make_nvp("energies", output));
    };

    run("waveguide", [&](const auto& source, const auto& receiver) {
        const model::parameters params{source, receiver};
        auto raw = run_waveguide(box,
                                 make_surface(absorption, 0),
                                 params,
                                 sample_rate,
                                 2 / params.speed_of_sound);
        return postprocess_waveguide(begin(raw),
                                     end(raw),
                                     mic,
                                     sample_rate,
                                     params.acoustic_impedance);
    });

    run("img_src", [&](const auto& source, const auto& receiver) {
        const model::parameters params{source, receiver};
        const auto simulation_time = 2 / params.speed_of_sound;
        auto raw = run_exact_img_src(
                box, absorption, params, simulation_time, false);
        return raytracer::postprocess(begin(raw),
                                      end(raw),
                                      mic,
                                      params.receiver,
                                      params.speed_of_sound,
                                      sample_rate,
                                      simulation_time);
    });

    return EXIT_SUCCESS;
}
