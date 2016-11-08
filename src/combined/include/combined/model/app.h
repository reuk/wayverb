#pragma once

#include "combined/model/material.h"
#include "combined/model/scene.h"
#include "combined/threaded_engine.h"

#include <future>

namespace wayverb {
namespace combined {
namespace model {

struct project final {
    const core::scene_data_loader scene_data;
    scene scene;
    materials materials;

    static constexpr const char* model_name = "model.model";
    static constexpr const char* config_name = "config.json";

    static std::string compute_model_path(const std::string& root);
    static std::string compute_config_path(const std::string& root);

    static bool is_project_file(const std::string& fpath);

    static project load_wayverb_project(const std::string& fpath);
    static project load_3d_object(const std::string& fpath);

    static project load(const std::string& fpath);
    static void save_to(const project& project, const std::string& fpath);
};

////////////////////////////////////////////////////////////////////////////////

class app final : public member<app, materials, scene> {
public:
    //  SPECIAL MEMBERS  ///////////////////////////////////////////////////////
    //  TODO
    //  If the file is a .way then open scene + config.
    //  Otherwise, attempt to read as a normal 3D file and use defaults for
    //  everything else.
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
    void save() const;

    //  TODO
    //  Add .way to the file if it is not specified.
    //  Write the project to the specifed path, and set currently_open_file_.
    void save_as(std::string name) const;

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

private:
    std::string currently_open_file_;

    const core::scene_data_loader scene_;

    complete_engine engine_;
    std::future<void> future_;

public:
    materials materials;
    scene scene;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
