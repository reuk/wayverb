#include "box/img_src.h"
#include "box/raytracer.h"
#include "box/waveguide.h"

#include "combined/engine.h"

#include "raytracer/postprocess.h"

#include "common/attenuator/hrtf.h"
#include "common/attenuator/microphone.h"
#include "common/cl/iterator.h"
#include "common/reverb_time.h"

#include "utilities/aligned/map.h"
#include "utilities/for_each.h"
#include "utilities/named_value.h"
#include "utilities/string_builder.h"
#include "utilities/type_debug.h"

#include "audio_file/audio_file.h"

#include <iomanip>
#include <iostream>

template <typename Collection>
void write_tuple(const char* prefix,
                 double sample_rate,
                 Collection&& collection) {
    for_each(
            [&](const auto& i) {
                write(build_string(prefix, ".", i.name, ".wav"),
                      audio_file::make_audio_file(i.value, sample_rate),
                      16);
            },
            collection);
}

////////////////////////////////////////////////////////////////////////////////

template <typename Input>
struct raytracer_processor final {
    Input input;
    const model::parameters& params;
    const float& room_volume;
    const float& sample_rate;

    template <typename U>
    auto operator()(const U& attenuator) const {
        return make_named_value(
                "raytracer",
                raytracer::postprocess(input,
                                       attenuator,
                                       params.receiver,
                                       room_volume,
                                       params.acoustic_impedance,
                                       params.speed_of_sound,
                                       sample_rate));
    }
};

template <typename Input>
constexpr auto make_raytracer_processor(Input input,
                                        const model::parameters& params,
                                        const float& room_volume,
                                        const float& sample_rate) {
    return raytracer_processor<Input>{
            std::move(input), params, room_volume, sample_rate};
}

struct raytracer_renderer final {
    const geo::box& box;
    const surface<simulation_bands>& scattering_surface;
    const model::parameters& params;
    const float& room_volume;
    const float& sample_rate;

    auto operator()() const {
        return make_raytracer_processor(
                run_raytracer(box,
                              scattering_surface,
                              params,
                              raytracer::simulation_parameters{1 << 16, 5}),
                params,
                room_volume,
                sample_rate);
    }
};

////////////////////////////////////////////////////////////////////////////////

struct waveguide_processor final {
    waveguide::simulation_results input;
    const model::parameters& params;
    const float& sample_rate;

    template <typename U>
    auto operator()(const U& attenuator) const {
        return make_named_value(
                "waveguide",
                postprocess_waveguide(input,
                                      attenuator,
                                      sample_rate,
                                      params.acoustic_impedance));
    }
};

struct waveguide_renderer final {
    const geo::box& box;
    const surface<simulation_bands>& scattering_surface;
    const model::parameters& params;
    const float& sample_rate;
    const float& max_time;

    auto operator()() const {
        return waveguide_processor{
                run_waveguide(
                        box, scattering_surface, params, sample_rate, max_time),
                params,
                sample_rate};
    }
};

////////////////////////////////////////////////////////////////////////////////

struct img_src_processor final {
    aligned::vector<impulse<simulation_bands>> input;
    const model::parameters& params;
    const float& sample_rate;

    template <typename U>
    auto operator()(const U& attenuator) const {
        return make_named_value(
                "img_src",
                raytracer::image_source::postprocess(begin(input),
                                                     end(input),
                                                     attenuator,
                                                     params.receiver,
                                                     params.speed_of_sound,
                                                     sample_rate));
    }
};

struct img_src_renderer final {
    const geo::box& box;
    const surface<simulation_bands>& scattering_surface;
    const model::parameters& params;
    const float& sample_rate;
    const float& max_time;

    auto operator()() const {
        return img_src_processor{
                run_exact_img_src(
                        box,
                        surface<simulation_bands>{scattering_surface.absorption,
                                                  bands_type{}},
                        params,
                        max_time,
                        true),
                params,
                sample_rate};
    }
};

////////////////////////////////////////////////////////////////////////////////

struct engine_processor final {
    std::unique_ptr<wayverb::intermediate> intermediate;
    const float& output_sample_rate;

    template <typename U>
    auto operator()(const U& attenuator) const {
        return make_named_value(
                "engine",
                intermediate->postprocess(attenuator, output_sample_rate));
    }
};

struct engine_renderer final {
    wayverb::engine engine;
    const float& output_sample_rate;
    auto operator()() const {
        const auto callback = [](auto state, auto progress) {
            std::cout << '\r' << std::setw(30) << to_string(state)
                      << std::setw(10) << progress << std::flush;
        };

        return engine_processor{engine.run(true, callback), output_sample_rate};
    }
};

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
    //  constants //////////////////////////////////////////////////////////////

    const auto box = geo::box{glm::vec3{0}, glm::vec3{5.56, 3.97, 2.81}};
    constexpr auto sample_rate = 44100.0;

    constexpr auto source = glm::vec3{2.09, 2.12, 2.12};
    constexpr auto receiver = glm::vec3{2.09, 3.08, 0.96};
    constexpr glm::vec3 pointing{0, 0, 1};
    constexpr glm::vec3 up{0, 1, 0};

    constexpr auto scattering_surface = surface<simulation_bands>{
            {{0.1, 0.1, 0.1, 0.1, 0.12, 0.14, 0.16, 0.17}},
            {{0.1, 0.1, 0.1, 0.1, 0.12, 0.14, 0.16, 0.17}}};

    const auto scene_data = geo::get_scene_data(box, scattering_surface);

    // const auto room_volume = estimate_room_volume(scene_data);
    // const auto eyring = eyring_reverb_time(scene_data, 0.0f);
    // const auto max_time = max_element(eyring);

    //  tests //////////////////////////////////////////////////////////////////

    const auto rendered = apply_each(std::make_tuple(
            engine_renderer{
                    wayverb::engine{
                            compute_context{},
                            scene_data,
                            source,
                            receiver,
                            raytracer::simulation_parameters{1 << 16, 4},
                            waveguide::single_band_parameters{10000.0, 0.6}},
                    sample_rate}

            // raytracer_renderer{
            //         box, scattering_surface, params, room_volume,
            //         sample_rate}
            // waveguide_renderer{
            //        box, scattering_surface, params, sample_rate, max_time}
            // img_src_renderer{
            //        box, scattering_surface, params, sample_rate, max_time}
            ));

    for_each(
            [&](const auto& tup) {
                auto processed =
                        apply_each(rendered, std::make_tuple(std::get<1>(tup)));
                const auto max_magnitude = foldl(
                        [](auto a, auto b) { return std::max(a, b); },
                        map([](const auto& i) { return max_mag(i.value); },
                            processed));
                for_each([&](auto& i) { mul(i.value, 1.0 / max_magnitude); },
                         processed);
                write_tuple(std::get<0>(tup), sample_rate, processed);
            },
            std::make_tuple(
//                    std::make_tuple("null", attenuator::null{}),
                    std::make_tuple(
                            "hrtf_l",
                            attenuator::hrtf{pointing,
                                             up,
                                             attenuator::hrtf::channel::left}),
                    std::make_tuple(
                            "hrtf_r",
                            attenuator::hrtf{pointing,
                                             up,
                                             attenuator::hrtf::channel::right}),
                    std::make_tuple("omnidirectional",
                                    attenuator::microphone{pointing, 0.0f}),
                    std::make_tuple("cardioid",
                                    attenuator::microphone{pointing, 0.5f}),
                    std::make_tuple("bidirectional",
                                    attenuator::microphone{pointing, 1.0f})));

    return EXIT_SUCCESS;
}
