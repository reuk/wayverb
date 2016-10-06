#include "run_methods.h"

#include "common/aligned/map.h"

#include "audio_file/audio_file.h"

#include <iostream>

//  TODO test raytracer diffuse output to compenstate for fast img-src decay
//
//  TODO bidirectional mic outputs don't match

template <typename It>
void normalize(It begin, It end) {
    const auto max_mags{
            map_to_vector(begin, end, [](auto i) { return max_mag(i); })};
    const auto mag{*std::max_element(max_mags.begin(), max_mags.end())};
    std::for_each(begin, end, [=](auto& vec) {
        for (auto& samp : vec) {
            samp /= mag;
        }
    });
}

int main(int argc, char** argv) {

    //  constants ------------------------------------------------------------//

    const geo::box box{glm::vec3{0}, glm::vec3{5.56, 3.97, 2.81}};
    constexpr auto sample_rate{16000.0};
    constexpr auto speed_of_sound{340.0};
    constexpr auto acoustic_impedance{400.0f};

    constexpr glm::vec3 source{2.09, 2.12, 2.12};
    constexpr glm::vec3 receiver{2.09, 3.08, 0.96};

    constexpr auto reflectance{0.95};
    const auto absorption{1 - pow(reflectance, 2)};

    const aligned::map<std::string, float> polar_pattern_map{
            {"omnidirectional", 0.0f},
            {"cardioid", 0.5f},
            {"bidirectional", 1.0f}};

    //  simulations ----------------------------------------------------------//

    for (const auto& pair : polar_pattern_map) {
        const microphone mic{glm::vec3{0, 0, 1}, pair.second};

        auto waveguide{run(box,
                           absorption,
                           source,
                           receiver,
                           mic,
                           speed_of_sound,
                           acoustic_impedance,
                           sample_rate,
                           run_waveguide)};

        auto exact_img_src{run(box,
                               absorption,
                               source,
                               receiver,
                               mic,
                               speed_of_sound,
                               acoustic_impedance,
                               sample_rate,
                               run_exact_img_src)};

        auto fast_img_src{run(box,
                              absorption,
                              source,
                              receiver,
                              mic,
                              speed_of_sound,
                              acoustic_impedance,
                              sample_rate,
                              run_fast_img_src)};

        //  postprocessing ---------------------------------------------------//

        auto waveguide_pressure{
                map_to_vector(waveguide, [](auto i) { return i.pressure; })};
        waveguide_filter(waveguide_pressure.begin(),
                         waveguide_pressure.end(),
                         sample_rate);

        auto waveguide_attenuated{waveguide::attenuate(
                mic, acoustic_impedance, waveguide.begin(), waveguide.end())};
        waveguide_filter(waveguide_attenuated.begin(),
                         waveguide_attenuated.end(),
                         sample_rate);

        const auto outputs = {&waveguide_pressure,
                              &waveguide_attenuated,
                              &exact_img_src,
                              &fast_img_src};

        const auto make_iterator{[](auto i) {
            return make_mapping_iterator_adapter(
                    std::move(i), [](auto& i) -> auto& { return *i; });
        }};
        normalize(make_iterator(outputs.begin()), make_iterator(outputs.end()));

        write(build_string(pair.first, ".waveguide_pressure.wav"),
              make_audio_file(waveguide_pressure, sample_rate),
              16);
        write(build_string(pair.first, ".waveguide_attenuated.wav"),
              make_audio_file(waveguide_attenuated, sample_rate),
              16);
        write(build_string(pair.first, ".exact_img_src.wav"),
              make_audio_file(exact_img_src, sample_rate),
              16);
        write(build_string(pair.first, ".fast_img_src.wav"),
              make_audio_file(fast_img_src, sample_rate),
              16);
    }

    return EXIT_SUCCESS;
}
