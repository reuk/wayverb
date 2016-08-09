#include "EngineFunctor.hpp"

EngineFunctor::EngineFunctor(Listener& listener,
                             std::atomic_bool& keep_going,
                             const std::string& file_name,
                             const model::Persistent& persistent,
                             const copyable_scene_data& scene_data,
                             bool visualise)
        : listener(listener)
        , keep_going(keep_going)
        , file_name(file_name)
        , persistent(persistent)
        , scene_data(scene_data)
        , visualise(visualise) {}

void EngineFunctor::operator()() const {
    compute_context compute_context;
    try {
        //  for each source/receiver pair
        const auto combinations =
                persistent.app.get_all_input_output_combinations();
        for (auto i = combinations.cbegin();
             i != combinations.cend() && keep_going;
             ++i) {
            //  run the simulation
            single_pair(listener,
                        file_name,
                        *i,
                        scene_data,
                        visualise,
                        compute_context);
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
                                const copyable_scene_data& scene_data,
                                bool visualise,
                                const compute_context& compute_context) const {
    auto state_callback = [this, &listener](auto state, auto progress) {
        listener.engine_state_changed(state, progress);
    };

    // init the engine
    state_callback(wayverb::state::initialising, 1.0);

    wayverb::engine engine(compute_context,
                           scene_data,
                           single_shot.source,
                           single_shot.receiver_settings.position,
                           single_shot.get_waveguide_sample_rate(),
                           single_shot.rays);

    //  register a visuliser callback if we want to render the waveguide
    //  state
    if (visualise) {
        listener.engine_nodes_changed(engine.get_node_positions());
        engine.register_waveguide_visual_callback([&](const auto& pressures,
                                                      auto current_time) {
            listener.engine_waveguide_visuals_changed(pressures, current_time);
        });
        engine.register_raytracer_visual_callback([&](const auto& impulses,
                                                      const auto& source,
                                                      const auto& receiver) {
            listener.engine_raytracer_visuals_changed(
                    aligned::vector<aligned::vector<raytracer::impulse>>(
                            impulses.begin(),
                            impulses.begin() +
                                    std::min(size_t{100}, impulses.size())),
                    source,
                    receiver);
        });
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

    //  TODO get output sr from dialog
    intermediate->attenuate(compute_context,
                            single_shot.receiver_settings,
                            44100,  //  TODO make this user-changeable
                            state_callback);
    //  TODO write out

    //  Launch viewer window or whatever

    //  if anything goes wrong, flag it up on stdout and
    //  quit the thread
}