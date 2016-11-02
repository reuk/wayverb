#include "box/img_src.h"
#include "box/raytracer.h"

#include "combined/engine.h"

#include "raytracer/postprocess.h"

#include "waveguide/canonical.h"
#include "waveguide/postprocess.h"

#include "core/attenuator/hrtf.h"
#include "core/attenuator/microphone.h"
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
        write(util::build_string(prefix, ".", i.name, ".wav"),
              audio_file::make_audio_file(i.value, sample_rate),
              16);
    });
}

////////////////////////////////////////////////////////////////////////////////

struct processor {
    processor() = default;
    processor(const processor&) = default;
    processor(processor&&) noexcept = default;
    processor& operator=(const processor&) = default;
    processor& operator=(processor&&) noexcept = default;
    virtual ~processor() = default;

    virtual util::named_value<util::aligned::vector<float>> process(
            const core::attenuator::null& attenuator) const = 0;
    virtual util::named_value<util::aligned::vector<float>> process(
            const core::attenuator::hrtf& attenuator) const = 0;
    virtual util::named_value<util::aligned::vector<float>> process(
            const core::attenuator::microphone& attenuator) const = 0;
};

template <typename Callback>
class concrete_processor final : public processor {
public:
    explicit concrete_processor(Callback callback)
            : callback_{std::move(callback)} {}

    util::named_value<util::aligned::vector<float>> process(
            const core::attenuator::null& attenuator) const override {
        return callback_(attenuator);
    }
    util::named_value<util::aligned::vector<float>> process(
            const core::attenuator::hrtf& attenuator) const override {
        return callback_(attenuator);
    }
    util::named_value<util::aligned::vector<float>> process(
            const core::attenuator::microphone& attenuator) const override {
        return callback_(attenuator);
    }

private:
    Callback callback_;
};

template <typename Callback>
auto make_concrete_processor_ptr(Callback callback) {
    return std::make_unique<concrete_processor<Callback>>(std::move(callback));
}

////////////////////////////////////////////////////////////////////////////////

struct renderer {
    renderer() = default;
    renderer(const renderer&) = default;
    renderer(renderer&&) noexcept = default;
    renderer& operator=(const renderer&) = default;
    renderer& operator=(renderer&&) noexcept = default;
    virtual ~renderer() noexcept = default;

    virtual std::unique_ptr<processor> render() const = 0;
};

template <typename Callback>
class concrete_renderer final : public renderer {
public:
    explicit concrete_renderer(Callback callback)
            : callback_{std::move(callback)} {}

    std::unique_ptr<processor> render() const override {
        return make_concrete_processor_ptr(callback_());
    }

private:
    Callback callback_;
};

template <typename Callback>
auto make_concrete_renderer_ptr(Callback callback) {
    return std::make_unique<concrete_renderer<Callback>>(std::move(callback));
}

////////////////////////////////////////////////////////////////////////////////

struct max_mag_functor final {
    template <typename T>
    auto operator()(T&& t) const {
        return core::max_mag(t.value);
    }
};

int main(int argc, char** argv) {
    //  constants //////////////////////////////////////////////////////////////

    const auto box = core::geo::box{glm::vec3{0}, glm::vec3{5.56, 3.97, 2.81}};
    constexpr auto sample_rate = 10000.0;

    constexpr auto params = core::model::parameters{
            glm::vec3{2.09, 2.12, 2.12}, glm::vec3{2.09, 3.08, 0.96}};
    constexpr glm::vec3 pointing{0, 0, 1};
    constexpr glm::vec3 up{0, 1, 0};

    constexpr auto scattering_surface = core::surface<core::simulation_bands>{
            {{0.1, 0.1, 0.11, 0.12, 0.13, 0.14, 0.16, 0.17}},
            {{0.1, 0.1, 0.11, 0.12, 0.13, 0.14, 0.16, 0.17}}};

    const auto scene_data = core::geo::get_scene_data(box, scattering_surface);

    const auto room_volume = estimate_room_volume(scene_data);
    const auto eyring = eyring_reverb_time(scene_data, 0.0f);
    const auto max_time = max_element(eyring);

    constexpr auto usable_portion = 0.6;

    constexpr auto volume_factor = 0.1;

    //  tests //////////////////////////////////////////////////////////////////

    util::aligned::vector<std::unique_ptr<renderer>> renderers;

    if (true) {
        //  engine /////////////////////////////////////////////////////////////
        renderers.emplace_back(make_concrete_renderer_ptr([&] {
            const auto callback = [](auto state, auto progress) {
                std::cout << '\r' << std::setw(30) << to_string(state)
                          << std::setw(10) << progress << std::flush;
            };

            auto input =
                    combined::engine{
                            core::compute_context{},
                            scene_data,
                            params,
                            raytracer::simulation_parameters{1 << 16, 4},
                            waveguide::
                                    multiple_band_constant_spacing_parameters{
                                            3, sample_rate, usable_portion}}
                            .run(true, callback);

            return [&, input = std::move(input) ](const auto& attenuator) {
                return util::make_named_value(
                        "engine", input->postprocess(attenuator, sample_rate));
            };
        }));
    }

    if (false) {
        //  raytracer //////////////////////////////////////////////////////////
        renderers.emplace_back(make_concrete_renderer_ptr([&] {
            auto input =
                    run_raytracer(box,
                                  scattering_surface,
                                  params,
                                  raytracer::simulation_parameters{1 << 16, 5});
            return [&, input = std::move(input) ](const auto& attenuator) {
                return util::make_named_value(
                        "raytracer",
                        raytracer::postprocess(input,
                                               attenuator,
                                               params.receiver,
                                               room_volume,
                                               params.acoustic_impedance,
                                               params.speed_of_sound,
                                               sample_rate));
            };
        }));
    }

    if (true) {
        //  image source ///////////////////////////////////////////////////////
        renderers.emplace_back(make_concrete_renderer_ptr([&] {
            auto input = run_exact_img_src(
                    box,
                    core::surface<core::simulation_bands>{
                            scattering_surface.absorption, core::bands_type{}},
                    params,
                    max_time,
                    false);

            return [&, input = std::move(input) ](const auto& attenuator) {
                return util::make_named_value(
                        "img_src",
                        raytracer::image_source::postprocess(
                                begin(input),
                                end(input),
                                attenuator,
                                params.receiver,
                                params.speed_of_sound,
                                sample_rate));
            };
        }));
    }

    const auto make_waveguide_renderer = [&](const auto& name,
                                             const auto& waveguide_params) {
        return make_concrete_renderer_ptr([&] {
            util::progress_bar pb;
            auto input = *waveguide::canonical(
                    core::compute_context{},
                    scene_data,
                    params,
                    waveguide_params,
                    max_element(eyring_reverb_time(scene_data, 0.0)),
                    true,
                    [&](auto step, auto steps) {
                        set_progress(pb, step, steps);
                    });

            return [&, input = std::move(input) ](const auto& attenuator) {
                return util::make_named_value(
                        name,
                        waveguide::postprocess(input,
                                               attenuator,
                                               params.acoustic_impedance,
                                               sample_rate));
            };
        });
    };

    if (false) {
        //  single band ////////////////////////////////////////////////////////
        renderers.emplace_back(
                make_waveguide_renderer("waveguide.single_band",
                                        waveguide::single_band_parameters{
                                                sample_rate, usable_portion}));
    }

    if (false) {
        //  multiple band constant spacing /////////////////////////////////////
        renderers.emplace_back(make_waveguide_renderer(
                "waveguide.multiple_band.constant_spacing",
                waveguide::multiple_band_constant_spacing_parameters{
                        3, sample_rate, usable_portion}));
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
                        core::mul(i.value, norm_factor);
                    }
                } else {
                    for (auto& i : processed) {
                        core::mul(i.value, volume_factor);
                    }
                }

                write_tuple(begin(processed),
                            end(processed),
                            std::get<0>(tup),
                            sample_rate);
            },

            std::make_tuple(std::make_tuple("null", core::attenuator::null{})
                            /*,
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
                                    attenuator::microphone{pointing, 1.0f})*/));

    return EXIT_SUCCESS;
}
