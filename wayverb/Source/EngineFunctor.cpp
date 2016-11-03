#include "EngineFunctor.hpp"

#include "core/cl/common.h"

EngineFunctor::EngineFunctor(Listener& listener,
                             std::atomic_bool& keep_going,
                              std::string file_name,
                             model::Persistent persistent,
                              wayverb::combined::engine::scene_data scene_data,
                             bool visualise)
        : listener(listener)
        , keep_going(keep_going)
        , file_name(std::move(file_name))
        , persistent(std::move(persistent))
        , scene(std::move(scene_data))
        , visualise(visualise) {}

void EngineFunctor::operator()() const {
    wayverb::core::compute_context compute_context;
    try {
        //  for each source/receiver pair
        const auto combinations =
                get_all_input_output_combinations(persistent.app);
        for (auto i = combinations.cbegin();
             i != combinations.cend() && keep_going;
             ++i) {
            //  run the simulation
            single_pair(
                    listener, file_name, *i, scene, visualise, compute_context);
        }
    } catch (const std::runtime_error& e) {
        listener.engine_encountered_error(e.what());
    } catch (...) {
        listener.engine_encountered_error("unknown error");
    }
    listener.engine_finished();
}

void EngineFunctor::single_pair(Listener& listener,
                                const std::string& file_name,
                                const model::SingleShot& single_shot,
                                const wayverb::combined::engine::scene_data& scene_data,
                                bool visualise,
                                const wayverb::core::compute_context& compute_context) const {
    auto state_callback = [this, &listener](auto state, auto progress) {
        listener.engine_state_changed(state, progress);
    };

    // init the engine
    state_callback(wayverb::combined::state::initialising, 1.0);

    //  TODO set parameters properly here
    wayverb::combined::engine engine(compute_context,
                           scene_data,
                           wayverb::core::model::parameters{single_shot.source,
                           single_shot.receiver.position},
                           wayverb::raytracer::simulation_parameters{},
                           wayverb::waveguide::single_band_parameters{});

    //  register a visuliser callback if we want to render the waveguide
    //  state
    if (visualise) {
        //  TODO register callbacks
        /*
        listener.engine_nodes_changed(engine.get_node_positions());
        engine.register_waveguide_visual_callback([&](const auto& pressures,
                                                      auto current_time) {
            listener.engine_waveguide_visuals_changed(pressures, current_time);
        });
        engine.register_raytracer_visual_callback([&](const auto& impulses,
                                                      const auto& source,
                                                      const auto& receiver) {
            listener.engine_raytracer_visuals_changed(
                    util::aligned::vector<util::aligned::vector<impulse>>(
                            impulses.begin(),
                            impulses.begin() +
                                    std::min(size_t{100}, impulses.size())),
                    source,
                    receiver);
        });
        */
    }

    //  now run the simulation proper
    const auto intermediate = engine.run(keep_going, state_callback);
    if (!keep_going) {
        //  user asked to stop
        return;
    }

    //  we should keep going, check whether results are valid
    if (!intermediate) {
        throw std::runtime_error("failed to generate intermediate results");
    }

    //  TODO postprocess properly
    for (const auto& capsule : single_shot.receiver.capsules) {
        //  TODO get output sr from dialog
        capsule->postprocess(*intermediate, 44100);
    }

    //  TODO write out

    //  Launch viewer window or whatever

    //  if anything goes wrong, flag it up on stdout and
    //  quit the thread
}
