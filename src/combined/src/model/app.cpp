#include "combined/model/app.h"

#include "core/cl/common.h"
#include "core/serialize/range.h"
#include "core/serialize/surface.h"

#include "cereal/archives/json.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/tuple.hpp"

#include <fstream>

namespace wayverb {
namespace combined {
namespace model {

project::project(const std::string& fpath)
        : scene_data_{is_project_file(fpath) ? compute_model_path(fpath)
                                             : fpath}
        , needs_save_{!is_project_file(fpath)}
        , persistent{core::geo::compute_aabb(
                  scene_data_.get_scene_data().get_vertices())} {
    if (is_project_file(fpath)) {
        const auto config_file = project::compute_config_path(fpath);

        //  load the config
        std::ifstream stream(config_file);
        cereal::JSONInputArchive archive(stream);
        archive(persistent);

    } else {
        const auto& surface_strings =
                scene_data_.get_scene_data().get_surfaces();
        persistent.materials() = materials_from_names<1>(begin(surface_strings),
                                                         end(surface_strings));
    }

    persistent.connect([&](auto&) { needs_save_ = true; });
}

std::string project::compute_model_path(const std::string& root) {
    return root + '/' + model_name;
}

std::string project::compute_config_path(const std::string& root) {
    return root + '/' + config_name;
}

bool project::is_project_file(const std::string& fpath) {
    return std::string{std::find_if(crbegin(fpath),
                                    crend(fpath),
                                    [](auto i) { return i == '.'; })
                               .base(),
                       end(fpath)} == ".way";
}

void project::save_to(const std::string& fpath) {
    if (needs_save_) {
        //  TODO create directory

        //  write current geometry to file
        scene_data_.save(project::compute_model_path(fpath));

        //  write config with all current materials to file
        std::ofstream stream(project::compute_config_path(fpath));
        cereal::JSONOutputArchive archive(stream);
        archive(persistent);

        needs_save_ = false;

        //  TODO register_recent_file(f.getFullPathName().toStdString());
    }
}

bool project::needs_save() const { return needs_save_; }

core::generic_scene_data<cl_float3, std::string> project::get_scene_data()
        const {
    return scene_data_.get_scene_data();
}

////////////////////////////////////////////////////////////////////////////////

app::app(const std::string& name)
        : project{name}
        , currently_open_file_{name} {}

app::~app() noexcept { cancel_render(); }

void app::start_render() {
    cancel_render();

    //  Collect parameters.

    auto scene_data = generate_scene_data();
    auto params = project.persistent;

    //  Start engine in new thread.
    future_ = std::async(
            std::launch::async,
            [ this, s = std::move(scene_data), p = std::move(params) ] {
                engine_.run(
                        core::compute_context{}, std::move(s), std::move(p));
            });
}

void app::cancel_render() { engine_.cancel(); }

//  void app::is_rendering() const { engine_.is_running(); }

bool app::save(const save_callback& callback) {
    if (project::is_project_file(currently_open_file_)) {
        project.save_to(currently_open_file_);
    } else {
        if (const auto fpath = callback()) {
            save_as(*fpath);
        }
    }
}

void app::save_as(std::string name) {
    if (!project::is_project_file(name)) {
        throw std::runtime_error{"save path must have project extension"};
    }
    project.save_to(name);
    currently_open_file_ = std::move(name);
}

bool app::needs_save() const { return project.needs_save(); }

//  DEBUG  /////////////////////////////////////////////////////////////////////

void app::generate_debug_mesh() {
    auto scene_data = generate_scene_data();
    auto sample_rate = project.persistent.waveguide().get_sampling_frequency();
    auto speed_of_sound = 340.0;

    future_ = std::async(
            std::launch::async,
            [ this, s = std::move(scene_data), sample_rate, speed_of_sound ] {
                auto pair = wayverb::waveguide::compute_voxels_and_mesh(
                        core::compute_context{},
                        s,
                        glm::vec3{},
                        sample_rate,
                        speed_of_sound);
                mesh_generated_(std::move(pair.mesh));
            });
}

//  CALLBACKS  /////////////////////////////////////////////////////////////////

engine_state_changed::connection app::connect_engine_state(
        engine_state_changed::callback_type t) {
    return engine_.add_engine_state_changed_callback(std::move(t));
}

waveguide_node_positions_changed::connection app::connect_node_positions(
        waveguide_node_positions_changed::callback_type t) {
    return engine_.add_waveguide_node_positions_changed_callback(std::move(t));
}

waveguide_node_pressures_changed::connection app::connect_node_pressures(
        waveguide_node_pressures_changed::callback_type t) {
    return engine_.add_waveguide_node_pressures_changed_callback(std::move(t));
}

raytracer_reflections_generated::connection app::connect_reflections(
        raytracer_reflections_generated::callback_type t) {
    return engine_.add_raytracer_reflections_generated_callback(std::move(t));
}

app::encountered_error::connection app::connect_error_handler(
        encountered_error::callback_type t) {
    return engine_.add_encountered_error_callback(std::move(t));
}

app::mesh_generated::connection app::connect_mesh_generated(
        mesh_generated::callback_type t) {
    return mesh_generated_.connect(std::move(t));
}

core::gpu_scene_data app::generate_scene_data() {
    util::aligned::unordered_map<std::string,
                                 core::surface<core::simulation_bands>>
            material_map;

    for (const auto& i : project.persistent.materials()) {
        material_map[i.get_name()] = i.get_surface();
    }

    return scene_with_extracted_surfaces(project.get_scene_data(),
                                         material_map);
}

}  // namespace model
}  // namespace combined
}  // namespace wayverb
