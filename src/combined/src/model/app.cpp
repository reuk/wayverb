#include "combined/model/app.h"

#include "core/cl/common.h"
#include "core/serialize/range.h"
#include "core/serialize/surface.h"

#include "cereal/archives/json.hpp"
#include "cereal/types/string.hpp"

#include <fstream>

namespace wayverb {
namespace combined {
namespace model {

project::project(const std::string& fpath)
        : scene_data_{is_project_file(fpath) ? compute_model_path(fpath)
                                             : fpath}
        , needs_save_{!is_project_file(fpath)}
        , scene{core::geo::compute_aabb(scene_data_.get_scene_data())} {
    if (is_project_file(fpath)) {
        const auto config_file = project::compute_config_path(fpath);

        //  load the config
        std::ifstream stream(config_file);
        cereal::JSONInputArchive archive(stream);
        archive(scene, materials);

    } else {
        const auto& surface_strings =
                scene_data_.get_scene_data().get_surfaces();
        materials.set_from_strings(begin(surface_strings),
                                   end(surface_strings));
    }

    connect();
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
        archive(scene, materials);

        needs_save_ = false;

        //  TODO register_recent_file(f.getFullPathName().toStdString());
    }
}

core::generic_scene_data<cl_float3, std::string> project::get_scene_data()
        const {
    return scene_data_.get_scene_data();
}

////////////////////////////////////////////////////////////////////////////////

app::app(const std::string& name)
        : project{name}
        , currently_open_file_{name} {}

app::~app() noexcept { stop_render(); }

void app::start_render() {
    stop_render();

    //  Collect parameters.

    util::aligned::unordered_map<std::string,
                                 core::surface<core::simulation_bands>>
            material_map;

    for (const auto& i : project.materials) {
        material_map[i.get_name()] = i.get_surface();
    }

    auto scene_data = scene_with_extracted_surfaces(project.get_scene_data(),
                                                    material_map);

    //  Start engine in new thread.
    future_ = std::async(std::launch::async, [&] {
        engine_.run(
                core::compute_context{}, std::move(scene_data), project.scene);
    });
}

void app::stop_render() {
    engine_.cancel();
    if (future_.valid()) {
        future_.get();
    }
}

void app::is_rendering() const { engine_.is_running(); }

void app::save() {
    if (project::is_project_file(currently_open_file_)) {
        project.save_to(currently_open_file_);
    } else {
        //  TODO ask where to save file
        //  TODO append .way if filename is not valid
    }
}

void app::save_as(const std::string& name) {
    //  TODO append .way if filename is not valid
    project.save_to(name);
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

}  // namespace model
}  // namespace combined
}  // namespace wayverb
