#include "EngineThread.hpp"

EngineThread::EngineThread(EngineFunctor::Listener& listener,
                           std::atomic_bool& keep_going,
                           const std::string& file_name,
                           const model::Persistent& wrapper,
                           const copyable_scene_data& scene_data,
                           bool visualise)
        : thread(EngineFunctor(listener,
                               keep_going,
                               file_name,
                               wrapper,
                               scene_data,
                               visualise)) {}

//----------------------------------------------------------------------------//

ScopedEngineThread::ScopedEngineThread(EngineFunctor::Listener& listener,
                                       const std::string& file_name,
                                       const model::Persistent& wrapper,
                                       const copyable_scene_data& scene_data,
                                       bool visualise)
        : thread(listener,
                 keep_going,
                 file_name,
                 wrapper,
                 scene_data,
                 visualise) {}

ScopedEngineThread::~ScopedEngineThread() noexcept { keep_going = false; }

//----------------------------------------------------------------------------//

void AsyncEngine::start(const File& file_name,
                        const model::Persistent& wrapper,
                        const copyable_scene_data& scene_data,
                        bool visualise) {
    std::lock_guard<std::mutex> lck(mut);
    concrete_listener.start(file_name, wrapper, scene_data, visualise);
}

void AsyncEngine::stop() {
    std::lock_guard<std::mutex> lck(mut);
    concrete_listener.stop();
}

bool AsyncEngine::is_running() const {
    std::lock_guard<std::mutex> lck(mut);
    return concrete_listener.is_running();
}

void AsyncEngine::addListener(Listener* l) {
    std::lock_guard<std::mutex> lck(mut);
    concrete_listener.addListener(l);
}

void AsyncEngine::removeListener(Listener* l) {
    std::lock_guard<std::mutex> lck(mut);
    concrete_listener.removeListener(l);
}

//----------------------------------------------------------------------------//

AsyncEngine::ConcreteListener::ConcreteListener(AsyncEngine& engine)
        : engine(engine) {}

void AsyncEngine::ConcreteListener::engine_encountered_error(
        const std::string& str) {
    work_queue.push([=] {
        listener_list.call(
                &AsyncEngine::Listener::engine_encountered_error, &engine, str);
    });
}

void AsyncEngine::ConcreteListener::engine_state_changed(wayverb::state state,
                                                         double progress) {
    work_queue.push([=] {
        listener_list.call(&AsyncEngine::Listener::engine_state_changed,
                           &engine,
                           state,
                           progress);
    });
}

void AsyncEngine::ConcreteListener::engine_nodes_changed(
        const aligned::vector<glm::vec3>& positions) {
    work_queue.push([=] {
        listener_list.call(&AsyncEngine::Listener::engine_nodes_changed,
                           &engine,
                           positions);
    });
}

void AsyncEngine::ConcreteListener::engine_waveguide_visuals_changed(
        const aligned::vector<float>& pressures, double current_time) {
    work_queue.push([=] {
        listener_list.call(
                &AsyncEngine::Listener::engine_waveguide_visuals_changed,
                &engine,
                pressures,
                current_time);
    });
}

void AsyncEngine::ConcreteListener::engine_raytracer_visuals_changed(
        const aligned::vector<aligned::vector<impulse>>& impulses,
        const glm::vec3& source,
        const glm::vec3& receiver) {
    work_queue.push([=] {
        listener_list.call(
                &AsyncEngine::Listener::engine_raytracer_visuals_changed,
                &engine,
                impulses,
                source,
                receiver);
    });
}

void AsyncEngine::ConcreteListener::engine_finished() {
    work_queue.push([=] {
        thread = nullptr;
        listener_list.call(&AsyncEngine::Listener::engine_finished, &engine);
    });
}

void AsyncEngine::ConcreteListener::start(const File& file_name,
                                          const model::Persistent& wrapper,
                                          const copyable_scene_data& scene_data,
                                          bool visualise) {
    thread = std::make_unique<ScopedEngineThread>(
            *this,
            file_name.getFullPathName().toStdString(),
            wrapper,
            scene_data,
            visualise);
}

void AsyncEngine::ConcreteListener::stop() { thread = nullptr; }

bool AsyncEngine::ConcreteListener::is_running() const {
    return thread != nullptr;
}

void AsyncEngine::ConcreteListener::addListener(AsyncEngine::Listener* l) {
    listener_list.add(l);
}

void AsyncEngine::ConcreteListener::removeListener(AsyncEngine::Listener* l) {
    listener_list.remove(l);
}