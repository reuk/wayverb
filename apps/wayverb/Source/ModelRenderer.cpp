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

class SceneRenderer::ContextLifetime : public BaseContextLifetime,
                                       public model::BroadcastListener {
public:
    ContextLifetime(SceneRenderer &owner,
                    const CopyableSceneData &scene_data,
                    model::ValueWrapper<std::vector<glm::vec3>> &sources,
                    model::ValueWrapper<std::vector<model::ReceiverSettings>>
                            &receivers)
            : owner(owner)
            , model(scene_data)
            , model_object(generic_shader, lit_scene_shader, scene_data)
            , sources(sources)
            , receivers(receivers)
            , axes(generic_shader) {
        auto aabb = model.get_aabb();
        auto m = aabb.centre();
        auto max = glm::length(aabb.dimensions());
        eye = eye_target = max > 0 ? 20 / max : 1;
        translation = -glm::vec3(m.x, m.y, m.z);

        sources_connector.trigger();
        receivers_connector.trigger();
    }

    //  called from another thread probably
    void receive_broadcast(model::Broadcaster *b) override {
        if (b == &sources) {
            incoming_work_queue.push([this] {
                std::vector<std::unique_ptr<PointObject>> ret;
                ret.reserve(sources.size());
                for (auto &i : sources) {
                    auto tmp = std::make_unique<PointObject>(
                            generic_shader, glm::vec4{0.7, 0, 0, 1});
                    tmp->set_position(*i);
                    ret.push_back(std::move(tmp));
                }
                source_objects = std::move(ret);
            });
        } else if (b == &receivers) {
            incoming_work_queue.push([this] {
                std::vector<std::unique_ptr<PointObject>> ret;
                ret.reserve(receivers.size());
                for (auto &i : receivers) {
                    auto tmp = std::make_unique<PointObject>(
                            generic_shader, glm::vec4{0, 0.7, 0.7, 1});
                    tmp->set_position(i->position);
                    tmp->set_pointing(i->get().get_pointing());
                    ret.push_back(std::move(tmp));
                }
                receiver_objects = std::move(ret);
            });
        }
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
        std::lock_guard<std::mutex> lck(mut);
        while (auto i = incoming_work_queue.pop()) {
            (*i)();
        }

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

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        for (const auto &i : source_objects) {
            i->draw(modelview_matrix);
        }

        for (const auto &i : receiver_objects) {
            i->draw(modelview_matrix);
        }

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
        auto hovered = get_currently_hovered(pos);
        if (hovered && allow_move_mode) {
            model::ValueWrapper<glm::vec3> *model = nullptr;
            assert(sources.size() == source_objects.size());
            for (auto i = 0u; i != sources.size() && !model; ++i) {
                if (source_objects[i].get() == hovered) {
                    model = &sources[i];
                }
            }
            assert(receivers.size() == receiver_objects.size());
            for (auto i = 0u; i != receivers.size() && !model; ++i) {
                if (receiver_objects[i].get() == hovered) {
                    model = &receivers[i].position;
                }
            }
            assert(model);
            mousing = std::unique_ptr<Mousing>(std::make_unique<Move>(
                    model, hovered, hovered->get_position()));
        } else {
            mousing = std::unique_ptr<Mousing>(
                    std::make_unique<Rotate>(azel_target, pos));
        }
    }

    void mouse_drag(const glm::vec2 &pos) override {
        std::lock_guard<std::mutex> lck(mut);
        assert(mousing);
        switch (mousing->get_mode()) {
            case Mousing::Mode::move: {
                auto camera_position = get_world_camera_position();
                auto normal = get_world_camera_direction();

                const auto &m = dynamic_cast<Move &>(*mousing);
                auto original = m.original_position;

                auto d = glm::dot(normal, camera_position - original);

                auto direction = get_world_mouse_direction(pos);
                auto dist = -d / glm::dot(normal, direction);
                auto new_pos = camera_position + direction * dist;

                m.model->set(new_pos);

                break;
            }
            case Mousing::Mode::rotate: {
                const auto &m = dynamic_cast<Rotate &>(*mousing);
                auto diff = pos - m.position;
                set_rotation_impl(Orientable::AzEl{
                        m.orientation.azimuth + diff.x * Rotate::angle_scale,
                        m.orientation.elevation +
                                diff.y * Rotate::angle_scale});
                break;
            }
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

    std::vector<PointObject *> get_all_point_objects() {
        std::vector<PointObject *> ret;
        ret.reserve(source_objects.size() + receiver_objects.size());
        for (auto &i : source_objects) {
            ret.push_back(i.get());
        }
        for (auto &i : receiver_objects) {
            ret.push_back(i.get());
        }
        return ret;
    }

    PointObject *get_currently_hovered(const glm::vec2 &pos) {
        auto origin = get_world_camera_position();
        auto direction = get_world_mouse_direction(pos);

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

    model::ValueWrapper<std::vector<glm::vec3>> &sources;
    model::ValueWrapper<std::vector<model::ReceiverSettings>> &receivers;

    model::BroadcastConnector sources_connector{&sources, this};
    model::BroadcastConnector receivers_connector{&receivers, this};

    std::vector<std::unique_ptr<PointObject>> source_objects;
    std::vector<std::unique_ptr<PointObject>> receiver_objects;

    bool rendering{false};

    AxesObject axes;

    Orientable::AzEl azel;
    Orientable::AzEl azel_target;
    float eye;
    float eye_target;
    glm::vec3 translation;

    WorkQueue incoming_work_queue;

    bool allow_move_mode{true};

    struct Mousing {
        virtual ~Mousing() noexcept = default;
        enum class Mode { rotate, move };
        virtual Mode get_mode() const = 0;
    };

    struct Rotate : public Mousing {
        Rotate(const Orientable::AzEl &azel, const glm::vec2 &position)
                : orientation(azel)
                , position(position) {
        }
        Mode get_mode() const {
            return Mode::rotate;
        }

        static const float angle_scale;
        Orientable::AzEl orientation;
        glm::vec2 position;
    };

    struct Move : public Mousing {
        Move(model::ValueWrapper<glm::vec3> *model,
             PointObject *to_move,
             const glm::vec3 &v)
                : model(model)
                , to_move(to_move)
                , original_position(v) {
            to_move->set_highlight(0.5);
        }
        virtual ~Move() noexcept {
            to_move->set_highlight(0);
        }
        Mode get_mode() const {
            return Mode::move;
        }

        model::ValueWrapper<glm::vec3> *model;
        PointObject *to_move{nullptr};
        glm::vec3 original_position;
    };

    std::unique_ptr<Mousing> mousing;
};

const float SceneRenderer::ContextLifetime::Rotate::angle_scale{0.01};

//----------------------------------------------------------------------------//

SceneRenderer::SceneRenderer(
        const CopyableSceneData &model,
        model::ValueWrapper<std::vector<glm::vec3>> &sources,
        model::ValueWrapper<std::vector<model::ReceiverSettings>> &receivers)
        : model(model)
        , sources(sources)
        , receivers(receivers) {
}

//  defined here so that we can PIMPL the ContextLifetime
SceneRenderer::~SceneRenderer() noexcept = default;

void SceneRenderer::newOpenGLContextCreated() {
    std::lock_guard<std::mutex> lck(mut);
    context_lifetime =
            std::make_unique<ContextLifetime>(*this, model, sources, receivers);
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