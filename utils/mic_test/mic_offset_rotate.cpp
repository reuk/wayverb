#include "run_methods.h"

#include "common/write_audio_file.h"
#include "common/aligned/map.h"

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

int main(int argc, char** argv) {
    //  arguments ------------------------------------------------------------//
    if (argc != 3) {
        std::cerr << "expected an output folder and a polar pattern\n";

        return EXIT_FAILURE;
    }

    std::string output_folder{argv[1]};
    std::string polar_string{argv[2]};

    const aligned::map<std::string, float> polar_pattern_map{
            {"omni", 0.0f}, {"cardioid", 0.5f}, {"bidirectional", 1.0f}};

    const auto it{polar_pattern_map.find(polar_string)};
    if (it == polar_pattern_map.end()) {
        throw std::runtime_error{"unrecognised polar pattern"};
    }
    const auto directionality{it->second};
    std::cout << "directionality: " << directionality << std::endl;

    //  constants ------------------------------------------------------------//

    const auto s{1.5f};
    const geo::box box{glm::vec3{-s}, glm::vec3{s}};

    const auto filter_frequency{8000.0};
    const auto oversample_ratio{1.0};
    const auto sample_rate{filter_frequency * oversample_ratio * (1 / 0.16)};

    const glm::vec3 receiver{0, 0, 0};
    const auto test_locations{16};

    const compute_context cc{};

    const auto scene_data{geo::get_scene_data(box, make_surface(0, 0))};

    constexpr auto speed_of_sound{340.0};
    constexpr auto acoustic_impedance{400.0};

    auto voxels_and_model{waveguide::compute_voxels_and_mesh(
            cc, scene_data, receiver, sample_rate, speed_of_sound)};

    auto& model{std::get<1>(voxels_and_model)};
    model.set_coefficients(
            {waveguide::to_flat_coefficients(make_surface(0, 0))});

    constexpr auto bands{8};
    aligned::vector<angle_info<bands>> output;

    for (auto i{0u}; i != test_locations; ++i) {
        const auto angle{i * M_PI * 2 / test_locations + M_PI};

        const glm::vec3 source{std::sin(angle), 0, std::cos(angle)};
        const auto dist{glm::distance(source, receiver)};
        const auto time_between_source_receiver{dist / speed_of_sound};
        const size_t required_steps =
                time_between_source_receiver * sample_rate;

        const auto receiver_index{
                compute_index(model.get_descriptor(), receiver)};

        const auto steps{2 * required_steps};

        aligned::vector<float> input{1.0f};
        input.resize(steps, 0.0f);
        const auto generator{waveguide::preprocessor::make_hard_source(
                compute_index(model.get_descriptor(), source),
                input.begin(),
                input.end())};

        callback_accumulator<waveguide::postprocessor::directional_receiver>
                postprocessor{model.get_descriptor(),
                              sample_rate,
                              acoustic_impedance / speed_of_sound,
                              receiver_index};

        std::cout << "running " << steps << " steps" << std::endl;

        progress_bar pb(std::cout, steps);
        waveguide::run(cc,
                       model,
                       generator,
                       [&](auto& queue, const auto& buffer, auto step) {
                           postprocessor(queue, buffer, step);
                           pb += 1;
                       },
                       true);

        auto out_signal{waveguide::attenuate(
                microphone{glm::vec3{0, 0, 1}, directionality},
                acoustic_impedance,
                postprocessor.get_output().begin(),
                postprocessor.get_output().end())};

        const auto lower{100 / sample_rate},
                upper{filter_frequency / sample_rate};
        const auto edges{band_edge_frequencies<bands>(lower, upper)};
        const auto widths{band_edge_widths<bands>(lower, upper, 1)};

        angle_info<bands> info;
        info.angle = angle;

        fast_filter filter{out_signal.size() * 2};

        for (auto i{0u}; i != bands; ++i) {
            auto band{out_signal};

            filter.filter(band.begin(),
                          band.end(),
                          band.begin(),
                          [&](auto cplx, auto freq) {
                              return cplx * static_cast<float>(
                                                    compute_bandpass_magnitude(
                                                            freq,
                                                            edges[i],
                                                            widths[i],
                                                            edges[i + 1],
                                                            widths[i + 1],
                                                            0));
                          });

            const auto band_energy{std::sqrt(proc::accumulate(
                    band, 0.0, [](auto a, auto b) { return a + b * b; }))};

            info.energy[i] = band_energy;
        }

        output.emplace_back(info);
    }

    std::ofstream file{output_folder + "/energies.txt"};
    cereal::JSONOutputArchive archive{file};
    archive(cereal::make_nvp("directionality", directionality),
            cereal::make_nvp("energies", output));

    return EXIT_SUCCESS;
}
