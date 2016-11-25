#include "box/img_src.h"
#include "box/poly.h"

#include "combined/engine.h"
#include "combined/waveguide_base.h"

#include "raytracer/postprocess.h"

#include "waveguide/canonical.h"
#include "waveguide/postprocess.h"

#include "core/cl/iterator.h"
#include "core/reverb_time.h"

#include "utilities/aligned/map.h"
#include "utilities/for_each.h"
#include "utilities/named_value.h"
#include "utilities/progress_bar.h"
#include "utilities/string_builder.h"
#include "utilities/tuple_like.h"
#include "utilities/type_debug.h"

#include "audio_file/audio_file.h"

#include <iomanip>
#include <iostream>

template <typename It>
void write_tuple(It b, It e, const char* prefix, double sample_rate) {
    for_each(b, e, [&](const auto& i) {
        write(util::build_string(prefix, ".", i.name, ".wav").c_str(),
              i.value,
              sample_rate,
              audio_file::format::wav,
              audio_file::bit_depth::pcm16);
    });
}

////////////////////////////////////////////////////////////////////////////////

struct max_mag_functor final {
    template <typename T>
    auto operator()(T&& t) const {
        return wayverb::core::max_mag(t.value);
    }
};

////////////////////////////////////////////////////////////////////////////////

int main(/*int argc, char** argv*/) {
    //  constants //////////////////////////////////////////////////////////////

    const auto box =
            wayverb::core::geo::box{glm::vec3{0}, glm::vec3{5.56, 3.97, 2.81}};
    constexpr auto sample_rate = 5000.0;

    constexpr auto source = glm::vec3{2.09, 2.12, 2.12};
    constexpr auto receiver = glm::vec3{2.09, 3.08, 0.96};
    constexpr wayverb::core::environment environment{};
    constexpr glm::vec3 pointing{0, 0, 1};
    constexpr glm::vec3 up{0, 1, 0};

    constexpr auto scattering_surface =
            wayverb::core::surface<wayverb::core::simulation_bands>{
                    {{0.07, 0.09, 0.11, 0.12, 0.13, 0.14, 0.16, 0.17}},
                    {{0.07, 0.09, 0.11, 0.12, 0.13, 0.14, 0.16, 0.17}}};

    const auto scene_data =
            wayverb::core::geo::get_scene_data(box, scattering_surface);

    const wayverb::core::compute_context cc{};
    const auto voxelised = wayverb::waveguide::compute_voxels_and_mesh(
            cc, scene_data, receiver, sample_rate, environment.speed_of_sound);

    const auto room_volume = estimate_room_volume(scene_data);
    const auto eyring = eyring_reverb_time(scene_data, 0.0f);
    const auto max_time = max_element(eyring);

    constexpr auto usable_portion = 0.6;
    constexpr auto waveguide_cutoff =
            wayverb::waveguide::compute_cutoff_frequency(sample_rate,
                                                         usable_portion);
    std::cout << "waveguide cutoff: " << waveguide_cutoff << "Hz\n";

    constexpr auto volume_factor = 0.1;

    //  tests //////////////////////////////////////////////////////////////////

    util::aligned::vector<std::unique_ptr<renderer>> renderers;

    if (true) {
        //  engine /////////////////////////////////////////////////////////////
        renderers.emplace_back(make_concrete_renderer_ptr([&] {
            auto input =
                    wayverb::combined::engine{
                            cc,
                            scene_data,
                            source,
                            receiver,
                            environment,
                            wayverb::raytracer::simulation_parameters{1 << 16,
                                                                      4},
                            wayverb::combined::make_waveguide_ptr(
                                    wayverb::waveguide::single_band_parameters{
                                                    waveguide_cutoff,
                                                    usable_portion})}
                            .run(true);

            return [&, input = std::move(input) ](const auto& attenuator) {
                return util::make_named_value("engine", input->postprocess(attenuator, sample_rate));
            };
        }));
    }

    if (false) {
        //  raytracer //////////////////////////////////////////////////////////
        renderers.emplace_back(make_concrete_renderer_ptr([&] {
            auto input = wayverb::raytracer::canonical(
                    cc,
                    voxelised.voxels,
                    source,
                    receiver,
                    environment,
                    wayverb::raytracer::simulation_parameters{1 << 16, 5},
                    0,
                    true,
                    [](auto /*step*/, auto /*steps*/) {});
            return [&, input = std::move(input) ](const auto& attenuator) {
                return util::make_named_value(
                        "raytracer",
                        wayverb::raytracer::postprocess(input->aural,
                                                        attenuator,
                                                        receiver,
                                                        room_volume,
                                                        environment,
                                                        sample_rate));
            };
        }));
    }

    if (false) {
        //  image source ///////////////////////////////////////////////////////
        renderers.emplace_back(make_concrete_renderer_ptr([&] {
            auto input = run_exact_img_src(
                    box,
                    wayverb::core::surface<wayverb::core::simulation_bands>{
                            scattering_surface.absorption,
                            wayverb::core::bands_type{}},
                    source,
                    receiver,
                    environment,
                    max_time,
                    false);

            return [&, input = std::move(input) ](const auto& attenuator) {
                return util::make_named_value(
                        "img_src",
                        wayverb::raytracer::image_source::postprocess(
                                begin(input),
                                end(input),
                                attenuator,
                                receiver,
                                environment.speed_of_sound,
                                sample_rate));
            };
        }));
    }

    const auto make_waveguide_renderer = [&](const auto& name,
                                             const auto& waveguide_params) {
        return make_concrete_renderer_ptr([&] {
            util::progress_bar pb;
            auto input = *wayverb::waveguide::canonical(
                    cc,
                    voxelised,
                    source,
                    receiver,
                    environment,
                    waveguide_params,
                    max_element(eyring_reverb_time(scene_data, 0.0)),
                    true,
                    [&](auto& /*queue*/,
                        const auto& /*buffer*/,
                        auto step,
                        auto steps) { set_progress(pb, step, steps); });

            return [&, input = std::move(input) ](const auto& attenuator) {
                return util::make_named_value(
                        name,
                        wayverb::waveguide::postprocess(
                                input,
                                attenuator,
                                environment.acoustic_impedance,
                                sample_rate));
            };
        });
    };

    if (false) {
        //  single band ////////////////////////////////////////////////////////
        renderers.emplace_back(make_waveguide_renderer(
                "waveguide.single_band",
                wayverb::waveguide::single_band_parameters{waveguide_cutoff,
                                                           usable_portion}));
    }

    if (false) {
        //  multiple band constant spacing /////////////////////////////////////
        renderers.emplace_back(make_waveguide_renderer(
                "waveguide.multiple_band.constant_spacing",
                wayverb::waveguide::multiple_band_constant_spacing_parameters{
                        3, waveguide_cutoff, usable_portion}));
    }

    const auto rendered = util::map_to_vector(
            begin(renderers), end(renderers), [](const auto& i) {
                return i->render();
            });

    util::for_each(
            [&](const auto& tup) {
                auto processed = util::map_to_vector(
                        begin(rendered), end(rendered), [&](const auto& i) {
                            return i->process(std::get<1>(tup));
                        });

                constexpr auto normalize = false;
                if (normalize) {
                    const auto make_iterator = [](auto it) {
                        return make_mapping_iterator_adapter(std::move(it),
                                                             max_mag_functor{});
                    };

                    const auto max_magnitude =
                            *std::max_element(make_iterator(begin(processed)),
                                              make_iterator(end(processed)));

                    const auto norm_factor = 1.0 / max_magnitude;

                    std::cout << "norm factor: " << norm_factor << '\n';

                    for (auto& i : processed) {
                        wayverb::core::mul(i.value, norm_factor);
                    }
                } else {
                    for (auto& i : processed) {
                        wayverb::core::mul(i.value, volume_factor);
                    }
                }

                write_tuple(begin(processed),
                            end(processed),
                            std::get<0>(tup),
                            sample_rate);
            },

            std::make_tuple(
                    std::make_tuple("null", wayverb::core::attenuator::null{})/*,
                    std::make_tuple(
                            "omnidirectional",
                            wayverb::core::attenuator::microphone{
                                    wayverb::core::orientation{pointing},
                                    0.0f}),
std::make_tuple("cardioid",
wayverb::core::attenuator::microphone{pointing, 0.5f}),
std::make_tuple("bidirectional",
wayverb::core::attenuator::microphone{pointing, 1.0f}),
std::make_tuple(
"hrtf_l",
attenuator::hrtf{pointing,
up,
wayverb::core::attenuator::hrtf::channel::left}),
std::make_tuple(
"hrtf_r",
attenuator::hrtf{pointing,
up,
wayverb::core::attenuator::hrtf::channel::right})*/
                    ));

    return EXIT_SUCCESS;
}
