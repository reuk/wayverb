#pragma once

#include "UtilityComponents/async_work_queue.h"

#include "utilities/scoped_thread.h"
#include "utilities/event.h"

#include "combined/engine.h"
#include "waveguide/mesh.h"

class MeshGeneratorFunctor final {
public:
    MeshGeneratorFunctor(wayverb::combined::engine::scene_data scene_data,
                         double sample_rate,
                         double speed_of_sound);

    void operator()() const;

    template <typename T>
    auto add_event_finished_callback(T&& t) {
        const auto lck = threading_policy_.get_lock();
        return event_finished_.add_scoped(std::forward<T>(t));
    }

private:
    util::threading_policy::scoped_lock threading_policy_;
    
    wayverb::combined::engine::scene_data scene_data_;
    double sample_rate_;
    double speed_of_sound_;

    util::event<wayverb::waveguide::mesh> event_finished_;
};

////////////////////////////////////////////////////////////////////////////////

class AsyncMeshGenerator final {
public:
    void run(wayverb::combined::engine::scene_data scene_data,
             double sample_rate,
             double speed_of_sound);

    template <typename T>
    auto add_event_finished_callback(T&& t) {
        return event_finished_.add_scoped(std::forward<T>(t));
    }

private:
    async_work_queue<util::threading_policy::scoped_lock> work_queue_;
    util::event<wayverb::waveguide::mesh> event_finished_;
    util::event<wayverb::waveguide::mesh>::scoped_connector thread_connector_;
    util::scoped_thread thread_;
};
