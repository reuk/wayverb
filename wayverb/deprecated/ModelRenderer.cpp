#include "ModelRenderer.h"

#include "AngularLookAndFeel.h"

#include "modern_gl_utils/exceptions.h"

#include "core/azimuth_elevation.h"
#include "core/cl/common.h"
#include "core/conversions.h"
#include "core/kernel.h"

PointObjects::PointObjects(const std::shared_ptr<mglu::generic_shader> &shader)
        : shader(shader) {}

void PointObjects::set_sources(util::aligned::vector<glm::vec3> u) {
    util::aligned::vector<PointObject> ret;
    ret.reserve(u.size());
    for (const auto &i : u) {
        PointObject p(shader, glm::vec4{0.7, 0, 0, 1});
        p.set_position(i);
        ret.emplace_back(std::move(p));
    }
    sources = std::move(ret);
}

void PointObjects::set_receivers(util::aligned::vector<model::receiver> u) {
    util::aligned::vector<PointObject> ret;
    ret.reserve(u.size());
    for (const auto &i : u) {
        PointObject p(shader, glm::vec4{0, 0.7, 0.7, 1});
        p.set_position(i.position);
        p.set_pointing(util::map_to_vector(
                begin(i.capsules), end(i.capsules), [](const auto &i) {
                    return i->get_pointing();
                }));
        ret.emplace_back(std::move(p));
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

util::aligned::vector<PointObject *> PointObjects::get_all_point_objects() {
    util::aligned::vector<PointObject *> ret;
    ret.reserve(sources.size() + receivers.size());
    for (auto &i : sources) {
        ret.emplace_back(&i);
    }
    for (auto &i : receivers) {
        ret.emplace_back(&i);
    }
    return ret;
}

//----------------------------------------------------------------------------//

SceneRendererContextLifetime::SceneRendererContextLifetime()
        : point_objects(generic_shader)
        , axes(generic_shader) {}

void SceneRendererContextLifetime::set_scene(
        wayverb::core::gpu_scene_data scene) {
    const auto aabb = wayverb::core::geo::compute_aabb(scene.get_vertices());
    const auto m = centre(aabb);
    const auto max = glm::length(dimensions(aabb));
    eye = eye_target = max > 0 ? 20 / max : 1;
    translation = -glm::vec3(m.x, m.y, m.z);

    model_object = std::make_unique<MultiMaterialObject>(
            generic_shader, lit_scene_shader, std::move(scene));
}

void SceneRendererContextLifetime::set_eye(float u) { set_eye_impl(u); }

void SceneRendererContextLifetime::set_rotation(const wayverb::core::az_el &u) {
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
        util::aligned::vector<glm::vec3> positions) {
    mesh_object =
            std::make_unique<MeshObject>(mesh_shader, std::move(positions));
}

void SceneRendererContextLifetime::set_pressures(
        util::aligned::vector<float> pressures) {
    if (mesh_object) {
        mesh_object->set_pressures(std::move(pressures));
    }
}

void SceneRendererContextLifetime::set_reflections(
        util::aligned::vector<util::aligned::vector<
                wayverb::raytracer::reflection>> reflections,
        const glm::vec3 &source) {
    //  TODO
    // ray_object = std::make_unique<RayVisualisation>(
    //        ray_shader, impulses, source, receiver);
}

void SceneRendererContextLifetime::set_distance_travelled(double distance) {
    if (ray_object) {
        ray_object->set_distance(distance);
    }
}

void SceneRendererContextLifetime::set_highlighted(int u) {
    if (model_object) {
        model_object->set_highlighted(u);
    }
}

void SceneRendererContextLifetime::set_emphasis(const glm::vec3 &c) {
    if (model_object) {
        model_object->set_colour(c);
    }
}

void SceneRendererContextLifetime::update(float dt) {
    eye += (eye_target - eye) * 0.1;
    azel += (azel_target - azel) * wayverb::core::az_el{0.1, 0.1};
}

void SceneRendererContextLifetime::do_draw(
        const glm::mat4 &model_matrix) const {
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

    if (model_object) {
        model_object->draw(model_matrix);
    }
    point_objects.draw(model_matrix);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    if (rendering) {
        if (mesh_object) {
            mesh_object->draw(model_matrix);
        }

        if (ray_object) {
            ray_object->draw(model_matrix);
        }
    }

    if (debug_mesh_object) {
        debug_mesh_object->draw(model_matrix);
    }

    axes.draw(model_matrix);
}

glm::mat4 SceneRendererContextLifetime::get_local_model_matrix() const {
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
        util::aligned::vector<glm::vec3> u) {
    point_objects.set_sources(std::move(u));
}

void SceneRendererContextLifetime::set_receivers(
        util::aligned::vector<model::receiver> u) {
    point_objects.set_receivers(std::move(u));
}

void SceneRendererContextLifetime::debug_show_closest_surfaces(
        wayverb::waveguide::mesh model) {
    debug_mesh_object = std::make_unique<DebugMeshObject>(
            generic_shader, model, DebugMeshObject::mode::closest_surface);
}

void SceneRendererContextLifetime::debug_show_boundary_types(
        wayverb::waveguide::mesh model) {
    debug_mesh_object = std::make_unique<DebugMeshObject>(
            generic_shader, model, DebugMeshObject::mode::boundary_type);
}

void SceneRendererContextLifetime::debug_hide_model() {
    debug_mesh_object = nullptr;
}

void SceneRendererContextLifetime::set_eye_impl(float u) {
    eye_target = std::max(0.0f, u);
}

void SceneRendererContextLifetime::set_rotation_impl(
        const wayverb::core::az_el &u) {
    azel_target =
            wayverb::core::az_el{u.azimuth,
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
