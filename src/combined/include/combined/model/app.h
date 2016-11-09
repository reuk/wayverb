#pragma once

#include "combined/model/material.h"
#include "combined/model/scene.h"
#include "combined/threaded_engine.h"

#include <future>

namespace wayverb {
namespace combined {
namespace model {

class project final : public member<project, scene, materials> {
    void connect() {
        const auto connect = [&](auto& param) {
            using scoped_connection =
                    typename util::event<decltype(param)>::scoped_connection;
            return scoped_connection{param.connect_on_change([&](auto&) {
                needs_save_ = true;
                notify();
            })};
        };

        set_connections(std::make_tuple(connect(scene), connect(materials)));
    }

    const core::scene_data_loader scene_data_;
    bool needs_save_;

public:
    project(const std::string& fpath);

    static constexpr const char* model_name = "model.model";
    static constexpr const char* config_name = "config.json";

    static std::string compute_model_path(const std::string& root);
    static std::string compute_config_path(const std::string& root);

    static bool is_project_file(const std::string& fpath);

    void save_to(const std::string& fpath);

    core::generic_scene_data<cl_float3, std::string> get_scene_data() const;

    scene scene;
    materials materials;
};

////////////////////////////////////////////////////////////////////////////////

class app final {
public:
    //  SPECIAL MEMBERS  ///////////////////////////////////////////////////////
    app(const std::string& name);

    app(const app&) = delete;
    app(app&&) noexcept = delete;

    app& operator=(const app&) = delete;
    app& operator=(app&&) noexcept = delete;

    ~app() noexcept;

    //  RENDERING  /////////////////////////////////////////////////////////////
    void start_render();
    void stop_render();
    void is_rendering() const;

    //  SAVE  //////////////////////////////////////////////////////////////////
    //  TODO
    //  If currently_open_file_ is blank/not-a-.way then fire a callback asking
    //  for a save location.
    //  Add .way to the file if it is not specified.
    //  Otherwise just overwrite the existing file.
    void save();

    //  TODO
    //  Add .way to the file if it is not specified.
    //  Write the project to the specifed path, and set currently_open_file_.
    void save_as(std::string name);

    //  CALLBACKS  /////////////////////////////////////////////////////////////

    engine_state_changed::connection connect_engine_state(
            engine_state_changed::callback_type t);

    waveguide_node_positions_changed::connection connect_node_positions(
            waveguide_node_positions_changed::callback_type t);

    waveguide_node_pressures_changed::connection connect_node_pressures(
            waveguide_node_pressures_changed::callback_type t);

    raytracer_reflections_generated::connection connect_reflections(
            raytracer_reflections_generated::callback_type t);

    using encountered_error = complete_engine::encountered_error;
    encountered_error::connection connect_error_handler(
            encountered_error::callback_type t);

    project project;

private:
    std::string currently_open_file_;

    complete_engine engine_;
    std::future<void> future_;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
