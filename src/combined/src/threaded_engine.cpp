#include "combined/threaded_engine.h"
#include "combined/forwarding_call.h"
#include "combined/validate_placements.h"
#include "combined/waveguide_base.h"

#include "waveguide/config.h"

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
    std::string file_name;
};

}  // namespace

std::unique_ptr<capsule_base> polymorphic_capsule_model(
        const model::capsule& i, const core::orientation& orientation) {
    switch (i.get_mode()) {
        case model::capsule::mode::microphone:
            return make_capsule_ptr(i.microphone().item()->get(), orientation);
        case model::capsule::mode::hrtf:
            return make_capsule_ptr(i.hrtf().item()->get(), orientation);
    }
}

std::unique_ptr<waveguide_base> polymorphic_waveguide_model(
        const model::waveguide& i) {
    switch (i.get_mode()) {
        case model::waveguide::mode::single:
            return make_waveguide_ptr(i.single_band().item()->get());
        case model::waveguide::mode::multiple:
            return make_waveguide_ptr(i.multiple_band().item()->get());
    }
}

////////////////////////////////////////////////////////////////////////////////

complete_engine::~complete_engine() noexcept { cancel(); }

bool complete_engine::is_running() const { return is_running_; }
void complete_engine::cancel() { keep_going_ = false; }

void complete_engine::run(core::compute_context compute_context,
                          core::gpu_scene_data scene_data,
                          model::persistent persistent,
                          model::output output) {
    cancel();

    future_ = std::async(std::launch::async, [
        this,
        compute_context = std::move(compute_context),
        scene_data = std::move(scene_data),
        persistent = std::move(persistent),
        output = std::move(output)
    ] {
        do_run(std::move(compute_context),
               std::move(scene_data),
               std::move(persistent),
               std::move(output));
    });
}

void complete_engine::do_run(core::compute_context compute_context,
                             core::gpu_scene_data scene_data,
                             model::persistent persistent,
                             model::output output) {
    try {
        is_running_ = true;
        keep_going_ = true;

        //  Send the "IT HAS BEGUN" message.
        begun_();

        constexpr core::environment environment{};

        //  First, we check that all the sources and receivers are valid, to
        //  avoid doing useless work.

        const auto make_position_extractor_iterator = [](auto it) {
            return util::make_mapping_iterator_adapter(
                    std::move(it),
                    [](const auto& i) { return i.item()->get_position(); });
        };

        if (!is_pairwise_distance_acceptable(
                    make_position_extractor_iterator(
                            std::begin(*persistent.sources())),
                    make_position_extractor_iterator(
                            std::end(*persistent.sources())),
                    make_position_extractor_iterator(
                            std::begin(*persistent.receivers())),
                    make_position_extractor_iterator(
                            std::end(*persistent.receivers())),
                    waveguide::config::grid_spacing(
                            environment.speed_of_sound,
                            1 / compute_sampling_frequency(
                                        *persistent.waveguide())))) {
            throw std::runtime_error{
                    "Placing sources and receivers too close "
                    "together will produce inaccurate results."};
        }

        {
            //  Check that all sources and receivers are inside the mesh.
            const auto voxelised =
                    core::make_voxelised_scene_data(scene_data, 5, 0.1f);

            if (!are_all_inside(make_position_extractor_iterator(
                                        std::begin(*persistent.sources())),
                                make_position_extractor_iterator(
                                        std::end(*persistent.sources())),
                                voxelised)) {
                throw std::runtime_error{"Source is outside mesh."};
            }

            if (!are_all_inside(make_position_extractor_iterator(
                                        std::begin(*persistent.receivers())),
                                make_position_extractor_iterator(
                                        std::end(*persistent.receivers())),
                                voxelised)) {
                throw std::runtime_error{"Receiver is outside mesh."};
            }
        }

        //  Now we can start rendering.

        const auto poly_waveguide =
                polymorphic_waveguide_model(*persistent.waveguide().item());

        std::vector<channel_info> all_channels;

        const auto runs = persistent.sources().item()->size() *
                          persistent.receivers().item()->size();

        auto run = 0;

        //  For each source-receiver pair.
        for (auto source = std::begin(*persistent.sources().item()),
                  e_source = std::end(*persistent.sources().item());
             source != e_source && keep_going_;
             ++source) {
            for (auto receiver = std::begin(*persistent.receivers().item()),
                      e_receiver = std::end(*persistent.receivers().item());
                 receiver != e_receiver && keep_going_;
                 ++receiver, ++run) {
                //  Set up an engine to use.
                postprocessing_engine eng{compute_context,
                                          scene_data,
                                          source->item()->get_position(),
                                          receiver->item()->get_position(),
                                          environment,
                                          persistent.raytracer().item()->get(),
                                          poly_waveguide->clone()};

                //  Send new node position notification.
                waveguide_node_positions_changed_(
                        eng.get_voxels_and_mesh().mesh.get_descriptor());

                //  Register callbacks.
                if (!engine_state_changed_.empty()) {
                    eng.connect_engine_state_changed([this, runs, run](
                            auto state, auto progress) {
                        engine_state_changed_(run, runs, state, progress);
                    });
                }

                if (!waveguide_node_pressures_changed_.empty()) {
                    eng.connect_waveguide_node_pressures_changed(
                            make_forwarding_call(
                                    waveguide_node_pressures_changed_));
                }

                if (!raytracer_reflections_generated_.empty()) {
                    eng.connect_raytracer_reflections_generated(
                            make_forwarding_call(
                                    raytracer_reflections_generated_));
                }

                const auto polymorphic_capsules = util::map_to_vector(
                        std::begin(*receiver->item()->capsules().item()),
                        std::end(*receiver->item()->capsules().item()),
                        [&](const auto& i) {
                            return polymorphic_capsule_model(
                                    *i.item(),
                                    receiver->item()->get_orientation());
                        });

                //  Run the simulation, cache the result.
                auto channel =
                        eng.run(begin(polymorphic_capsules),
                                end(polymorphic_capsules),
                                get_sample_rate(output.get_sample_rate()),
                                keep_going_);

                //  If user cancelled while processing the channel, channel
                //  will be null, but we want to exit before throwing an
                //  exception.
                if (!keep_going_) {
                    break;
                }

                if (!channel) {
                    throw std::runtime_error{
                            "Encountered unknown error, causing channel not to "
                            "be rendered."};
                }

                for (size_t i = 0,
                            e = receiver->item()->capsules().item()->size();
                     i != e;
                     ++i) {
                    all_channels.emplace_back(channel_info{
                            std::move((*channel)[i]),
                            compute_output_path(
                                    *source->item(),
                                    *receiver->item(),
                                    *(*receiver->item()->capsules().item())[i]
                                             .item(),
                                    output)});
                }
            }
        }

        //  If keep going is false now, then the simulation was cancelled.
        if (keep_going_) {
            if (all_channels.empty()) {
                throw std::runtime_error{"No channels were rendered."};
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
                throw std::runtime_error{"All channels are silent."};
            }

            const auto factor = 1.0 / max_mag;

            for (auto& channel : all_channels) {
                for (auto& sample : channel.data) {
                    sample *= factor;
                }
            }

            //  Write out files.
            for (const auto& i : all_channels) {
                audio_file::write(i.file_name.c_str(),
                                  i.data,
                                  get_sample_rate(output.get_sample_rate()),
                                  output.get_format(),
                                  output.get_bit_depth());
            }
        }

    } catch (const std::exception& e) {
        encountered_error_(e.what());
    }

    is_running_ = false;

    finished_();
}

complete_engine::engine_state_changed::connection
complete_engine::connect_engine_state_changed(
        engine_state_changed::callback_type callback) {
    return engine_state_changed_.connect(std::move(callback));
}

complete_engine::waveguide_node_positions_changed::connection
complete_engine::connect_waveguide_node_positions_changed(
        waveguide_node_positions_changed::callback_type callback) {
    return waveguide_node_positions_changed_.connect(std::move(callback));
}

complete_engine::waveguide_node_pressures_changed::connection
complete_engine::connect_waveguide_node_pressures_changed(
        waveguide_node_pressures_changed::callback_type callback) {
    return waveguide_node_pressures_changed_.connect(std::move(callback));
}

complete_engine::raytracer_reflections_generated::connection
complete_engine::connect_raytracer_reflections_generated(
        raytracer_reflections_generated::callback_type callback) {
    return raytracer_reflections_generated_.connect(std::move(callback));
}

complete_engine::encountered_error::connection
complete_engine::connect_encountered_error(
        encountered_error::callback_type callback) {
    return encountered_error_.connect(std::move(callback));
}

complete_engine::begun::connection complete_engine::connect_begun(
        begun::callback_type callback) {
    return begun_.connect(std::move(callback));
}

complete_engine::finished::connection complete_engine::connect_finished(
        finished::callback_type callback) {
    return finished_.connect(std::move(callback));
}

}  // namespace combined
}  // namespace wayverb
