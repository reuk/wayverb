#pragma once

#include "FullModel.hpp"

class AsyncEngine : private AsyncUpdater {
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
                                          wayverb::state state,
                                          double progress) = 0;
        virtual void engine_nodes_changed(
                AsyncEngine*, const aligned::vector<glm::vec3>& positions) = 0;
        virtual void engine_visuals_changed(
                AsyncEngine*, const aligned::vector<float>& pressures) = 0;
        virtual void engine_finished(AsyncEngine*) = 0;
    };

    AsyncEngine();
    virtual ~AsyncEngine() noexcept;

    void start(const File& file_name,
               const model::Persistent& wrapper,
               const copyable_scene_data& scene_data,
               bool visualise);
    void stop();

    bool is_running() const;

    void addListener(Listener* l);
    void removeListener(Listener* l);

private:
    void handleAsyncUpdate() override;

    mutable std::mutex mut;
    class SingleShotEngineThread;
    std::unique_ptr<SingleShotEngineThread> thread;
    ListenerList<Listener> listener_list;

    void engine_encountered_error(const std::string& str);
    void engine_state_changed(wayverb::state state, double progress);
    void engine_nodes_changed(const aligned::vector<glm::vec3>& positions);
    void engine_visuals_changed(const aligned::vector<float>& pressures);
    void engine_finished();
};