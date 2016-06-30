#include "ModelRenderer.hpp"

#include "OtherComponents/AngularLookAndFeel.hpp"
#include "OtherComponents/MoreConversions.hpp"

#include "common/azimuth_elevation.h"
#include "common/boundaries.h"
#include "common/cl_common.h"
#include "common/conversions.h"
#include "common/kernel.h"

#include "common/serialize/json_read_write.h"

namespace {
void push_triangle_indices(std::vector<GLuint> &ret, const Triangle &tri) {
    ret.push_back(tri.v0);
    ret.push_back(tri.v1);
    ret.push_back(tri.v2);
}
}  // namespace

std::vector<GLuint> MultiMaterialObject::SingleMaterialSection::get_indices(
        const CopyableSceneData &scene_data, int material_index) {
    std::vector<GLuint> ret;
    for (const auto &i : scene_data.get_triangles()) {
        if (i.surface == material_index) {
            push_triangle_indices(ret, i);
        }
    }
    return ret;
}

MultiMaterialObject::SingleMaterialSection::SingleMaterialSection(
        const CopyableSceneData &scene_data, int material_index) {
    auto indices = get_indices(scene_data, material_index);
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

MultiMaterialObject::MultiMaterialObject(mglu::GenericShader &generic_shader,
                                         LitSceneShader &lit_scene_shader,
                                         const CopyableSceneData &scene_data)
        : generic_shader(&generic_shader)
        , lit_scene_shader(&lit_scene_shader) {
    for (auto i = 0; i != scene_data.get_surfaces().size(); ++i) {
        sections.emplace_back(scene_data, i);
    }

    geometry.data(scene_data.get_converted_vertices());
    colors.data(std::vector<glm::vec4>(scene_data.get_vertices().size(),
                                       glm::vec4(0.5, 0.5, 0.5, 1.0)));

    auto configure_vao = [this](const auto &vao, const auto &shader) {
        auto s_vao = vao.get_scoped();

        geometry.bind();
        auto v_pos = shader.get_attrib_location("v_position");
        glEnableVertexAttribArray(v_pos);
        glVertexAttribPointer(v_pos, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        colors.bind();
        auto c_pos = shader.get_attrib_location("v_color");
        glEnableVertexAttribArray(c_pos);
        glVertexAttribPointer(c_pos, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
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
            auto s_shader = lit_scene_shader->get_scoped();
            lit_scene_shader->set_model_matrix(modelview_matrix);
            auto s_vao = fill_vao.get_scoped();
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            sections[i].draw(modelview_matrix);
        } else {
            auto s_shader = generic_shader->get_scoped();
            generic_shader->set_model_matrix(modelview_matrix);
            auto s_vao = wire_vao.get_scoped();
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            sections[i].draw(modelview_matrix);
        }
    }
}

void MultiMaterialObject::set_highlighted(int material) {
    highlighted = material;
}

void MultiMaterialObject::set_colour(const glm::vec3 &c) {
    auto s = lit_scene_shader->get_scoped();
    lit_scene_shader->set_colour(c);
}

//----------------------------------------------------------------------------//

class PointObjects final {
public:
    PointObjects(mglu::GenericShader &shader)
            : shader(shader) {
    }

    void set_sources(const std::vector<glm::vec3> &u) {
        std::vector<PointObject> ret;
        ret.reserve(u.size());
        for (const auto &i : u) {
            PointObject p(shader, glm::vec4{0.7, 0, 0, 1});
            p.set_position(i);
            ret.push_back(std::move(p));
        }
        sources = std::move(ret);
    }

    void set_receivers(const std::vector<model::ReceiverSettings> &u) {
        std::vector<PointObject> ret;
        ret.reserve(u.size());
        for (const auto &i : u) {
            PointObject p(shader, glm::vec4{0, 0.7, 0.7, 1});
            p.set_position(i.position);
            p.set_pointing(i.get_pointing());
            ret.push_back(std::move(p));
        }
        receivers = std::move(ret);
    }

    void draw(const glm::mat4 &matrix) const {
        for (const auto &i : sources) {
            i.draw(matrix);
        }
        for (const auto &i : receivers) {
            i.draw(matrix);
        }
    }

    PointObject *get_currently_hovered(const glm::vec3 &origin,
                                       const glm::vec3 &direction) {
        struct Intersection {
            PointObject *ref;
            double distance;
        };

        Intersection intersection{nullptr, 0};
        for (auto i : get_all_point_objects()) {
            auto diff = origin - i->get_position();
            auto b = glm::dot(direction, diff);
            auto c = glm::dot(diff, diff) - glm::pow(0.4, 2);
            auto det = glm::pow(b, 2) - c;
            if (0 <= det) {
                auto sq_det = std::sqrt(det);
                auto dist = std::min(-b + sq_det, -b - sq_det);
                if (!intersection.ref || dist < intersection.distance) {
                    intersection = Intersection{i, dist};
                }
            }
        }

        return intersection.ref;
    }

private:
    std::vector<PointObject *> get_all_point_objects() {
        std::vector<PointObject *> ret;
        ret.reserve(sources.size() + receivers.size());
        for (auto &i : sources) {
            ret.push_back(&i);
        }
        for (auto &i : receivers) {
            ret.push_back(&i);
        }
        return ret;
    }

    mglu::GenericShader &shader;

    std::vector<PointObject> sources;
    std::vector<PointObject> receivers;
};

//----------------------------------------------------------------------------//

class SceneRenderer::ContextLifetime : public BaseContextLifetime {
public:
    ContextLifetime(SceneRenderer &owner, const CopyableSceneData &scene_data)
            : owner(owner)
            , model(scene_data)
            , model_object(generic_shader, lit_scene_shader, scene_data)
            , point_objects(generic_shader)
            , axes(generic_shader) {
        auto aabb = model.get_aabb();
        auto m = aabb.centre();
        auto max = glm::length(aabb.dimensions());
        eye = eye_target = max > 0 ? 20 / max : 1;
        translation = -glm::vec3(m.x, m.y, m.z);
    }

    void set_eye(float u) {
        std::lock_guard<std::mutex> lck(mut);
        set_eye_impl(u);
    }

    void set_rotation(const Orientable::AzEl &u) {
        std::lock_guard<std::mutex> lck(mut);
        set_rotation_impl(u);
    }

    void set_rendering(bool b) {
        std::lock_guard<std::mutex> lck(mut);
        rendering = b;
        if (!b) {
            if (mesh_object) {
                mesh_object->zero_pressures();
            }
        }
        allow_move_mode = !b;
    }

    void set_positions(const std::vector<cl_float3> &positions) {
        std::lock_guard<std::mutex> lck(mut);
        std::vector<glm::vec3> ret(positions.size());
        proc::transform(positions, ret.begin(), [](const auto &i) {
            return to_glm_vec3(i);
        });
        mesh_object = std::make_unique<MeshObject>(mesh_shader, ret);
    }

    void set_pressures(const std::vector<float> &pressures) {
        std::lock_guard<std::mutex> lck(mut);
        assert(mesh_object);
        mesh_object->set_pressures(pressures);
    }

    void set_highlighted(int u) {
        std::lock_guard<std::mutex> lck(mut);
        model_object.set_highlighted(u);
    }

    void set_emphasis(const glm::vec3 &c) {
        std::lock_guard<std::mutex> lck(mut);
        model_object.set_colour(c);
    }

    void update(float dt) override {
        eye += (eye_target - eye) * 0.1;
        azel += (azel_target - azel) * 0.1;
    }

    void do_draw(const glm::mat4 &modelview_matrix) const override {
        std::lock_guard<std::mutex> lck(mut);
        auto c = 0.0;
        glClearColor(c, c, c, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_DEPTH_TEST);

        glEnable(GL_MULTISAMPLE);
        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_POLYGON_SMOOTH);
        //        glEnable(GL_BLEND);
        //        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        auto config_shader = [this](const auto &shader) {
            auto s_shader = shader.get_scoped();
            shader.set_model_matrix(glm::mat4());
            shader.set_view_matrix(get_view_matrix());
            shader.set_projection_matrix(get_projection_matrix());
        };

        config_shader(generic_shader);
        config_shader(mesh_shader);
        config_shader(lit_scene_shader);

        model_object.draw(modelview_matrix);
        point_objects.draw(modelview_matrix);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        if (rendering && mesh_object) {
            mesh_object->draw(modelview_matrix);
        }

        axes.draw(modelview_matrix);
    }

    glm::mat4 get_local_modelview_matrix() const override {
        std::lock_guard<std::mutex> lck(mut);
        return glm::mat4{};
    }

    void mouse_down(const glm::vec2 &pos) override {
        std::lock_guard<std::mutex> lck(mut);
//        auto hovered = point_objects.get_currently_hovered(
//                get_world_camera_position(), get_world_mouse_direction(pos));
//        if (hovered && allow_move_mode) {
//            mousing = std::unique_ptr<Mousing>(
//                    std::make_unique<Move>(hovered, hovered->get_position()));
//        } else {
            mousing = std::unique_ptr<Mousing>(
                    std::make_unique<Rotate>(azel_target, pos));
//        }
    }

    void mouse_drag(const glm::vec2 &pos) override {
        std::lock_guard<std::mutex> lck(mut);
        assert(mousing);
        if (auto m = dynamic_cast<Rotate *>(mousing.get())) {
            auto diff = pos - m->position;
            set_rotation_impl(Orientable::AzEl{
                    m->orientation.azimuth + diff.x * Rotate::angle_scale,
                    m->orientation.elevation + diff.y * Rotate::angle_scale});
        } else if (auto m = dynamic_cast<Move *>(mousing.get())) {
            auto camera_position = get_world_camera_position();
            auto normal = get_world_camera_direction();

            auto original = m->original_position;

            auto d = glm::dot(normal, camera_position - original);

            auto direction = get_world_mouse_direction(pos);
            auto dist = -d / glm::dot(normal, direction);
            auto new_pos = camera_position + direction * dist;

            m->to_move->set_position(new_pos);
        }
    }

    void mouse_up(const glm::vec2 &pos) override {
        std::lock_guard<std::mutex> lck(mut);
        mousing = nullptr;
    }

    void mouse_wheel_move(float delta_y) override {
        std::lock_guard<std::mutex> lck(mut);
        set_eye_impl(eye_target + delta_y);
    }

    void set_sources(const std::vector<glm::vec3> &u) {
        point_objects.set_sources(u);
    }

    void set_receivers(const std::vector<model::ReceiverSettings> &u) {
        point_objects.set_receivers(u);
    }

private:
    void set_eye_impl(float u) {
        eye_target = std::max(0.0f, u);
    }

    void set_rotation_impl(const Orientable::AzEl &u) {
        azel_target =
                Orientable::AzEl{u.azimuth,
                                 glm::clamp(u.elevation,
                                            static_cast<float>(-M_PI / 2),
                                            static_cast<float>(M_PI / 2))};
    }

    glm::vec3 get_world_camera_position() const {
        return glm::inverse(get_view_matrix())[3];
    }

    glm::vec3 get_world_camera_direction() const {
        return glm::normalize(glm::vec3{glm::inverse(get_view_matrix()) *
                                        glm::vec4{0, 0, -1, 0}});
    }

    glm::vec3 get_world_mouse_direction(const glm::vec2 &pos) const {
        auto ray_clip = glm::vec4{(2 * pos.x) / get_viewport().x - 1,
                                  1 - (2 * pos.y) / get_viewport().y,
                                  -1,
                                  1};
        auto ray_eye = glm::inverse(get_projection_matrix()) * ray_clip;
        ray_eye = glm::vec4{ray_eye.x, ray_eye.y, -1, 0};
        return glm::normalize(
                glm::vec3{glm::inverse(get_view_matrix()) * ray_eye});
    }

    glm::mat4 get_projection_matrix() const {
        return glm::perspective(45.0f, get_aspect(), 0.05f, 1000.0f);
    }

    glm::mat4 get_view_matrix() const {
        glm::vec3 from(0, 0, eye);
        glm::vec3 target(0, 0, 0);
        glm::vec3 up(0, 1, 0);
        return glm::lookAt(from, target, up) *
               glm::rotate(azel.elevation, glm::vec3(1, 0, 0)) *
               glm::rotate(azel.azimuth, glm::vec3(0, 1, 0)) *
               glm::translate(translation);
    }

    mutable std::mutex mut;

    SceneRenderer &owner;
    const CopyableSceneData &model;

    mglu::GenericShader generic_shader;
    MeshShader mesh_shader;
    LitSceneShader lit_scene_shader;

    MultiMaterialObject model_object;

    std::unique_ptr<MeshObject> mesh_object;

    bool rendering{false};

    PointObjects point_objects;

    AxesObject axes;

    Orientable::AzEl azel;
    Orientable::AzEl azel_target;
    float eye;
    float eye_target;
    glm::vec3 translation;

    bool allow_move_mode{true};

    struct Mousing {
        virtual ~Mousing() noexcept = default;
    };

    struct Rotate : public Mousing {
        Rotate(const Orientable::AzEl &azel, const glm::vec2 &position)
                : orientation(azel)
                , position(position) {
        }

        static const float angle_scale;
        Orientable::AzEl orientation;
        glm::vec2 position;
    };

    struct Move : public Mousing {
        Move(PointObject *to_move, const glm::vec3 &v)
                : to_move(to_move)
                , original_position(v) {
            to_move->set_highlight(0.5);
        }
        virtual ~Move() noexcept {
            to_move->set_highlight(0);
        }

        PointObject *to_move{nullptr};
        glm::vec3 original_position;
    };

    std::unique_ptr<Mousing> mousing;
};

const float SceneRenderer::ContextLifetime::Rotate::angle_scale{0.01};

//----------------------------------------------------------------------------//

SceneRenderer::SceneRenderer(const CopyableSceneData &model)
        : model(model) {
}

//  defined here so that we can PIMPL the ContextLifetime
SceneRenderer::~SceneRenderer() noexcept = default;

void SceneRenderer::newOpenGLContextCreated() {
    std::lock_guard<std::mutex> lck(mut);
    context_lifetime = std::make_unique<ContextLifetime>(*this, model);
    context_lifetime->set_emphasis(
            glm::vec3(AngularLookAndFeel::emphasis.getFloatRed(),
                      AngularLookAndFeel::emphasis.getFloatGreen(),
                      AngularLookAndFeel::emphasis.getFloatBlue()));
    BaseRenderer::newOpenGLContextCreated();
}

void SceneRenderer::openGLContextClosing() {
    std::lock_guard<std::mutex> lck(mut);
    context_lifetime = nullptr;
    BaseRenderer::openGLContextClosing();
}

void SceneRenderer::set_rendering(bool b) {
    std::lock_guard<std::mutex> lck(mut);
    push_incoming([this, b] { context_lifetime->set_rendering(b); });
}

void SceneRenderer::set_sources(const std::vector<glm::vec3> &sources) {
    std::lock_guard<std::mutex> lck(mut);
    push_incoming([this, sources] { context_lifetime->set_sources(sources); });
}

void SceneRenderer::set_receivers(
        const std::vector<model::ReceiverSettings> &receivers) {
    std::lock_guard<std::mutex> lck(mut);
    push_incoming(
            [this, receivers] { context_lifetime->set_receivers(receivers); });
}

void SceneRenderer::set_positions(const std::vector<cl_float3> &positions) {
    std::lock_guard<std::mutex> lck(mut);
    push_incoming(
            [this, positions] { context_lifetime->set_positions(positions); });
}

void SceneRenderer::set_pressures(const std::vector<float> &pressures) {
    std::lock_guard<std::mutex> lck(mut);
    push_incoming(
            [this, pressures] { context_lifetime->set_pressures(pressures); });
}

void SceneRenderer::set_highlighted(int u) {
    std::lock_guard<std::mutex> lck(mut);
    push_incoming([this, u] { context_lifetime->set_highlighted(u); });
}

void SceneRenderer::set_emphasis(const glm::vec3 &u) {
    std::lock_guard<std::mutex> lck(mut);
    push_incoming([this, u] { context_lifetime->set_emphasis(u); });
}

void SceneRenderer::broadcast_receiver_positions(
        const std::vector<glm::vec3> &pos) {
    push_outgoing([this, pos] {
        listener_list.call(&Listener::receiver_dragged, this, pos);
    });
}

void SceneRenderer::broadcast_source_positions(
        const std::vector<glm::vec3> &pos) {
    push_outgoing([this, pos] {
        listener_list.call(&Listener::source_dragged, this, pos);
    });
}

void SceneRenderer::addListener(Listener *l) {
    std::lock_guard<std::mutex> lck(mut);
    listener_list.add(l);
}
void SceneRenderer::removeListener(Listener *l) {
    std::lock_guard<std::mutex> lck(mut);
    listener_list.remove(l);
}

BaseContextLifetime *SceneRenderer::get_context_lifetime() {
    return context_lifetime.get();
}