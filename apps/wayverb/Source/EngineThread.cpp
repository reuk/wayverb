#include "EngineThread.hpp"

#include "combined/engine.h"

#include "UtilityComponents/RAIIThread.hpp"

//  state callback
// auto callback = [&wrapper](auto state, auto progress) {
//    wrapper.render_state.state.set(state);
//    wrapper.render_state.progress.set(progress);
//};

namespace {
float max_reflectivity(const VolumeType& vt) {
    return *proc::max_element(vt.s);
}

float max_reflectivity(const Surface& surface) {
    return std::max(max_reflectivity(surface.diffuse),
                    max_reflectivity(surface.specular));
}

float max_reflectivity(const SceneData::Material& material) {
    return max_reflectivity(material.surface);
}

float max_reflectivity(const std::vector<SceneData::Material>& materials) {
    return std::accumulate(materials.begin() + 1,
                           materials.end(),
                           max_reflectivity(materials.front()),
                           [](const auto& i, const auto& j) {
                               return std::max(i, max_reflectivity(j));
                           });
}
}  // namespace

/// Manages the lifetime of a single engine invocation.
/// Construct to start, destruct to stop.
class AsyncEngine::SingleShotEngineThread {
    std::atomic_bool keep_going;
    RAIIThread thread;

    using Engine = engine::WayverbEngine<BufferType::cl>;

    auto single_pair(AsyncEngine& listener,
                     const File& file_name,
                     const model::SingleShot& single_shot,
                     const CopyableSceneData& scene_data,
                     bool visualise,
                     ComputeContext& compute_context,
                     size_t impulses) {
        try {
            auto state_callback = [this, &listener](auto state, auto progress) {
                listener.engine_state_changed(state, progress);
            };

            // init the engine
            state_callback(engine::State::initialising, 1.0);
            Engine engine(compute_context,
                          scene_data,
                          single_shot.source,
                          single_shot.receiver_settings.position,
                          single_shot.get_waveguide_sample_rate(),
                          single_shot.rays,
                          impulses,
                          //    TODO get samplerate from dialog?
                          44100);

            //  check that source and mic are inside model
            auto check_position = [](auto valid, const std::string& str) {
                if (!valid) {
                    NativeMessageBox::showMessageBoxAsync(
                            AlertWindow::AlertIconType::WarningIcon,
                            str + " position is invalid",
                            "It looks like that " + str +
                                    " position is outside the waveguide mesh. "
                                    "Make sure the 3D model is completely "
                                    "closed, and the " +
                                    str + " is inside.");
                    throw std::runtime_error(str + " is outside mesh");
                }
            };

            check_position(engine.get_mic_position_is_valid(), "microphone");
            check_position(engine.get_source_position_is_valid(), "source");

            //  now run the simulation proper
            auto run = [this, &engine, &state_callback] {
                return engine.run(keep_going, state_callback);
            };

            auto run_visualised = [this, &listener, &engine, &state_callback] {
                listener.engine_nodes_changed(engine.get_node_positions());
                return engine.run_visualised(
                        keep_going,
                        state_callback,
                        [this, &listener](const auto& i) {
                            listener.engine_visuals_changed(i);
                        });
            };
            auto intermediate = visualise ? run_visualised() : run();
            engine.attenuate(intermediate, state_callback);
            //  TODO write out

            //  Launch viewer window or whatever

            //  if anything goes wrong, flag it up on stdout and
            //  quit
            //  the thread
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
                ComputeContext compute_context;
                try {
                    //  compute ideal number of impulses
                    auto impulses = compute_optimum_reflection_number(
                            Decibels::decibelsToGain(-48.0),
                            max_reflectivity(persistent.materials));
                    std::cerr << "impulses estimated: " << impulses
                              << std::endl;

                    //  for each source/receiver pair
                    for (const auto& single_shot :
                         persistent.app.get_all_input_output_combinations()) {
                        //  run the simulation
                        single_pair(listener,
                                    file_name,
                                    single_shot,
                                    scene_data,
                                    visualise,
                                    compute_context,
                                    impulses);
                    }
                } catch (const std::runtime_error& e) {
                    listener.engine_encountered_error(e.what());
                } catch (...) {
                    listener.engine_encountered_error("unknown error");
                }
                listener.engine_finished();
            }) {
    }

    virtual ~SingleShotEngineThread() noexcept {
        keep_going = false;
    }
};

//----------------------------------------------------------------------------//

AsyncEngine::AsyncEngine() = default;
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

void AsyncEngine::engine_state_changed(engine::State state, double progress) {
    listener_list.call(&Listener::engine_state_changed, this, state, progress);
}

void AsyncEngine::engine_nodes_changed(
        const std::vector<cl_float3>& positions) {
    listener_list.call(&Listener::engine_nodes_changed, this, positions);
}

void AsyncEngine::engine_visuals_changed(const std::vector<float>& pressures) {
    listener_list.call(&Listener::engine_visuals_changed, this, pressures);
}

void AsyncEngine::engine_finished() {
    triggerAsyncUpdate();
}

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