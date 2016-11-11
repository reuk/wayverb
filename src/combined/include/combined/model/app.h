#pragma once

#include "combined/model/material.h"
#include "combined/model/persistent.h"
#include "combined/threaded_engine.h"

#include "waveguide/mesh.h"

#include "core/scene_data_loader.h"

#include <future>

namespace wayverb {
namespace combined {
namespace model {

class state final
        : public owning_member<
                  state,
                  persistent,                    // save-file contents
                  vector<material, 0>,           // preset materials
                  vector<vector<capsule, 1>, 0>  // preset capsule groups
                  > {
public:
    explicit state(const core::geo::box& aabb);

    using persistent_t = class persistent;
    using material_presets_t = vector<material, 0>;
    using capsule_presets_t = vector<vector<capsule, 1>, 0>;

    persistent_t& persistent();
    const persistent_t& persistent() const;

    material_presets_t& material_presets();
    const material_presets_t& material_presets() const;

    capsule_presets_t& capsule_presets();
    const capsule_presets_t& capsule_presets() const;
};

////////////////////////////////////////////////////////////////////////////////

class project final {
    const core::scene_data_loader scene_data_;
    bool needs_save_;

public:
    state state;

    explicit project(const std::string& fpath);

    static constexpr const char* model_name = "model.model";
    static constexpr const char* config_name = "config.json";

    static std::string compute_model_path(const std::string& root);
    static std::string compute_config_path(const std::string& root);

    static bool is_project_file(const std::string& fpath);

    void save_to(const std::string& fpath);

    core::generic_scene_data<cl_float3, std::string> get_scene_data() const;
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
    void cancel_render();
    //  void is_rendering() const;

    //  SAVE  //////////////////////////////////////////////////////////////////
    using save_callback =
            std::function<std::experimental::optional<std::string>()>;
    /// Will call save_callback if a new filepath is required.
    void save(const save_callback& callback);

    void save_as(std::string name);

    //  DEBUG  /////////////////////////////////////////////////////////////////

    void generate_debug_mesh();

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

    using mesh_generated = util::event<wayverb::waveguide::mesh>;
    mesh_generated::connection connect_mesh_generated(
            mesh_generated::callback_type t);

    project project;

private:
    core::gpu_scene_data generate_scene_data();

    std::string currently_open_file_;

    mesh_generated mesh_generated_;

    complete_engine engine_;
    std::future<void> future_;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
