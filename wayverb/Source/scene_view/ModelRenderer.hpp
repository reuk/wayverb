#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "BoxObject.hpp"
#include "LitSceneShader.hpp"
#include "MeshObject.hpp"
#include "ModelObject.hpp"
#include "PointObject.hpp"
#include "RayVisualisation.hpp"

#include "../model/model.h"

#include "AxesObject.hpp"
#include "BasicDrawableObject.hpp"
#include "RenderHelpers.hpp"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/noise.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"

#include "waveguide/waveguide.h"

#include "raytracer/raytracer.h"

#include "core/scene_data.h"
#include "core/spatial_division/voxel_collection.h"

#include <cmath>
#include <future>
#include <mutex>
#include <queue>

class MultiMaterialObject : public mglu::drawable {
public:
    MultiMaterialObject(
            const std::shared_ptr<mglu::generic_shader> &generic_shader,
            const std::shared_ptr<LitSceneShader> &lit_scene_shader,
            const wayverb::combined::engine::scene_data &scene_data);

    class SingleMaterialSection : public mglu::drawable {
    public:
        SingleMaterialSection(const wayverb::combined::engine::scene_data &scene_data, int material_index);

    private:
        void do_draw(const glm::mat4 &modelview_matrix) const override;
        glm::mat4 get_local_modelview_matrix() const override;

        static util::aligned::vector<GLuint> get_indices(const wayverb::combined::engine::scene_data &scene_data,
                                                   int material_index);
        mglu::static_ibo ibo;
        GLuint size;
    };

    void set_highlighted(int material);
    void set_colour(const glm::vec3 &c);

private:
    void do_draw(const glm::mat4 &modelview_matrix) const override;
    glm::mat4 get_local_modelview_matrix() const override;

    std::shared_ptr<mglu::generic_shader> generic_shader;
    std::shared_ptr<LitSceneShader> lit_scene_shader;

    mglu::vao wire_vao;
    mglu::vao fill_vao;
    mglu::static_vbo geometry;
    mglu::static_vbo colors;

    int highlighted{-1};

    util::aligned::vector<SingleMaterialSection> sections;
};

//----------------------------------------------------------------------------//

class PointObjects final {
public:
    PointObjects(const std::shared_ptr<mglu::generic_shader> &shader);

    void set_sources(util::aligned::vector<glm::vec3> u);
    void set_receivers(util::aligned::vector<model::receiver> u);

    void draw(const glm::mat4 &matrix) const;

    PointObject *get_currently_hovered(const glm::vec3 &origin,
                                       const glm::vec3 &direction);

private:
    util::aligned::vector<PointObject *> get_all_point_objects();

    std::shared_ptr<mglu::generic_shader> shader;

    util::aligned::vector<PointObject> sources;
    util::aligned::vector<PointObject> receivers;
};

//----------------------------------------------------------------------------//

class SceneRendererContextLifetime final : public BaseContextLifetime {
public:
    SceneRendererContextLifetime();

    SceneRendererContextLifetime(const SceneRendererContextLifetime &) = delete;
    SceneRendererContextLifetime &operator=(
            const SceneRendererContextLifetime &) = delete;
    SceneRendererContextLifetime(SceneRendererContextLifetime &&) = default;
    SceneRendererContextLifetime &operator=(SceneRendererContextLifetime &&) =
            default;

    void set_scene(wayverb::combined::engine::scene_data scene);

    void set_eye(float u);
    void set_rotation(const wayverb::core::az_el &u);

    void set_rendering(bool b);

    void set_positions(util::aligned::vector<glm::vec3> positions);
    void set_pressures(util::aligned::vector<float> pressures);
    void set_reflections(util::aligned::vector<util::aligned::vector<wayverb::raytracer::reflection>> reflections,
                         const glm::vec3& source);
    void set_distance_travelled(double distance);

    void set_highlighted(int u);
    void set_emphasis(const glm::vec3 &c);

    void update(float dt) override;

    void mouse_down(const glm::vec2 &pos);
    void mouse_drag(const glm::vec2 &pos);
    void mouse_up(const glm::vec2 &pos);
    void mouse_wheel_move(float delta_y);

    void set_sources  (util::aligned::vector<glm::vec3> u);
    void set_receivers(util::aligned::vector<model::receiver> u);

    void debug_show_closest_surfaces(wayverb::waveguide::mesh model);
    void debug_show_boundary_types(wayverb::waveguide::mesh model);
    void debug_hide_model();

private:
    void do_draw(const glm::mat4 &modelview_matrix) const override;
    glm::mat4 get_local_modelview_matrix() const override;

    void set_eye_impl(float u);
    void set_rotation_impl(const wayverb::core::az_el &u);

    glm::vec3 get_world_camera_position() const;
    glm::vec3 get_world_camera_direction() const;

    glm::vec3 get_world_mouse_direction(const glm::vec2 &pos) const;

    glm::mat4 get_projection_matrix() const;
    glm::mat4 get_view_matrix() const;

    std::shared_ptr<mglu::generic_shader> generic_shader{
            std::make_shared<mglu::generic_shader>()};
    std::shared_ptr<MeshShader> mesh_shader{std::make_shared<MeshShader>()};
    std::shared_ptr<LitSceneShader> lit_scene_shader{
            std::make_shared<LitSceneShader>()};
    std::shared_ptr<RayShader> ray_shader{std::make_shared<RayShader>()};

    std::unique_ptr<MultiMaterialObject> model_object;
    std::unique_ptr<MeshObject> mesh_object;
    std::unique_ptr<DebugMeshObject> debug_mesh_object;
    std::unique_ptr<RayVisualisation> ray_object;
    bool rendering{false};
    PointObjects point_objects;
    AxesObject axes;

    wayverb::core::az_el azel{0, 0};
    wayverb::core::az_el azel_target{0, 0};
    float eye{2};
    float eye_target{2};
    glm::vec3 translation{0};

    bool allow_move_mode{true};

    class RotateMouseAction;
    class MoveMouseAction;

    /// Will be constructed on mouse down, called with the current mouse
    /// position on mouse drag, destroyed on mouse up
    std::function<void(const glm::vec2 &)> mouse_action;
};
