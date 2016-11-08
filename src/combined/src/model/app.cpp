#include "combined/model/app.h"

#include <fstream>

namespace wayverb {
namespace combined {
namespace model {

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

project project::load_wayverb_project(const std::string& fpath) {
    //  look inside for a model
    const auto model_file = project::compute_model_path(fpath);

    //  load the model
    wayverb::core::scene_data_loader scene_loader{model_file};
    const auto aabb = core::geo::compute_aabb(scene_loader.get_scene_data());

    //  look inside for a config
    const auto config_file = project::compute_config_path(fpath);

    project ret{std::move(scene_loader), aabb};

    //  load the config
    std::ifstream stream(config_file);
    cereal::JSONInputArchive archive(stream);
    archive(ret.scene, ret.materials);

    return ret;
}

project project::load_3d_object(const std::string& fpath) {
    wayverb::core::scene_data_loader scene_loader{fpath};
    const auto aabb = core::geo::compute_aabb(scene_loader.get_scene_data());
    return project{std::move(scene_loader), aabb};
}

project project::load(const std::string& fpath) {
    return project::is_project_file(fpath) ? load_wayverb_project(fpath)
                                           : load_3d_object(fpath);
}

void project::save_to(const project& project, const std::string& fpath) {
    //  TODO create directory

    //  write current geometry to file
    project.scene_data.save(project::compute_model_path(fpath));

    //  write config with all current materials to file
    std::ofstream stream(project::compute_config_path(fpath));
    cereal::JSONOutputArchive archive(stream);
    archive(project.scene, project.materials);

    //  TODO register_recent_file(f.getFullPathName().toStdString());
}

////////////////////////////////////////////////////////////////////////////////

//  TODO
app::app(const std::string& name) {
    connect(materials, scene);
}

app::~app() noexcept { stop_render(); }

void app::start_render() {
    stop_render();

    //  Collect parameters.

    util::aligned::unordered_map<std::string,
                                 core::surface<core::simulation_bands>>
            material_map;

    for (const auto& i : materials) {
        material_map[i.get_name()] = i.get_surface();
    }

    auto scene_data = scene_with_extracted_surfaces(scene_.get_scene_data(),
                                                    material_map);

    //  TODO replace surfaces with the surfaces from the model.

    //  Start engine in new thread.
    future_ = std::async(std::launch::async, [&] {
        engine_.run(core::compute_context{}, std::move(scene_data), scene);
    });
}

void app::stop_render() {
    engine_.cancel();
    if (future_.valid()) {
        future_.get();
    }
}

void app::is_rendering() const { engine_.is_running(); }

void app::save() const {
    if (project::is_project_file(currently_open_file_)) {
        project::save_to(project, currently_open_file_);
    } else {
        //  TODO ask where to save file
        //  TODO append .way if filename is not valid
    }
}

void app::save_as(std::string name) const {
    //  TODO append .way if filename is not valid
    project::save_to(project, name);
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
