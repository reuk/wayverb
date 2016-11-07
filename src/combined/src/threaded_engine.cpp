#include "combined/threaded_engine.h"

namespace wayverb {
namespace combined {

std::unique_ptr<capsule_base> polymorphic_capsule_info(const capsule_info& i) {
    switch (i.mode) {
        case capsule_info::capsule_mode::microphone:
            return make_capsule_ptr(i.microphone);
        case capsule_info::capsule_mode::hrtf: return make_capsule_ptr(i.hrtf);
    }
}

std::unique_ptr<waveguide_base> polymorphic_waveguide(const waveguide_info& i) {
    switch (i.mode) {
        case waveguide_info::waveguide_mode::single_band:
            return make_waveguide_ptr(i.single_band);
        case waveguide_info::waveguide_mode::multiple_band:
            return make_waveguide_ptr(i.multiple_band);
    }
}

////////////////////////////////////////////////////////////////////////////////

void complete_engine::run(const core::compute_context& compute_context,
                          const core::gpu_scene_data& scene_data,
                          const scene_parameters& scene_parameters) {
    try {
        is_running_ = true;
        keep_going_ = true;

        if (scene_parameters.sources.empty()) {
            throw std::runtime_error{"no sources specified"};
        }

        if (scene_parameters.receivers.empty()) {
            throw std::runtime_error{"no receivers specified"};
        }

        const auto poly_waveguide =
                polymorphic_waveguide(scene_parameters.waveguide);

        std::vector<channel_info> all_channels;

        //  For each source-receiver pair.
        for (auto source = begin(scene_parameters.sources),
                  e_source = end(scene_parameters.sources);
             source != e_source && keep_going_;
             ++source) {
            for (auto receiver = begin(scene_parameters.receivers),
                      e_receiver = end(scene_parameters.receivers);
                 receiver != e_receiver && keep_going_;
                 ++receiver) {
                //  Set up an engine to use.
                postprocessing_engine eng{compute_context,
                                          scene_data,
                                          source->position,
                                          receiver->position,
                                          scene_parameters.environment,
                                          scene_parameters.raytracer,
                                          poly_waveguide->clone()};

                //  Send new node position notification.
                waveguide_node_positions_changed_(
                        eng.get_voxels_and_mesh().mesh.get_descriptor());

                //  Register callbacks.
                const auto engine_state_change_connector =
                        eng.add_engine_state_changed_callback(
                                make_forwarding_call(engine_state_changed_));

                const auto node_pressure_connector =
                        eng.add_waveguide_node_pressures_changed_callback(
                                make_forwarding_call(
                                        waveguide_node_pressures_changed_));

                const auto raytracer_reflection_connector =
                        eng.add_raytracer_reflections_generated_callback(
                                make_forwarding_call(
                                        raytracer_reflections_generated_));

                const auto polymorphic_capsules =
                        util::map_to_vector(begin(receiver->capsules),
                                            end(receiver->capsules),
                                            polymorphic_capsule_info);

                //  Run the simulation, cache the result.
                auto channel = eng.run(begin(polymorphic_capsules),
                                       end(polymorphic_capsules),
                                       scene_parameters.output.sample_rate,
                                       keep_going_);

                if (!keep_going_) {
                    throw std::runtime_error{"simulation cancelled"};
                }

                if (!channel) {
                    throw std::runtime_error{"encountered unanticipated error"};
                }

                for (size_t i = 0, e = receiver->capsules.size(); i != e; ++i) {
                    all_channels.emplace_back(
                            channel_info{std::move((*channel)[i]),
                                         source->name,
                                         receiver->name,
                                         receiver->capsules[i].name});
                }
            }
        }

        //  If keep going is false now, then the simulation was cancelled.
        if (!keep_going_) {
            throw std::runtime_error{"simulation cancelled"};
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
            const auto file_name =
                    util::build_string(scene_parameters.output.output_folder,
                                       '/',
                                       scene_parameters.output.name,
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
                          i.data, scene_parameters.output.sample_rate),
                  convert_bit_depth(scene_parameters.output.bit_depth));
        }

    } catch (const std::exception& e) {
        encountered_error_(e.what());
    }

    finished_();

    is_running_ = false;
}

engine_state_changed::connection
complete_engine::add_engine_state_changed_callback(
        engine_state_changed::callback_type callback) {
    if (!is_running_) {
        return engine_state_changed_.connect(std::move(callback));
    }
    return {};
}

waveguide_node_positions_changed::connection
complete_engine::add_waveguide_node_positions_changed_callback(
        waveguide_node_positions_changed::callback_type callback) {
    if (!is_running_) {
        return waveguide_node_positions_changed_.connect(std::move(callback));
    }
    return {};
}

waveguide_node_pressures_changed::connection
complete_engine::add_waveguide_node_pressures_changed_callback(
        waveguide_node_pressures_changed::callback_type callback) {
    if (!is_running_) {
        return waveguide_node_pressures_changed_.connect(std::move(callback));
    }
    return {};
}

raytracer_reflections_generated::connection
complete_engine::add_raytracer_reflections_generated_callback(
        raytracer_reflections_generated::callback_type callback) {
    if (!is_running_) {
        return raytracer_reflections_generated_.connect(std::move(callback));
    }
    return {};
}

complete_engine::encountered_error::connection
complete_engine::add_encountered_error_callback(
        encountered_error::callback_type callback) {
    if (!is_running_) {
        return encountered_error_.connect(std::move(callback));
    }
    return {};
}

complete_engine::finished::connection complete_engine::add_finished_callback(
        finished::callback_type callback) {
    if (!is_running_) {
        return finished_.connect(std::move(callback));
    }
    return {};
}

}  // namespace combined
}  // namespace wayverb
