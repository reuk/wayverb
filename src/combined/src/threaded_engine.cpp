#include "combined/threaded_engine.h"
#include "combined/forwarding_call.h"
#include "combined/waveguide_base.h"

#include "core/dsp_vector_ops.h"
#include "core/environment.h"

#include "waveguide/mesh.h"

#include "audio_file/audio_file.h"

namespace wayverb {
namespace combined {
namespace {

struct max_mag_functor final {
    template <typename T>
    auto operator()(const T& t) const {
        return core::max_mag(t.data);
    }
};

struct channel_info final {
    util::aligned::vector<float> data;
    std::string source_name;
    std::string receiver_name;
    std::string capsule_name;
};

}  // namespace

std::unique_ptr<capsule_base> polymorphic_capsule_model(
        const model::capsule& i, const core::orientation& orientation) {
    switch (i.get_mode()) {
        case model::capsule::mode::microphone:
            return make_capsule_ptr(i.microphone()->get(), orientation);
        case model::capsule::mode::hrtf:
            return make_capsule_ptr(i.hrtf()->get(), orientation);
    }
}

std::unique_ptr<waveguide_base> polymorphic_waveguide_model(
        const model::waveguide& i) {
    switch (i.get_mode()) {
        case model::waveguide::mode::single:
            return make_waveguide_ptr(i.single_band()->get());
        case model::waveguide::mode::multiple:
            return make_waveguide_ptr(i.multiple_band()->get());
    }
}

////////////////////////////////////////////////////////////////////////////////

void complete_engine::run(const core::compute_context& compute_context,
                          const core::gpu_scene_data& scene_data,
                          const model::persistent& persistent) {
    try {
        is_running_ = true;
        keep_going_ = true;

        //  First, we check that all the sources and receivers are valid, to
        //  avoid doing useless work.

        {
            const auto voxelised =
                    core::make_voxelised_scene_data(scene_data, 5, 0.1f);

            for (const auto& source : *persistent.sources()) {
                if (!inside(voxelised, source->position()->get())) {
                    throw std::runtime_error{"source is outside mesh"};
                }
            }

            for (const auto& receiver : *persistent.receivers()) {
                if (!inside(voxelised, receiver->position()->get())) {
                    throw std::runtime_error{"receiver is outside mesh"};
                }
            }
        }

        constexpr core::environment environment{};

        const auto poly_waveguide =
                polymorphic_waveguide_model(*persistent.waveguide());

        std::vector<channel_info> all_channels;

        //  For each source-receiver pair.
        for (auto source = std::begin(*persistent.sources()),
                  e_source = std::end(*persistent.sources());
             source != e_source && keep_going_;
             ++source) {
            for (auto receiver = std::begin(*persistent.receivers()),
                      e_receiver = std::end(*persistent.receivers());
                 receiver != e_receiver && keep_going_;
                 ++receiver) {
                //  Set up an engine to use.
                postprocessing_engine eng{compute_context,
                                          scene_data,
                                          (*source)->position()->get(),
                                          (*receiver)->position()->get(),
                                          environment,
                                          persistent.raytracer()->get(),
                                          poly_waveguide->clone()};

                //  Send new node position notification.
                waveguide_node_positions_changed_(
                        eng.get_voxels_and_mesh().mesh.get_descriptor());

                //  Register callbacks.
                if (!engine_state_changed_.empty()) {
                    eng.add_engine_state_changed_callback(
                            make_forwarding_call(engine_state_changed_));
                }

                if (!waveguide_node_pressures_changed_.empty()) {
                    eng.add_waveguide_node_pressures_changed_callback(
                            make_forwarding_call(
                                    waveguide_node_pressures_changed_));
                }

                if (!raytracer_reflections_generated_.empty()) {
                    eng.add_raytracer_reflections_generated_callback(
                            make_forwarding_call(
                                    raytracer_reflections_generated_));
                }

                const auto polymorphic_capsules = util::map_to_vector(
                        std::begin(*(*receiver)->capsules()),
                        std::end(*(*receiver)->capsules()),
                        [&](const auto& i) {
                            return polymorphic_capsule_model(
                                    *i, (*receiver)->get_orientation());
                        });

                //  Run the simulation, cache the result.
                auto channel = eng.run(begin(polymorphic_capsules),
                                       end(polymorphic_capsules),
                                       persistent.output()->get_sample_rate(),
                                       keep_going_);

                //  If user cancelled while processing the channel, channel
                //  will be null, but we want to exit before throwing an
                //  exception.
                if (!keep_going_) {
                    break;
                }

                if (!channel) {
                    throw std::runtime_error{"encountered unanticipated error"};
                }

                for (size_t i = 0, e = (*receiver)->capsules()->size(); i != e;
                     ++i) {
                    all_channels.emplace_back(channel_info{
                            std::move((*channel)[i]),
                            (*source)->get_name(),
                            (*receiver)->get_name(),
                            (*(*receiver)->capsules())[i]->get_name()});
                }
            }
        }

        //  If keep going is false now, then the simulation was cancelled.
        if (keep_going_) {
            if (all_channels.empty()) {
                throw std::runtime_error{"no channels were rendered"};
            }

            //  Normalize.
            const auto make_iterator = [](auto it) {
                return util::make_mapping_iterator_adapter(std::move(it),
                                                           max_mag_functor{});
            };

            const auto max_mag =
                    *std::max_element(make_iterator(begin(all_channels)),
                                      make_iterator(end(all_channels)));

            if (max_mag == 0.0f) {
                throw std::runtime_error{"all channels are silent"};
            }

            const auto factor = 1.0 / max_mag;

            for (auto& channel : all_channels) {
                for (auto& sample : channel.data) {
                    sample *= factor;
                }
            }

            //  Write out files.
            for (const auto& i : all_channels) {
                const auto file_name = util::build_string(
                        persistent.output()->get_output_folder(),
                        '/',
                        persistent.output()->get_name(),
                        '.',
                        "s_",
                        i.source_name,
                        '.',
                        "r_",
                        i.receiver_name,
                        '.',
                        "c_",
                        i.capsule_name,
                        ".wav");

                write(file_name,
                      audio_file::make_audio_file(
                              i.data, persistent.output()->get_sample_rate()),
                      convert_bit_depth(persistent.output()->get_bit_depth()));
            }
        }

    } catch (const std::exception& e) {
        encountered_error_(e.what());
    }

    is_running_ = false;

    finished_();
}

engine_state_changed::connection
complete_engine::add_engine_state_changed_callback(
        engine_state_changed::callback_type callback) {
    return engine_state_changed_.connect(std::move(callback));
}

waveguide_node_positions_changed::connection
complete_engine::add_waveguide_node_positions_changed_callback(
        waveguide_node_positions_changed::callback_type callback) {
    return waveguide_node_positions_changed_.connect(std::move(callback));
}

waveguide_node_pressures_changed::connection
complete_engine::add_waveguide_node_pressures_changed_callback(
        waveguide_node_pressures_changed::callback_type callback) {
    return waveguide_node_pressures_changed_.connect(std::move(callback));
}

raytracer_reflections_generated::connection
complete_engine::add_raytracer_reflections_generated_callback(
        raytracer_reflections_generated::callback_type callback) {
    return raytracer_reflections_generated_.connect(std::move(callback));
}

complete_engine::encountered_error::connection
complete_engine::add_encountered_error_callback(
        encountered_error::callback_type callback) {
    return encountered_error_.connect(std::move(callback));
}

complete_engine::finished::connection complete_engine::add_finished_callback(
        finished::callback_type callback) {
    return finished_.connect(std::move(callback));
}

}  // namespace combined
}  // namespace wayverb
