#include "MeshGeneratorFunctor.hpp"

#include "core/spatial_division/voxelised_scene_data.h"

MeshGeneratorFunctor::MeshGeneratorFunctor(wayverb::core::gpu_scene_data scene,
                                           double sample_rate,
                                           double speed_of_sound)
        : scene_data_{std::move(scene)}
        , sample_rate_{sample_rate}
        , speed_of_sound_{speed_of_sound} {}

void MeshGeneratorFunctor::operator()() const {
    const auto lck = threading_policy_.get_lock();
    auto pair = wayverb::waveguide::compute_voxels_and_mesh(
            wayverb::core::compute_context{}, scene_data_, glm::vec3{}, sample_rate_, speed_of_sound_);
    event_finished_(std::move(pair.mesh));
}

////////////////////////////////////////////////////////////////////////////////

void AsyncMeshGenerator::run(wayverb::core::gpu_scene_data scene_data,
                             double sample_rate,
                             double speed_of_sound) {
    MeshGeneratorFunctor functor{std::move(scene_data), sample_rate, speed_of_sound};

    //  When the thread finishes, call the notifier from the normal event thread.
    thread_connector_ = functor.add_event_finished_callback([this] (auto mesh) {
        work_queue_.push([this, mesh = std::move(mesh)] {
            event_finished_(std::move(mesh));
        });
    });
    thread_ = util::scoped_thread{std::thread{std::move(functor)}};
}
