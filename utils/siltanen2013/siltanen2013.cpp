#include "box/img_src.h"
#include "box/raytracer.h"
#include "box/waveguide.h"

#include "common/attenuator/hrtf.h"
#include "common/attenuator/microphone.h"
#include "common/cl/iterator.h"
#include "common/reverb_time.h"

#include "utilities/aligned/map.h"
#include "utilities/named_value.h"
#include "utilities/string_builder.h"
#include "utilities/type_debug.h"

#include "audio_file/audio_file.h"

#include <iostream>

//  TODO mic output modal responses don't match

//  TODO proper crossover filter

//  Absorption = proportion of energy lost to reflecting surface
//  Diffusion = proportion of energy directly reflected to diffusely reflected

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
    const glm::vec3 pointing{0, 0, 1};
    const glm::vec3 up{0, 1, 0};

    constexpr auto reflectance{0.95};
    const auto absorption{1 - pow(reflectance, 2)};

    const auto scattering{0.1};

    const aligned::map<std::string, float> polar_pattern_map{
            {"omnidirectional", 0.0f},
            {"cardioid", 0.5f},
            {"bidirectional", 1.0f}};

    const auto eyring{
            eyring_reverb_time(geo::get_scene_data(box, absorption), 0)};

    std::cerr << "expected reverb time: " << eyring << '\n';

    //  tests ----------------------------------------------------------------//

    const auto raytracer{make_named_value("raytracer",
                                          run_raytracer(box,
                                                        absorption,
                                                        scattering,
                                                        source,
                                                        receiver,
                                                        acoustic_impedance))};

    /*
    const auto waveguide{make_named_value("waveguide",
                                          run_waveguide(box,
                                                        absorption,
                                                        source,
                                                        receiver,
                                                        speed_of_sound,
                                                        acoustic_impedance,
                                                        sample_rate,
                                                        eyring))};

    const auto exact_img_src{
            make_named_value("exact_img_src",
                             run_exact_img_src(box,
                                               absorption,
                                               source,
                                               receiver,
                                               speed_of_sound,
                                               acoustic_impedance,
                                               eyring))};

    const auto fast_img_src{make_named_value(
            "fast_img_src",
            run_fast_img_src(
                    box, absorption, source, receiver, acoustic_impedance))};
    */

    const auto normalize_and_write{[&](auto prefix, auto b, auto e) {
        const auto make_iterator{[](auto i) {
            return make_mapping_iterator_adapter(
                    std::move(i),
                    [](const auto& i) -> auto& { return i->value; });
        }};

        normalize(make_iterator(b), make_iterator(e));

        for (; b != e; ++b) {
            write(build_string(prefix, ".", (*b)->name, ".wav"),
                  audio_file::make_audio_file((*b)->value, sample_rate),
                  16);
        }
    }};

    {
        const auto run_postprocess_impulses{[&](const auto& in) {
            return map(in, [&](const auto& i) {
                return raytracer::postprocess(begin(i),
                                              end(i),
                                              receiver,
                                              speed_of_sound,
                                              sample_rate,
                                              eyring);
            });
        }};

        auto raytracer_p{run_postprocess_impulses(raytracer)};

        /*
        auto exact_img_src_p{run_postprocess_impulses(exact_img_src)};
        auto fast_img_src_p{run_postprocess_impulses(fast_img_src)};

        auto waveguide_p{map(waveguide, [&](const auto& i) {
            return postprocess_waveguide(begin(i), end(i), sample_rate);
        })};

        const auto results = {
                &waveguide_p, &exact_img_src_p, &fast_img_src_p, &raytracer_p};
        */

        const auto results = {&raytracer_p};

        normalize_and_write("no_processing", begin(results), end(results));
    }

    /*

    //  postprocessing -------------------------------------------------------//

    const auto run_postprocess_impulses{
            [&](auto begin, auto end, auto attenuator) {
                return raytracer::postprocess(std::move(begin),
                                              std::move(end),
                                              std::move(attenuator),
                                              receiver,
                                              speed_of_sound,
                                              sample_rate,
                                              eyring);
            }};

    const auto postprocess{[&](auto prefix, auto attenuator) {
        auto waveguide_p{map(waveguide, [&](const auto& i) {
            return postprocess_waveguide(begin(i),
                                         end(i),
                                         attenuator,
                                         sample_rate,
                                         acoustic_impedance);
        })};
        auto exact_img_src_p{map(exact_img_src, [&](const auto& i) {
            return run_postprocess_impulses(begin(i), end(i), attenuator);
        })};
        auto fast_img_src_p{map(fast_img_src, [&](const auto& i) {
            return run_postprocess_impulses(begin(i), end(i), attenuator);
        })};
        auto raytracer_p{map(raytracer, [&](const auto& i) {
            return run_postprocess_impulses(begin(i), end(i), attenuator);
        })};

        const auto results = {
                &waveguide_p, &exact_img_src_p, &fast_img_src_p, &raytracer_p};

        normalize_and_write(prefix, begin(results), end(results));
    }};

    //  simulations ----------------------------------------------------------//

    for (const auto& pair : polar_pattern_map) {
        postprocess(pair.first, microphone{pointing, pair.second});
    }

    postprocess("hrtf_l", hrtf{pointing, up, hrtf::channel::left});
    postprocess("hrtf_r", hrtf{pointing, up, hrtf::channel::right});

    */

    return EXIT_SUCCESS;
}
