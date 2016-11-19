#pragma once

#include "combined/model/material.h"
#include "combined/model/persistent.h"
#include "combined/model/presets/capsule.h"
#include "combined/model/presets/material.h"
#include "combined/model/scene.h"
#include "combined/threaded_engine.h"

#include "waveguide/mesh.h"

#include "core/scene_data_loader.h"

#include <future>

namespace wayverb {
namespace combined {
namespace model {

/// All the stuff that goes into a save-file/project.
/// Projects consist of a (copy of a) 3d model, along with a json save file.
class project final {
    const core::scene_data_loader scene_data_;
    bool needs_save_;

public:
    persistent persistent;

    explicit project(const std::string& fpath);

    static constexpr const char* model_name = "model.model";
    static constexpr const char* config_name = "config.json";

    static constexpr const char* project_extension = "*.way";

    static std::string compute_model_path(const std::string& root);
    static std::string compute_config_path(const std::string& root);

    static bool is_project_file(const std::string& fpath);

    std::string get_extensions() const;

    void save_to(const std::string& fpath);

    bool needs_save() const;

    core::generic_scene_data<cl_float3, std::string> get_scene_data() const;
};

////////////////////////////////////////////////////////////////////////////////

/// Holds a project file, along with
/// a list of preset materials and capsules, which will be loaded from disk,
/// some events, for updating the ui after long-running operations,
/// any transient data which needs a home,
/// an engine instance
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
    void start_render(const std::string& output_path, const std::string& unique_id);
    void cancel_render();
    bool is_rendering() const;

    //  SAVE  //////////////////////////////////////////////////////////////////
    using save_callback =
            std::function<std::experimental::optional<std::string>()>;
    /// Will call save_callback if a new filepath is required.
    void save(const save_callback& callback);

    void save_as(std::string name);

    bool needs_save() const;

    //  CALLBACKS  /////////////////////////////////////////////////////////////

    using begun = util::event<>;
    begun::connection connect_begun(begun::callback_type t);

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

    using finished = complete_engine::finished;
    finished::connection connect_finished(finished::callback_type t);

    //  MISC FUNCTIONS  ////////////////////////////////////////////////////////

    void reset_view();

    //  Backing data for rending 3D scene view.
    //  There are arguably better homes for this, but I'm short on time...
    scene scene;

    //  Project data.
    project project;

    //  Preset data.

    using material_presets_t =
            decltype(wayverb::combined::model::presets::materials);
    material_presets_t material_presets;

    using capsule_presets_t =
            decltype(wayverb::combined::model::presets::capsules);
    capsule_presets_t capsule_presets;

private:
    core::gpu_scene_data generate_scene_data();

    std::string currently_open_file_;

    begun begun_;

    complete_engine engine_;
    std::future<void> future_;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
