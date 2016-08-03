#include "EngineThread.hpp"

#include "combined/engine.h"

#include "UtilityComponents/RAIIThread.hpp"

//  state callback
// auto callback = [&wrapper](auto state, auto progress) {
//    wrapper.render_state.state.set(state);
//    wrapper.render_state.progress.set(progress);
//};

/// Manages the lifetime of a single engine invocation.
/// Construct to start, destruct to stop.
class AsyncEngine::SingleShotEngineThread {
    std::atomic_bool keep_going;
    RAIIThread thread;

    auto single_pair(AsyncEngine& listener,
                     const File& file_name,
                     const model::SingleShot& single_shot,
                     const CopyableSceneData& scene_data,
                     bool visualise,
                     const compute_context& compute_context) {
        try {
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
                engine.register_waveguide_visual_callback([&](const auto& i) {
                    listener.engine_visuals_changed(i);
                });
            }

            //  now run the simulation proper
            auto intermediate = engine.run(keep_going, state_callback);
            //  TODO get output sr from dialog
            intermediate->attenuate(compute_context,
                                    single_shot.receiver_settings,
                                    44100,
                                    state_callback);
            //  TODO write out

            //  Launch viewer window or whatever

            //  if anything goes wrong, flag it up on stdout and
            //  quit the thread
        } catch (const std::runtime_error& e) {
            if (keep_going == false) {
                throw;
            }
            listener.engine_encountered_error(e.what());
        } catch (...) {
            //  if the error was caused by the keep_going flag being flipped
            //  then just propagate the error
            if (keep_going == false) {
                throw;
            }
            //  otherwise trigger a warning
            listener.engine_encountered_error("unknown processing error");
        }
    }

public:
    SingleShotEngineThread(AsyncEngine& listener,
                           const File& file_name,
                           const model::Persistent& persistent,
                           const CopyableSceneData& scene_data,
                           bool visualise)
            : keep_going(true)
            , thread([this,
                      &listener,
                      file_name,
                      persistent,
                      scene_data,
                      visualise]() mutable {
                compute_context compute_context;
                try {
                    //  for each source/receiver pair
                    for (const auto& single_shot :
                         persistent.app.get_all_input_output_combinations()) {
                        //  run the simulation
                        single_pair(listener,
                                    file_name,
                                    single_shot,
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
            }) {}

    virtual ~SingleShotEngineThread() noexcept { keep_going = false; }
};

//----------------------------------------------------------------------------//

AsyncEngine::AsyncEngine()           = default;
AsyncEngine::~AsyncEngine() noexcept = default;

void AsyncEngine::start(const File& file_name,
                        const model::Persistent& wrapper,
                        const CopyableSceneData& scene_data,
                        bool visualise) {
    std::lock_guard<std::mutex> lck(mut);
    thread = std::make_unique<SingleShotEngineThread>(
            *this, file_name, wrapper, scene_data, visualise);
}

void AsyncEngine::stop() {
    std::lock_guard<std::mutex> lck(mut);
    thread = nullptr;
}

bool AsyncEngine::is_running() const {
    std::lock_guard<std::mutex> lck(mut);
    return thread != nullptr;
}

void AsyncEngine::engine_encountered_error(const std::string& str) {
    listener_list.call(&Listener::engine_encountered_error, this, str);
}

void AsyncEngine::engine_state_changed(wayverb::state state, double progress) {
    listener_list.call(&Listener::engine_state_changed, this, state, progress);
}

void AsyncEngine::engine_nodes_changed(
        const aligned::vector<glm::vec3>& positions) {
    listener_list.call(&Listener::engine_nodes_changed, this, positions);
}

void AsyncEngine::engine_visuals_changed(
        const aligned::vector<float>& pressures) {
    listener_list.call(&Listener::engine_visuals_changed, this, pressures);
}

void AsyncEngine::engine_finished() { triggerAsyncUpdate(); }

void AsyncEngine::addListener(Listener* l) {
    std::lock_guard<std::mutex> lck(mut);
    listener_list.add(l);
}

void AsyncEngine::removeListener(Listener* l) {
    std::lock_guard<std::mutex> lck(mut);
    listener_list.remove(l);
}

void AsyncEngine::handleAsyncUpdate() {
    listener_list.call(&Listener::engine_finished, this);
    stop();
}