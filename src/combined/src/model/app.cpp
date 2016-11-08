#include "combined/model/app.h"

namespace wayverb {
namespace combined {
namespace model {

app::app(const std::string& name)
        : aabb_{core::geo::compute_aabb(scene_.get_scene_data())} {
    add_source(0);
    add_receiver(0);

    connect_all(raytracer, waveguide, output, sources_, receivers_);
}

app::~app() noexcept { stop_render(); }

void app::start_render() {
    stop_render();

    //  Collect parameters.

    auto scene_data = scene_.get_scene_data();
    //  TODO replace surfaces with the surfaces from the model.

    //  Start engine in new thread.
    future_ = std::async(std::launch::async, [&] {
        engine_.run(core::compute_context{}, scene_data, scene_parameters{});
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

//  TODO scene materials

const source& app::get_source(size_t index) const { return sources_[index]; }
source& app::get_source(size_t index) { return sources_[index]; }

void app::add_source(size_t index) { sources_.emplace(index, aabb_); }

void app::remove_source(size_t index) {
    if (1 < sources_.size()) {
        sources_.erase(index);
    }
}

const receiver& app::get_receiver(size_t index) const {
    return receivers_[index];
}
receiver& app::get_receiver(size_t index) { return receivers_[index]; }

void app::add_receiver(size_t index) { receivers_.emplace(index, aabb_); }

void app::remove_receiver(size_t index) {
    if (1 < receivers_.size()) {
        receivers_.erase(index);
    }
}

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
