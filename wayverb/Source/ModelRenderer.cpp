#include "ModelRenderer.hpp"

#include "OtherComponents/AngularLookAndFeel.hpp"
#include "OtherComponents/MoreConversions.hpp"

#include "modern_gl_utils/exceptions.h"

#include "common/azimuth_elevation.h"
#include "common/cl/common.h"
#include "common/conversions.h"
#include "common/kernel.h"

#include "common/serialize/json_read_write.h"

class SceneRendererContextLifetime::RotateMouseAction final {
public:
    RotateMouseAction(const glm::vec2 &mouse_down_position,
                      SceneRendererContextLifetime &scene)
            : mouse_down_position(mouse_down_position)
            , scene(scene)
            , starting_azel(scene.azel_target) {}

    void operator()(const glm::vec2 &position) {
        const auto diff = position - mouse_down_position;
        const auto angle_scale = 0.01;
        scene.set_rotation(AzEl{static_cast<float>(diff.x * angle_scale +
                                                   starting_azel.azimuth),
                                static_cast<float>(diff.y * angle_scale +
                                                   starting_azel.elevation)});
    }

private:
    glm::vec2 mouse_down_position;
    SceneRendererContextLifetime &scene;
    AzEl starting_azel;
};

class SceneRendererContextLifetime::MoveMouseAction final {
public:
    MoveMouseAction(const glm::vec2 &mouse_down_position,
                    SceneRendererContextLifetime &scene,
                    PointObject *to_move)
            : mouse_down_position(mouse_down_position)
            , scene(scene)
            , to_move(to_move)
            , original_position(/* TODO get to_move's position */) {}

    void operator()(const glm::vec2 &position) {}

    ~MoveMouseAction() noexcept {}

private:
    glm::vec2 mouse_down_position;
    SceneRendererContextLifetime &scene;
    PointObject *to_move;
    glm::vec3 original_position;
};

//----------------------------------------------------------------------------//

namespace {
void push_triangle_indices(aligned::vector<GLuint> &ret, const triangle &tri) {
    ret.push_back(tri.v0);
    ret.push_back(tri.v1);
    ret.push_back(tri.v2);
}
}  // namespace

aligned::vector<GLuint> MultiMaterialObject::SingleMaterialSection::get_indices(
        const scene_data &scene_data, int material_index) {
    aligned::vector<GLuint> ret;
    for (const auto &i : scene_data.get_triangles()) {
        if (i.surface == material_index) {
            push_triangle_indices(ret, i);
        }
    }
    return ret;
}

MultiMaterialObject::SingleMaterialSection::SingleMaterialSection(
        const scene_data &scene_data, int material_index) {
    const auto indices = get_indices(scene_data, material_index);
    size = indices.size();
    ibo.data(indices);
}

void MultiMaterialObject::SingleMaterialSection::do_draw(
        const glm::mat4 &modelview_matrix) const {
    ibo.bind();
    glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, nullptr);
}

glm::mat4
MultiMaterialObject::SingleMaterialSection::get_local_modelview_matrix() const {
    return glm::mat4{};
}

MultiMaterialObject::MultiMaterialObject(
        const std::shared_ptr<mglu::generic_shader> &g,
        const std::shared_ptr<LitSceneShader> &l,
        const scene_data &scene_data)
        : generic_shader(g)
        , lit_scene_shader(l) {
    for (auto i = 0; i != scene_data.get_surfaces().size(); ++i) {
        sections.emplace_back(scene_data, i);
    }

    geometry.data(convert(scene_data.get_vertices()));
    mglu::check_for_gl_error();
    colors.data(aligned::vector<glm::vec4>(scene_data.get_vertices().size(),
                                           glm::vec4(0.5, 0.5, 0.5, 1.0)));
    mglu::check_for_gl_error();

    const auto configure_vao = [this](const auto &vao, const auto &shader) {
        const auto s_vao = vao.get_scoped();
        mglu::check_for_gl_error();
        mglu::enable_and_bind_buffer(vao,
                                     geometry,
                                     shader->get_attrib_location_v_position(),
                                     3,
                                     GL_FLOAT);
        mglu::check_for_gl_error();
        mglu::enable_and_bind_buffer(vao,
                                     colors,
                                     shader->get_attrib_location_v_color(),
                                     4,
                                     GL_FLOAT);
        mglu::check_for_gl_error();
    };

    configure_vao(wire_vao, generic_shader);
    configure_vao(fill_vao, lit_scene_shader);
}

glm::mat4 MultiMaterialObject::get_local_modelview_matrix() const {
    return glm::mat4{};
}

void MultiMaterialObject::do_draw(const glm::mat4 &modelview_matrix) const {
    for (auto i = 0u; i != sections.size(); ++i) {
        if (i == highlighted) {
            const auto s_shader = lit_scene_shader->get_scoped();
            lit_scene_shader->set_model_matrix(modelview_matrix);
            const auto s_vao = fill_vao.get_scoped();
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            sections[i].draw(modelview_matrix);
        } else {
            const auto s_shader = generic_shader->get_scoped();
            generic_shader->set_model_matrix(modelview_matrix);
            const auto s_vao = wire_vao.get_scoped();
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            sections[i].draw(modelview_matrix);
        }
    }
}

void MultiMaterialObject::set_highlighted(int material) {
    highlighted = material;
}

void MultiMaterialObject::set_colour(const glm::vec3 &c) {
    const auto s_shader = lit_scene_shader->get_scoped();
    lit_scene_shader->set_colour(c);
}

//----------------------------------------------------------------------------//

PointObjects::PointObjects(const std::shared_ptr<mglu::generic_shader> &shader)
        : shader(shader) {}

void PointObjects::set_sources(const aligned::vector<glm::vec3> &u) {
    aligned::vector<PointObject> ret;
    ret.reserve(u.size());
    for (const auto &i : u) {
        PointObject p(shader, glm::vec4{0.7, 0, 0, 1});
        p.set_position(i);
        ret.push_back(std::move(p));
    }
    sources = std::move(ret);
}

void PointObjects::set_receivers(
        const aligned::vector<model::ReceiverSettings> &u) {
    aligned::vector<PointObject> ret;
    ret.reserve(u.size());
    for (const auto &i : u) {
        PointObject p(shader, glm::vec4{0, 0.7, 0.7, 1});
        p.set_position(i.position);
        p.set_pointing(get_pointing(i));
        ret.push_back(std::move(p));
    }
    receivers = std::move(ret);
}

void PointObjects::draw(const glm::mat4 &matrix) const {
    for (const auto &i : sources) {
        i.draw(matrix);
    }
    for (const auto &i : receivers) {
        i.draw(matrix);
    }
}

PointObject *PointObjects::get_currently_hovered(const glm::vec3 &origin,
                                                 const glm::vec3 &direction) {
    struct Intersection {
        PointObject *ref;
        double distance;
    };

    Intersection intersection{nullptr, 0};
    for (const auto i : get_all_point_objects()) {
        const auto diff = origin - i->get_position();
        const auto b = glm::dot(direction, diff);
        const auto c = glm::dot(diff, diff) - glm::pow(0.4, 2);
        const auto det = glm::pow(b, 2) - c;
        if (0 <= det) {
            const auto sq_det = std::sqrt(det);
            const auto dist = std::min(-b + sq_det, -b - sq_det);
            if (!intersection.ref || dist < intersection.distance) {
                intersection = Intersection{i, dist};
            }
        }
    }

    return intersection.ref;
}

aligned::vector<PointObject *> PointObjects::get_all_point_objects() {
    aligned::vector<PointObject *> ret;
    ret.reserve(sources.size() + receivers.size());
    for (auto &i : sources) {
        ret.push_back(&i);
    }
    for (auto &i : receivers) {
        ret.push_back(&i);
    }
    return ret;
}

//----------------------------------------------------------------------------//

SceneRendererContextLifetime::SceneRendererContextLifetime(
        const scene_data &scene_data, double speed_of_sound)
        : model_object(generic_shader, lit_scene_shader, scene_data)
        , point_objects(generic_shader)
        , axes(generic_shader)
        , speed_of_sound(speed_of_sound) {
    const auto aabb = scene_data.get_aabb();
    const auto m = centre(aabb);
    const auto max = glm::length(dimensions(aabb));
    eye = eye_target = max > 0 ? 20 / max : 1;
    translation = -glm::vec3(m.x, m.y, m.z);
}

void SceneRendererContextLifetime::set_eye(float u) { set_eye_impl(u); }

void SceneRendererContextLifetime::set_rotation(const AzEl &u) {
    set_rotation_impl(u);
}

void SceneRendererContextLifetime::set_rendering(bool b) {
    rendering = b;
    if (!b) {
        mesh_object = nullptr;
        ray_object = nullptr;
    } else {
        debug_mesh_object = nullptr;
    }
    allow_move_mode = !b;
}

void SceneRendererContextLifetime::set_positions(
        const aligned::vector<glm::vec3> &positions) {
    mesh_object = std::make_unique<MeshObject>(mesh_shader, positions);
}

void SceneRendererContextLifetime::set_pressures(
        const aligned::vector<float> &pressures, float current_time) {
    if (mesh_object) {
        mesh_object->set_pressures(pressures);
    }

    if (ray_object) {
        ray_object->set_distance(current_time * speed_of_sound);
    }
}

void SceneRendererContextLifetime::set_impulses(
        const aligned::vector<aligned::vector<impulse>> &impulses,
        const glm::vec3 &source,
        const glm::vec3 &receiver) {
    ray_object = std::make_unique<RayVisualisation>(
            ray_shader, impulses, source, receiver);
}

void SceneRendererContextLifetime::set_highlighted(int u) {
    model_object.set_highlighted(u);
}

void SceneRendererContextLifetime::set_emphasis(const glm::vec3 &c) {
    model_object.set_colour(c);
}

void SceneRendererContextLifetime::update(float dt) {
    eye += (eye_target - eye) * 0.1;
    azel += (azel_target - azel) * 0.1;
}

void SceneRendererContextLifetime::do_draw(
        const glm::mat4 &modelview_matrix) const {
    const auto c = 0.0;
    glClearColor(c, c, c, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const auto config_shader = [this](const auto &shader) {
        const auto s_shader = shader->get_scoped();
        shader->set_model_matrix(glm::mat4());
        shader->set_view_matrix(get_view_matrix());
        shader->set_projection_matrix(get_projection_matrix());
    };

    config_shader(generic_shader);
    config_shader(mesh_shader);
    config_shader(lit_scene_shader);
    config_shader(ray_shader);

    model_object.draw(modelview_matrix);
    point_objects.draw(modelview_matrix);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    if (rendering) {
        if (mesh_object) {
            mesh_object->draw(modelview_matrix);
        }

        if (ray_object) {
            ray_object->draw(modelview_matrix);
        }
    }

    if (debug_mesh_object) {
        debug_mesh_object->draw(modelview_matrix);
    }

    axes.draw(modelview_matrix);
}

glm::mat4 SceneRendererContextLifetime::get_local_modelview_matrix() const {
    return glm::mat4{};
}

void SceneRendererContextLifetime::mouse_down(const glm::vec2 &pos) {
    // auto hovered = point_objects.get_currently_hovered(
    //        get_world_camera_position(),
    //        get_world_mouse_direction(pos));
    // if (hovered && allow_move_mode) {
    //    mousing = std::unique_ptr<Mousing>(
    //            std::make_unique<Move>(hovered,
    //            hovered->get_position()));
    //}
    mouse_action = RotateMouseAction(pos, *this);
}

void SceneRendererContextLifetime::mouse_drag(const glm::vec2 &pos) {
    assert(mouse_action);
    mouse_action(pos);

    // if (auto m = dynamic_cast<Move *>(mousing.get())) {
    //    auto camera_position = get_world_camera_position();
    //    auto normal = get_world_camera_direction();
    //    auto original = m->original_position;
    //    auto d = glm::dot(normal, camera_position - original);
    //    auto direction = get_world_mouse_direction(pos);
    //    auto dist = -d / glm::dot(normal, direction);
    //    auto new_pos = camera_position + direction * dist;
    //    m->to_move->set_position(new_pos);
    //}
}

void SceneRendererContextLifetime::mouse_up(const glm::vec2 &pos) {
    mouse_action = decltype(mouse_action)();
}

void SceneRendererContextLifetime::mouse_wheel_move(float delta_y) {
    set_eye_impl(eye_target + delta_y);
}

void SceneRendererContextLifetime::set_sources(
        const aligned::vector<glm::vec3> &u) {
    point_objects.set_sources(u);
}

void SceneRendererContextLifetime::set_receivers(
        const aligned::vector<model::ReceiverSettings> &u) {
    point_objects.set_receivers(u);
}

void SceneRendererContextLifetime::debug_show_closest_surfaces(
        waveguide::mesh model) {
    debug_mesh_object = std::make_unique<DebugMeshObject>(
            generic_shader, model, DebugMeshObject::mode::closest_surface);
}

void SceneRendererContextLifetime::debug_show_boundary_types(
        waveguide::mesh model) {
    debug_mesh_object = std::make_unique<DebugMeshObject>(
            generic_shader, model, DebugMeshObject::mode::boundary_type);
}

void SceneRendererContextLifetime::debug_hide_model() {
    debug_mesh_object = nullptr;
}

void SceneRendererContextLifetime::set_eye_impl(float u) {
    eye_target = std::max(0.0f, u);
}

void SceneRendererContextLifetime::set_rotation_impl(const AzEl &u) {
    azel_target = AzEl{u.azimuth,
                       glm::clamp(u.elevation,
                                  static_cast<float>(-M_PI / 2),
                                  static_cast<float>(M_PI / 2))};
}

glm::vec3 SceneRendererContextLifetime::get_world_camera_position() const {
    return glm::inverse(get_view_matrix())[3];
}

glm::vec3 SceneRendererContextLifetime::get_world_camera_direction() const {
    return glm::normalize(glm::vec3{glm::inverse(get_view_matrix()) *
                                    glm::vec4{0, 0, -1, 0}});
}

glm::vec3 SceneRendererContextLifetime::get_world_mouse_direction(
        const glm::vec2 &pos) const {
    auto ray_clip = glm::vec4{(2 * pos.x) / get_viewport().x - 1,
                              1 - (2 * pos.y) / get_viewport().y,
                              -1,
                              1};
    auto ray_eye = glm::inverse(get_projection_matrix()) * ray_clip;
    ray_eye = glm::vec4{ray_eye.x, ray_eye.y, -1, 0};
    return glm::normalize(glm::vec3{glm::inverse(get_view_matrix()) * ray_eye});
}

glm::mat4 SceneRendererContextLifetime::get_projection_matrix() const {
    return glm::perspective(45.0f, get_aspect(), 0.05f, 1000.0f);
}

glm::mat4 SceneRendererContextLifetime::get_view_matrix() const {
    glm::vec3 from(0, 0, eye);
    glm::vec3 target(0, 0, 0);
    glm::vec3 up(0, 1, 0);
    return glm::lookAt(from, target, up) *
           glm::rotate(azel.elevation, glm::vec3(1, 0, 0)) *
           glm::rotate(azel.azimuth, glm::vec3(0, 1, 0)) *
           glm::translate(translation);
}
