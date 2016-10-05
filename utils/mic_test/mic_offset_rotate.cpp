#include "run_methods.h"

#include "common/aligned/map.h"
#include "common/map.h"
#include "common/range.h"
#include "common/write_audio_file.h"

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
    float angle;
    std::array<float, bands> energy;
};

template <typename T, size_t bands>
void serialize(T& archive, angle_info<bands>& info) {
    archive(cereal::make_nvp("angle", info.angle),
            cereal::make_nvp("energy", info.energy));
}

template <size_t num, typename T>
constexpr auto generate_range(util::range<T> r) {
    std::array<T, num> ret;
    for (auto i{0ul}; i != ret.size(); ++i) {
        ret[i] = util::map(i, util::range<T>{0, num}, r);
    }
    return ret;
}

template <size_t bands, typename It>
auto per_band_energy(It begin, It end, util::range<double> range) {
    std::array<float, bands> ret{};

    const auto edges{band_edge_frequencies<bands>(range)};
    const auto widths{band_edge_widths<bands>(range, 1)};

    const size_t input_size = std::distance(begin, end);
    fast_filter filter{input_size * 2};
    for (auto i{0u}; i != bands; ++i) {
        aligned::vector<float> band(input_size, 0);
        filter.filter(begin, end, band.begin(), [&](auto cplx, auto freq) {
            return cplx * static_cast<float>(compute_bandpass_magnitude(
                                  freq,
                                  util::range<double>{edges[i], edges[i + 1]},
                                  widths[i],
                                  widths[i + 1],
                                  0));
        });

        const auto band_energy{std::sqrt(proc::accumulate(
                band, 0.0, [](auto a, auto b) { return a + b * b; }))};

        ret[i] = band_energy;
    }
    return ret;
}

template <size_t bands, typename Callback>
auto run_single_angle(float angle,
                      const glm::vec3& receiver,
                      util::range<double> audible_range,
                      const Callback& callback) {
    const glm::vec3 source{receiver +
                           glm::vec3{std::sin(angle), 0, std::cos(angle)}};
    const auto signal{callback(source, receiver)};
    return angle_info<bands>{
            angle,
            per_band_energy<bands>(
                    signal.begin(), signal.end(), audible_range)};
}

template <size_t bands, typename Callback>
auto run_multiple_angles(const glm::vec3& receiver,
                         float speed_of_sound,
                         float sample_rate,
                         util::range<double> audible_range,
                         const Callback& callback) {
    constexpr auto test_locations{16};
    const auto angles{
            generate_range<test_locations>(util::range<double>{0, 2 * M_PI})};
    return map_to_vector(angles, [&](auto angle) {
        return run_single_angle<bands>(
                angle, receiver, audible_range, callback);
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

    const auto it{polar_pattern_map.find(polar_string)};
    if (it == polar_pattern_map.end()) {
        throw std::runtime_error{"unrecognised polar pattern"};
    }
    const auto directionality{it->second};

    //  constants ------------------------------------------------------------//

    const auto s{1.5f};
    const geo::box box{glm::vec3{-s}, glm::vec3{s}};

    const auto filter_frequency{8000.0};
    const auto oversample_ratio{1.0};
    const auto sample_rate{filter_frequency * oversample_ratio * (1 / 0.16)};

    constexpr auto speed_of_sound{340.0};
    constexpr auto acoustic_impedance{400.0};

    const glm::vec3 receiver{0, 0, 0};
    const microphone mic{glm::vec3{0, 0, 1}, directionality};

    constexpr auto bands{8};

    const auto simulation_time{2 / speed_of_sound};

    constexpr auto absorption{0.001f};

    //  simulations ----------------------------------------------------------//

    const auto run{[&](const auto& name, const auto& callback) {
        const auto output{run_multiple_angles<bands>(
                receiver,
                speed_of_sound,
                sample_rate,
                util::range<double>{80 / sample_rate,
                                    filter_frequency / sample_rate},
                callback)};
        std::ofstream file{output_folder + "/" + name + ".txt"};
        cereal::JSONOutputArchive archive{file};
        archive(cereal::make_nvp("directionality", directionality),
                cereal::make_nvp("energies", output));
    }};

    run("waveguide", [&](const auto& source, const auto& receiver) {
        auto output{run_waveguide(box,
                                  absorption,
                                  source,
                                  receiver,
                                  mic,
                                  speed_of_sound,
                                  acoustic_impedance,
                                  sample_rate,
                                  simulation_time)};
        return waveguide::attenuate(
                mic, acoustic_impedance, output.begin(), output.end());
    });

    run("img_src", [&](const auto& source, const auto& receiver) {
        const auto ret{run_exact_img_src(box,
                                         absorption,
                                         source,
                                         receiver,
                                         mic,
                                         speed_of_sound,
                                         acoustic_impedance,
                                         sample_rate,
                                         simulation_time)};
        return ret;
    });

    return EXIT_SUCCESS;
}
