#include "combined/model/app.h"

namespace wayverb {
namespace combined {
namespace model {

app::app(const std::string& name)
        : scene{core::geo::compute_aabb(scene_.get_scene_data())} {
    connect(scene, materials_);
}

app::~app() noexcept { stop_render(); }

void app::start_render() {
    stop_render();

    //  Collect parameters.

    util::aligned::unordered_map<std::string,
                                 core::surface<core::simulation_bands>>
            material_map;

    for (const auto& i : materials_) {
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

//  TODO
//  If currently_open_file_ is blank/not-a-.way then fire a callback
//  asking
//  for a save location.
//  Add .way to the file if it is not specified.
//  Otherwise just overwrite the existing file.
void app::save() const {}

//  TODO
//  Add .way to the file if it is not specified.
//  Write the project to the specifed path, and set
//  currently_open_file_.
void app::save_as(std::string name) const {}

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
