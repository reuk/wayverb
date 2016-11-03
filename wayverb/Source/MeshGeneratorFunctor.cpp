#include "MeshGeneratorFunctor.hpp"

#include "core/spatial_division/voxelised_scene_data.h"

MeshGeneratorFunctor::MeshGeneratorFunctor(Listener& listener,
                                           const scene_data& scene,
                                           double sample_rate,
                                           double speed_of_sound)
        : listener_(listener)
        , scene_data_(scene)
        , sample_rate_(sample_rate)
        , speed_of_sound_(speed_of_sound) {}

void MeshGeneratorFunctor::operator()() const {
    const compute_context cc{};
    auto pair{waveguide::compute_voxels_and_mesh(
            cc, scene_data_, glm::vec3{}, sample_rate_, speed_of_sound_)};
    listener_.mesh_generator_finished(std::move(std::get<1>(pair)));
}

//----------------------------------------------------------------------------//

MeshGeneratorThread::MeshGeneratorThread(
        MeshGeneratorFunctor::Listener& listener,
        const scene_data& scene_data,
        double sample_rate,
        double speed_of_sound)
        : thread(std::thread{MeshGeneratorFunctor(
                  listener, scene_data, sample_rate, speed_of_sound)}) {}

//----------------------------------------------------------------------------//

void AsyncMeshGenerator::run(const scene_data& scene_data,
                             double sample_rate,
                             double speed_of_sound) {
    concrete_listener.run(scene_data, sample_rate, speed_of_sound);
}

void AsyncMeshGenerator::addListener(Listener* l) {
    concrete_listener.addListener(l);
}

void AsyncMeshGenerator::removeListener(Listener* l) {
    concrete_listener.removeListener(l);
}

//----------------------------------------------------------------------------//

AsyncMeshGenerator::ConcreteListener::ConcreteListener(
        AsyncMeshGenerator& mesh_generator)
        : mesh_generator(mesh_generator) {}

void AsyncMeshGenerator::ConcreteListener::mesh_generator_finished(
        waveguide::mesh model) {
    work_queue.push([ this, m = std::move(model) ] {
        listener_list.call(
                &AsyncMeshGenerator::Listener::async_mesh_generator_finished,
                &mesh_generator,
                std::move(m));
        thread = nullptr;
    });
}

void AsyncMeshGenerator::ConcreteListener::run(const scene_data& scene_data,
                                               double sample_rate,
                                               double speed_of_sound) {
    thread = std::make_unique<MeshGeneratorThread>(
            *this, scene_data, sample_rate, speed_of_sound);
}

void AsyncMeshGenerator::ConcreteListener::addListener(
        AsyncMeshGenerator::Listener* l) {
    listener_list.add(l);
}

void AsyncMeshGenerator::ConcreteListener::removeListener(
        AsyncMeshGenerator::Listener* l) {
    listener_list.remove(l);
}
