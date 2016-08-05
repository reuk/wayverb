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
    thread = std::make_unique<ScopedEngineThread>(
            *this,
            file_name.getFullPathName().toStdString(),
            wrapper,
            scene_data,
            visualise);
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
    std::lock_guard<std::mutex> lck(mut);
    work_queue.push([=] {
        listener_list.call(&Listener::engine_encountered_error, this, str);
    });
}

void AsyncEngine::engine_state_changed(wayverb::state state, double progress) {
    std::lock_guard<std::mutex> lck(mut);
    work_queue.push([=] {
        listener_list.call(
                &Listener::engine_state_changed, this, state, progress);
    });
}

void AsyncEngine::engine_nodes_changed(
        const aligned::vector<glm::vec3>& positions) {
    std::lock_guard<std::mutex> lck(mut);
    work_queue.push([=] {
        listener_list.call(&Listener::engine_nodes_changed, this, positions);
    });
}

void AsyncEngine::engine_waveguide_visuals_changed(
        const aligned::vector<float>& pressures) {
    std::lock_guard<std::mutex> lck(mut);
    work_queue.push([=] {
        listener_list.call(
                &Listener::engine_waveguide_visuals_changed, this, pressures);
    });
}

void AsyncEngine::engine_raytracer_visuals_changed(
        const aligned::vector<aligned::vector<raytracer::impulse>>& impulses) {
    std::lock_guard<std::mutex> lck(mut);
    work_queue.push([=] {
        listener_list.call(
                &Listener::engine_raytracer_visuals_changed, this, impulses);
    });
}

void AsyncEngine::engine_finished() {
    std::lock_guard<std::mutex> lck(mut);
    work_queue.push([=] {
        thread = nullptr;
        listener_list.call(&Listener::engine_finished, this);
    });
}

void AsyncEngine::addListener(Listener* l) {
    std::lock_guard<std::mutex> lck(mut);
    listener_list.add(l);
}

void AsyncEngine::removeListener(Listener* l) {
    std::lock_guard<std::mutex> lck(mut);
    listener_list.remove(l);
}