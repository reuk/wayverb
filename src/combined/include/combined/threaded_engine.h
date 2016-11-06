#pragma once

#include "combined/full_run.h"

#include "audio_file/audio_file.h"

namespace wayverb {
namespace combined {

struct source_info final {
    std::string name;
    glm::vec3 position;
};

struct capsule_info final {
    std::string name;
    std::unique_ptr<capsule_base> capsule;
};

struct receiver_info final {
    std::string name;
    glm::vec3 position;
    std::vector<capsule_info> capsules;
};

struct output_info final {
    std::string output_folder;
    std::string name;
    double sample_rate;
    int bit_depth;
};

struct channel_info final {
    util::aligned::vector<float> data;
    std::string source_name;
    std::string receiver_name;
    std::string capsule_name;
};

struct max_mag_functor final {
    template <typename T>
    auto operator()(const T& t) const {
        return core::max_mag(t.data);
    }
};

using waveguide_node_positions_changed =
        util::event<waveguide::mesh_descriptor>;

/// Given a scene, and a collection of sources and receivers,
/// For each source-receiver pair:
///     Simulate the scene.
///     Do microphone post-processing according to the receiver's capsules.
///     Cache the results.
/// Once all outputs have been calculated:
///     Do global normalization.
///     Write files out.
/// If the engine encounters an error:
///     Signal using a callback, and quit.
/// If the user cancels early:
///     Signal using a callback, and quit.

class complete_engine final {
public:
    template <typename WaveguideParameters>
    void run(const core::compute_context& compute_context,
             const core::gpu_scene_data& scene_data,
             const std::vector<source_info>& sources,
             const std::vector<receiver_info>& receivers,
             const core::environment& environment,
             const raytracer::simulation_parameters& raytracer,
             const WaveguideParameters& waveguide,
             const output_info& output,
             const std::atomic_bool& keep_going) const {
        try {
            if (sources.empty()) {
                throw std::runtime_error{"no sources specified"};
            }

            if (receivers.empty()) {
                throw std::runtime_error{"no receivers specified"};
            }

            std::vector<channel_info> all_channels;

            //  For each source-receiver pair.
            for (auto source = begin(sources), e_source = end(sources);
                 source != e_source && keep_going;
                 ++source) {
                for (auto receiver = begin(receivers),
                          e_receiver = end(receivers);
                     receiver != e_receiver && keep_going;
                     ++receiver) {
                    //  Set up an engine to use.
                    auto eng =
                            make_postprocessing_engine_ptr(compute_context,
                                                           scene_data,
                                                           source->position,
                                                           receiver->position,
                                                           environment,
                                                           raytracer,
                                                           waveguide);

                    //  Send new node position notification.
                    waveguide_node_positions_changed_(
                            eng->get_voxels_and_mesh().mesh.get_descriptor());

                    //  Register callbacks.
                    const auto engine_state_change_connector =
                            eng->add_scoped_engine_state_changed_callback(
                                    make_forwarding_call(
                                            engine_state_changed_));

                    const auto node_pressure_connector =
                            eng->add_scoped_waveguide_node_pressures_changed_callback(
                                    make_forwarding_call(
                                            waveguide_node_pressures_changed_));

                    const auto raytracer_reflection_connector =
                            eng->add_scoped_raytracer_reflections_generated_callback(
                                    make_forwarding_call(
                                            raytracer_reflections_generated_));

                    const auto make_iterator = [](auto it) {
                        return util::make_mapping_iterator_adapter(
                                std::move(it),
                                [](const auto& i) -> const std::unique_ptr<
                                        capsule_base>& { return i.capsule; });
                    };

                    //  Run the simulation, cache the result.
                    auto channel =
                            eng->run(make_iterator(begin(receiver->capsules)),
                                     make_iterator(end(receiver->capsules)),
                                     output.sample_rate,
                                     keep_going);

                    if (!keep_going) {
                        throw std::runtime_error{"simulation cancelled"};
                    }

                    if (!channel) {
                        throw std::runtime_error{
                                "encountered unanticipated error"};
                    }

                    for (size_t i = 0, e = receiver->capsules.size(); i != e;
                         ++i) {
                        all_channels.emplace_back(
                                channel_info{std::move((*channel)[i]),
                                             source->name,
                                             receiver->name,
                                             receiver->capsules[i].name});
                    }
                }
            }

            //  If keep going is false now, then the simulation was cancelled.
            if (!keep_going) {
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
                const auto file_name = util::build_string(output.output_folder,
                                                          '/',
                                                          output.name,
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
                      audio_file::make_audio_file(i.data, output.sample_rate),
                      output.bit_depth);
            }

        } catch (const std::exception& e) {
            encountered_error_(e.what());
            return;
        }
    }

    engine_state_changed::scoped_connector
            add_scoped_engine_state_changed_callback(
                    engine_state_changed::callback_type);

    waveguide_node_positions_changed::scoped_connector
            add_scoped_waveguide_node_positions_changed_callback(
                    waveguide_node_positions_changed::callback_type);

    waveguide_node_pressures_changed::scoped_connector
            add_scoped_waveguide_node_pressures_changed_callback(
                    waveguide_node_pressures_changed::callback_type);

    raytracer_reflections_generated::scoped_connector
            add_scoped_raytracer_reflections_generated_callback(
                    raytracer_reflections_generated::callback_type);

    using encountered_error = util::event<std::string>;

    encountered_error::scoped_connector add_scoped_encountered_error_callback(
            encountered_error::callback_type);

private:
    engine_state_changed engine_state_changed_;
    waveguide_node_positions_changed waveguide_node_positions_changed_;
    waveguide_node_pressures_changed waveguide_node_pressures_changed_;
    raytracer_reflections_generated raytracer_reflections_generated_;
    encountered_error encountered_error_;
};

}  // namespace combined
}  // namespace wayverb
