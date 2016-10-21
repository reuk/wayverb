#include "box/img_src.h"
#include "box/raytracer.h"
#include "box/waveguide.h"

#include "raytracer/postprocess.h"

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

template <typename It>
void normalize(It begin, It end) {
    const auto max_mags =
            map_to_vector(begin, end, [](auto i) { return max_mag(i); });
    const auto mag = *std::max_element(max_mags.begin(), max_mags.end());
    std::for_each(begin, end, [=](auto& vec) {
        for (auto& samp : vec) {
            samp /= mag;
        }
    });
}

int main(int argc, char** argv) {
    //  constants ------------------------------------------------------------//

    const auto box = geo::box{glm::vec3{0}, glm::vec3{5.56, 3.97, 2.81}};
    constexpr auto sample_rate = 16000.0;

    constexpr model::parameters params{glm::vec3{2.09, 2.12, 2.12},
                                       glm::vec3{2.09, 3.08, 0.96}};
    constexpr glm::vec3 pointing{0, 0, 1};
    constexpr glm::vec3 up{0, 1, 0};

    // constexpr auto reflectance=0.99;
    // const auto absorption=1 - pow(reflectance, 2);
    // constexpr auto scattering=0.05;

    constexpr auto scattering_surface = surface<simulation_bands>{
            {{0.1, 0.1, 0.1, 0.1, 0.12, 0.14, 0.16, 0.17}},
            {{0.1, 0.1, 0.1, 0.1, 0.12, 0.14, 0.16, 0.17}}};

    //constexpr auto no_scattering_surface = surface<simulation_bands>{
    //        scattering_surface.absorption, bands_type{}};

    const auto scene_data = geo::get_scene_data(box, scattering_surface);

    const auto room_volume = estimate_room_volume(scene_data);
    const auto eyring = eyring_reverb_time(scene_data, 0.0f);
    const auto max_time = max_element(eyring);

    //  tests ----------------------------------------------------------------//

    const auto raytracer_raw = make_named_value(
            "raytracer", run_raytracer(box, scattering_surface, params, 4));

    /*

    const auto waveguide_raw = make_named_value(
            "waveguide",
            run_waveguide(
                    box, scattering_surface, params, sample_rate, max_time));

    const auto exact_img_src_raw = make_named_value(
            "exact_img_src",
            run_exact_img_src(
                    box, no_scattering_surface, params, max_time, true));

    const auto fast_img_src_raw = make_named_value(
            "fast_img_src",
            run_fast_img_src(box, scattering_surface, params, true));

    */

    const auto normalize_and_write = [&](auto prefix, auto b, auto e) {
        const auto make_iterator = [](auto i) {
            return make_mapping_iterator_adapter(
                    std::move(i),
                    [](const auto& i) -> auto& { return i->value; });
        };

        normalize(make_iterator(b), make_iterator(e));

        for (; b != e; ++b) {
            write(build_string(prefix, ".", (*b)->name, ".wav"),
                  audio_file::make_audio_file((*b)->value, sample_rate),
                  16);
        }
    };

    //  postprocessing -------------------------------------------------------//

    const auto postprocess = [&](auto prefix, auto attenuator) {
        auto raytracer_p = map(
                [&](const auto& i) {
                    return raytracer::postprocess(i,
                                                  attenuator,
                                                  params.receiver,
                                                  room_volume,
                                                  params.acoustic_impedance,
                                                  params.speed_of_sound,
                                                  sample_rate,
                                                  max_time);
                },
                raytracer_raw);

        /*
         
        auto waveguide_p = map(
                [&](const auto& i) {
                    return postprocess_waveguide(begin(i),
                                                 end(i),
                                                 attenuator,
                                                 sample_rate,
                                                 params.acoustic_impedance);
                },
                waveguide_raw);

        const auto run_postprocess_impulses = [&](
                auto b, auto e, auto attenuator) {
            return raytracer::image_source::postprocess(std::move(b),
                                                        std::move(e),
                                                        attenuator,
                                                        params.receiver,
                                                        params.speed_of_sound,
                                                        sample_rate,
                                                        max_time);
        };

        auto exact_img_src_p = map(
                [&](const auto& i) {
                    return run_postprocess_impulses(
                            begin(i), end(i), attenuator);
                },
                exact_img_src_raw);

        auto fast_img_src_p = map(
                [&](const auto& i) {
                    return run_postprocess_impulses(
                            begin(i), end(i), attenuator);
                },
                fast_img_src_raw);

        */

        const auto results = {//&waveguide_p, &exact_img_src_p, &fast_img_src_p,
                              &raytracer_p};

        normalize_and_write(prefix, begin(results), end(results));
    };

    postprocess("null", attenuator::null{});

    postprocess(
            "hrtf_l",
            attenuator::hrtf{pointing, up, attenuator::hrtf::channel::left});
    postprocess(
            "hrtf_r",
            attenuator::hrtf{pointing, up, attenuator::hrtf::channel::right});

    const aligned::map<std::string, float> polar_pattern_map{
            {"omnidirectional", 0.0f},
            {"cardioid", 0.5f},
            {"bidirectional", 1.0f}};

    for (const auto& pair : polar_pattern_map) {
        postprocess(pair.first, attenuator::microphone{pointing, pair.second});
    }

    return EXIT_SUCCESS;
}
