#pragma once

#include "EngineFunctor.hpp"
#include "WorkQueue.hpp"

#include "UtilityComponents/scoped_thread.hpp"

/// Runs the engine on another thread.
class EngineThread final {
public:
    EngineThread(EngineFunctor::Listener& listener,
                 std::atomic_bool& keep_going,
                 std::string file_name,
                 model::Persistent wrapper,
                 wayverb::combined::engine::scene_data scene_data,
                 bool visualise);

private:
    scoped_thread thread;
};

//----------------------------------------------------------------------------//

/// Runs the engine on another thread, will block and try to quit the thread
/// early if destructed while still running.
class ScopedEngineThread final {
public:
    ScopedEngineThread(EngineFunctor::Listener& listener,
                       std::string file_name,
                       model::Persistent wrapper,
                       wayverb::combined::engine::scene_data scene_data,
                       bool visualise);

    ~ScopedEngineThread() noexcept;

private:
    std::atomic_bool keep_going{true};
    EngineThread thread;
};

//----------------------------------------------------------------------------//

class AsyncEngine final {
public:
    class Listener {
    public:
        Listener() = default;
        Listener(const Listener&) = default;
        Listener& operator=(const Listener&) = default;
        Listener(Listener&&) noexcept = default;
        Listener& operator=(Listener&&) noexcept = default;
        virtual ~Listener() noexcept = default;

        virtual void engine_encountered_error(AsyncEngine*,
                                              const std::string& str) = 0;
        virtual void engine_state_changed(AsyncEngine*,
                                          wayverb::combined::state state,
                                          double progress) = 0;
        virtual void engine_nodes_changed(
                AsyncEngine*, const util::aligned::vector<glm::vec3>& positions) = 0;
        virtual void engine_waveguide_visuals_changed(
                AsyncEngine*,
                const util::aligned::vector<float>& pressures,
                double current_time) = 0;
        virtual void engine_raytracer_visuals_changed(
                AsyncEngine*,
                const util::aligned::vector<util::aligned::vector<wayverb::raytracer::impulse<wayverb::core::simulation_bands>>>& impulses,
                const glm::vec3& sources,
                const glm::vec3& receivers) = 0;
        virtual void engine_finished(AsyncEngine*) = 0;
    };

    void start(const File& file_name,
               model::Persistent wrapper,
               wayverb::combined::engine::scene_data scene_data,
               bool visualise);
    void stop();

    bool is_running() const;

    void addListener(Listener* l);
    void removeListener(Listener* l);

private:
    mutable std::mutex mut;

    /// Extra level of indirection yo
    class ConcreteListener final : public EngineFunctor::Listener {
    public:
        ConcreteListener(AsyncEngine& engine);

        void engine_encountered_error(const std::string& str) override;
        void engine_state_changed(wayverb::combined::state state,
                                  double progress) override;
        void engine_nodes_changed(
                const util::aligned::vector<glm::vec3>& positions) override;
        void engine_waveguide_visuals_changed(
                const util::aligned::vector<float>& pressures,
                double current_time) override;
        void engine_raytracer_visuals_changed(
                const util::aligned::vector<util::aligned::vector<wayverb::raytracer::impulse<wayverb::core::simulation_bands>>>& impulses,
                const glm::vec3& source,
                const glm::vec3& receiver) override;
        void engine_finished() override;

        void start(const File& file_name,
                   model::Persistent wrapper,
                   wayverb::combined::engine::scene_data scene_data,
                   bool visualise);
        void stop();
        bool is_running() const;

        void addListener(AsyncEngine::Listener* l);
        void removeListener(AsyncEngine::Listener* l);

    private:
        AsyncEngine& engine;
        ListenerList<AsyncEngine::Listener> listener_list;
        AsyncWorkQueue work_queue;

        //  IMPORTANT
        //  The thread relies on the rest of this object. It communicates with
        //  the outside world through the work_queue and listener_list.
        //  It MUST be destroyed before these objects so that it doesn't try to
        //  use them (it must be after them in the class declaration b/c object
        //  members are destroyed in reverse-declaration order).
        std::unique_ptr<ScopedEngineThread> thread;
    };

    ConcreteListener concrete_listener{*this};
};
