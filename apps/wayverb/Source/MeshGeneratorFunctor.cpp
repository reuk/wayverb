#include "MeshGeneratorFunctor.hpp"

#include "common/spatial_division/voxelised_scene_data.h"

MeshGeneratorFunctor::MeshGeneratorFunctor(
        Listener& listener,
        const copyable_scene_data& scene_data,
        double sample_rate,
        double speed_of_sound)
        : listener(listener)
        , persistent(persistent)
        , scene_data(scene_data)
        , sample_rate(sample_rate)
        , speed_of_sound(speed_of_sound) {}

void MeshGeneratorFunctor::operator()() const {
    const auto cc{compute_context{}};
    auto pair{waveguide::mesh::compute_voxels_and_model(cc.get_context(),
                                                        cc.get_device(),
                                                        scene_data,
                                                        glm::vec3{},
                                                        sample_rate,
                                                        speed_of_sound)};
    listener.mesh_generator_finished(std::move(std::get<1>(pair)));
}

//----------------------------------------------------------------------------//

MeshGeneratorThread::MeshGeneratorThread(
        MeshGeneratorFunctor::Listener& listener,
        const copyable_scene_data& scene_data,
        double sample_rate,
        double speed_of_sound)
        : thread(MeshGeneratorFunctor(
                  listener, scene_data, sample_rate, speed_of_sound)) {}

//----------------------------------------------------------------------------//

void AsyncMeshGenerator::run(const copyable_scene_data& scene_data,
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
        waveguide::mesh::model model) {
    work_queue.push([ this, m = std::move(model) ] {
        listener_list.call(
                &AsyncMeshGenerator::Listener::async_mesh_generator_finished,
                &mesh_generator,
                std::move(m));
        thread = nullptr;
    });
}

void AsyncMeshGenerator::ConcreteListener::run(
        const copyable_scene_data& scene_data,
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
