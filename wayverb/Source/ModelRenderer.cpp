#include "ModelRenderer.hpp"
#include "VisualiserLookAndFeel.hpp"

#include "MoreConversions.hpp"

#include "common/azimuth_elevation.h"
#include "common/boundaries.h"
#include "common/cl_common.h"
#include "common/conversions.h"
#include "common/json_read_write.h"
#include "common/kernel.h"

#include "combined/config_serialize.h"

//  message thread
//  on mouse drag
//  locks scene renderer
//  tries to push mouse-down command for gl thread to do later
//  gl thread
// on draw
//  locks gl command queue
//  pops mouse-drag command
//  tries to tell scene renderer to broadcast new info

template <class T, class Compare>
static constexpr const T &clamp(const T &v,
                                const T &lo,
                                const T &hi,
                                Compare comp) {
    return comp(v, hi) ? std::max(v, lo, comp) : std::min(v, hi, comp);
}

template <class T>
static constexpr const T &clamp(const T &v, const T &lo, const T &hi) {
    return clamp(v, lo, hi, std::less<>());
}

static void push_triangle_indices(std::vector<GLuint> &ret,
                                  const Triangle &tri) {
    ret.push_back(tri.v0);
    ret.push_back(tri.v1);
    ret.push_back(tri.v2);
}

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

void MultiMaterialObject::SingleMaterialSection::draw() const {
    ibo.bind();
    glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, nullptr);
}

MultiMaterialObject::MultiMaterialObject(const GenericShader &generic_shader,
                                         const LitSceneShader &lit_scene_shader,
                                         const CopyableSceneData &scene_data)
        : generic_shader(generic_shader)
        , lit_scene_shader(lit_scene_shader) {
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

void MultiMaterialObject::draw() const {
    for (auto i = 0u; i != sections.size(); ++i) {
        if (i == highlighted) {
            auto s_shader = lit_scene_shader.get_scoped();
            auto s_vao = fill_vao.get_scoped();
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            sections[i].draw();
        } else {
            auto s_shader = generic_shader.get_scoped();
            auto s_vao = wire_vao.get_scoped();
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            sections[i].draw();
        }
    }
}

void MultiMaterialObject::set_highlighted(int material) {
    highlighted = material;
}

void MultiMaterialObject::set_colour(const glm::vec3 &c) {
    auto s = lit_scene_shader.get_scoped();
    lit_scene_shader.set_colour(c);
}

//----------------------------------------------------------------------------//

DrawableScene::DrawableScene(const GenericShader &generic_shader,
                             const MeshShader &mesh_shader,
                             const LitSceneShader &lit_scene_shader,
                             const CopyableSceneData &scene_data)
        : generic_shader(generic_shader)
        , mesh_shader(mesh_shader)
        , lit_scene_shader(lit_scene_shader)
        , model_object(generic_shader, lit_scene_shader, scene_data)
        , source_object(generic_shader, glm::vec4(0.7, 0, 0, 1))
        , receiver_object(generic_shader, glm::vec4(0, 0.7, 0.7, 1)) {
    //  TODO init raytrace object
}

void DrawableScene::draw() const {
    model_object.draw();

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    source_object.draw();
    receiver_object.draw();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if (rendering && mesh_object) {
        mesh_object->draw();
    }
}

void DrawableScene::set_receiver(const glm::vec3 &u) {
    receiver_object.set_position(u);
}

void DrawableScene::set_source(const glm::vec3 &u) {
    source_object.set_position(u);
}

void DrawableScene::set_rendering(bool b) {
    rendering = b;
}

void DrawableScene::set_positions(const std::vector<glm::vec3> &p) {
    mesh_object = std::make_unique<MeshObject>(mesh_shader, p);
}

void DrawableScene::set_pressures(const std::vector<float> &p) {
    mesh_object->set_pressures(p);
}

void DrawableScene::set_highlighted(int u) {
    model_object.set_highlighted(u);
}

void DrawableScene::set_receiver_pointing(
    const std::vector<glm::vec3> &directions) {
    receiver_object.set_pointing(directions);
}

void DrawableScene::set_emphasis(const glm::vec3 &c) {
    model_object.set_colour(c);
}

std::vector<Node *> DrawableScene::get_selectable_objects() {
    return {&source_object, &receiver_object};
}

Node *DrawableScene::get_source() {
    return &source_object;
}
Node *DrawableScene::get_receiver() {
    return &receiver_object;
}

//----------------------------------------------------------------------------//

class SceneRenderer::ContextLifetime : public ::Drawable {
public:
    ContextLifetime(SceneRenderer &owner, const CopyableSceneData &scene_data)
            : owner(owner)
            , model(scene_data)
            , drawable_scene(
                  generic_shader, mesh_shader, lit_scene_shader, model)
            , axes(generic_shader) {
        auto aabb = model.get_aabb();
        auto m = aabb.centre();
        auto max = glm::length(aabb.dimensions());
        scale = max > 0 ? 20 / max : 1;
        translation = -glm::vec3(m.x, m.y, m.z);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_PROGRAM_POINT_SIZE);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void set_viewport(const glm::ivec2 &v) {
        viewport = v;
    }

    void set_scale(float u) {
        set_scale_impl(u);
    }

    void set_rotation(const Orientable::AzEl &u) {
        set_rotation_impl(u);
    }

    void set_rendering(bool b) {
        drawable_scene.set_rendering(b);
    }

    void set_receiver(const glm::vec3 &u) {
        drawable_scene.set_receiver(u);
    }

    void set_source(const glm::vec3 &u) {
        drawable_scene.set_source(u);
    }

    void set_positions(const std::vector<cl_float3> &positions) {
        std::vector<glm::vec3> ret(positions.size());
        proc::transform(positions, ret.begin(), [](const auto &i) {
            return to_glm_vec3(i);
        });
        drawable_scene.set_positions(ret);
    }
    void set_pressures(const std::vector<float> &pressures) {
        drawable_scene.set_pressures(pressures);
    }

    void set_highlighted(int u) {
        drawable_scene.set_highlighted(u);
    }

    void set_emphasis(const glm::vec3 &c) {
        drawable_scene.set_emphasis(c);
    }

    void draw() const {
        auto c = 0.0;
        glClearColor(c, c, c, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto config_shader = [this](const auto &shader) {
            auto s_shader = shader.get_scoped();
            shader.set_model_matrix(glm::mat4());
            shader.set_view_matrix(get_view_matrix());
            shader.set_projection_matrix(get_projection_matrix());
        };

        config_shader(generic_shader);
        config_shader(mesh_shader);
        config_shader(lit_scene_shader);

        drawable_scene.draw();
        axes.draw();
    }

    void set_receiver_pointing(const std::vector<glm::vec3> &directions) {
        drawable_scene.set_receiver_pointing(directions);
    }

    //  matrices:
    //  projection * view * model

    struct Ray {
        glm::vec3 origin;
        glm::vec3 direction;
    };

    void mouse_down(const glm::vec2 &pos) {
        auto hovered = get_currently_hovered(pos);
        mousing =
            hovered
                ? std::unique_ptr<Mousing>(
                      std::make_unique<Move>(hovered, hovered->get_position()))
                : std::unique_ptr<Mousing>(std::make_unique<Rotate>(azel, pos));
    }

    void mouse_drag(const glm::vec2 &pos) {
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

                if (m.to_move == drawable_scene.get_source()) {
                    owner.broadcast_source_position(new_pos);
                } else if (m.to_move == drawable_scene.get_receiver()) {
                    owner.broadcast_receiver_position(new_pos);
                }

                break;
            }
            case Mousing::Mode::rotate: {
                const auto &m = dynamic_cast<Rotate &>(*mousing);
                auto diff = pos - m.position;
                set_rotation_impl(Orientable::AzEl{
                    m.orientation.azimuth + diff.x * Rotate::angle_scale,
                    clamp(
                        m.orientation.elevation + diff.y * Rotate::angle_scale,
                        static_cast<float>(-M_PI / 2),
                        static_cast<float>(M_PI / 2))});
                break;
            }
        }
    }

    void mouse_up(const glm::vec2 &pos) {
        mousing = nullptr;
    }

    void mouse_wheel_move(float delta_y) {
        //  TODO tween this
        set_scale_impl(scale + delta_y);
    }

private:
    void set_rotation_impl(const Orientable::AzEl &u) {
        azel = Orientable::AzEl{
            std::fmod(u.azimuth, static_cast<float>(M_PI * 2)), u.elevation};
    }

    void set_scale_impl(float u) {
        scale = std::max(0.0f, u);
    }

    auto get_aspect() const {
        return viewport.x / static_cast<float>(viewport.y);
    }

    glm::vec3 get_world_camera_position() const {
        return glm::inverse(get_view_matrix())[3];
    }

    glm::vec3 get_world_camera_direction() const {
        return glm::normalize(glm::vec3{glm::inverse(get_view_matrix()) *
                                        glm::vec4{0, 0, -1, 0}});
    }

    glm::vec3 get_world_mouse_direction(const glm::vec2 &pos) const {
        auto ray_clip = glm::vec4{
            (2 * pos.x) / viewport.x - 1, 1 - (2 * pos.y) / viewport.y, -1, 1};
        auto ray_eye = glm::inverse(get_projection_matrix()) * ray_clip;
        ray_eye = glm::vec4{ray_eye.x, ray_eye.y, -1, 0};
        return glm::normalize(
            glm::vec3{glm::inverse(get_view_matrix()) * ray_eye});
    }

    Node *get_currently_hovered(const glm::vec2 &pos) {
        auto origin = get_world_camera_position();
        auto direction = get_world_mouse_direction(pos);

        struct Intersection {
            Node *ref;
            double distance;
        };

        Intersection intersection{nullptr, 0};
        for (auto i : drawable_scene.get_selectable_objects()) {
            auto diff = origin - i->get_position();
            auto b = glm::dot(direction, diff);
            auto c = glm::dot(diff, diff) - glm::pow(i->get_scale() * 0.4, 2);
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

    glm::mat4 get_rotation_matrix() const {
        auto i = glm::rotate(azel.azimuth, glm::vec3(0, 1, 0));
        auto j = glm::rotate(azel.elevation, glm::vec3(1, 0, 0));
        return j * i;
    }

    glm::mat4 get_scale_matrix() const {
        return glm::scale(glm::vec3(scale));
    }

    glm::mat4 get_translation_matrix() const {
        return glm::translate(translation);
    }

    glm::mat4 get_view_matrix() const {
        auto rad = 20;
        glm::vec3 eye(0, 0, rad);
        glm::vec3 target(0, 0, 0);
        glm::vec3 up(0, 1, 0);
        return glm::lookAt(eye, target, up) * get_rotation_matrix() *
               get_scale_matrix() * get_translation_matrix();
    }

    SceneRenderer &owner;
    const CopyableSceneData &model;

    GenericShader generic_shader;
    MeshShader mesh_shader;
    LitSceneShader lit_scene_shader;
    DrawableScene drawable_scene;
    AxesObject axes;

    glm::ivec2 viewport;
    Orientable::AzEl azel;
    float scale;
    glm::vec3 translation;

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
        Move(Node *to_move, const glm::vec3 &v)
                : to_move(to_move)
                , original_position(v) {
            to_move->set_highlight(0.5);
        }
        virtual ~Move() noexcept {
            to_move->set_highlight(0);
        }
        Mode get_mode() const {
            return Mode::move;
        }
        Node *to_move{nullptr};
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
        glm::vec3(VisualiserLookAndFeel::emphasis.getFloatRed(),
                  VisualiserLookAndFeel::emphasis.getFloatGreen(),
                  VisualiserLookAndFeel::emphasis.getFloatBlue()));
    outgoing_work_queue.push([this] { sendChangeMessage(); });
}

void SceneRenderer::renderOpenGL() {
    std::lock_guard<std::mutex> lck(mut);
    while (!work_queue.empty()) {
        auto method = work_queue.pop_one();
        if (method) {
            (*method)();
        };
    }
    context_lifetime->draw();
}

void SceneRenderer::openGLContextClosing() {
    std::lock_guard<std::mutex> lck(mut);
    outgoing_work_queue.push([this] { sendChangeMessage(); });
    context_lifetime = nullptr;
}

void SceneRenderer::set_viewport(const glm::ivec2 &v) {
    std::lock_guard<std::mutex> lck(mut);
    work_queue.push([this, v] { context_lifetime->set_viewport(v); });
}

void SceneRenderer::set_scale(float u) {
    std::lock_guard<std::mutex> lck(mut);
    work_queue.push([this, u] { context_lifetime->set_scale(u); });
}

void SceneRenderer::set_rotation(const Orientable::AzEl &u) {
    std::lock_guard<std::mutex> lck(mut);
    work_queue.push([this, u] { context_lifetime->set_rotation(u); });
}

void SceneRenderer::set_rendering(bool b) {
    std::lock_guard<std::mutex> lck(mut);
    work_queue.push([this, b] { context_lifetime->set_rendering(b); });
}

void SceneRenderer::set_receiver(const glm::vec3 &u) {
    std::lock_guard<std::mutex> lck(mut);
    work_queue.push([this, u] { context_lifetime->set_receiver(u); });
}

void SceneRenderer::set_source(const glm::vec3 &u) {
    std::lock_guard<std::mutex> lck(mut);
    work_queue.push([this, u] { context_lifetime->set_source(u); });
}

void SceneRenderer::set_positions(const std::vector<cl_float3> &positions) {
    std::lock_guard<std::mutex> lck(mut);
    work_queue.push(
        [this, positions] { context_lifetime->set_positions(positions); });
}

void SceneRenderer::set_pressures(const std::vector<float> &pressures) {
    std::lock_guard<std::mutex> lck(mut);
    work_queue.push(
        [this, pressures] { context_lifetime->set_pressures(pressures); });
}

void SceneRenderer::set_highlighted(int u) {
    std::lock_guard<std::mutex> lck(mut);
    work_queue.push([this, u] { context_lifetime->set_highlighted(u); });
}

void SceneRenderer::set_emphasis(const glm::vec3 &u) {
    std::lock_guard<std::mutex> lck(mut);
    work_queue.push([this, u] { context_lifetime->set_emphasis(u); });
}

void SceneRenderer::set_receiver_pointing(
    const std::vector<glm::vec3> &directions) {
    std::lock_guard<std::mutex> lck(mut);
    work_queue.push([this, directions] {
        context_lifetime->set_receiver_pointing(directions);
    });
}

void SceneRenderer::mouse_down(const glm::vec2 &u) {
    std::lock_guard<std::mutex> lck(mut);
    work_queue.push([this, u] { context_lifetime->mouse_down(u); });
}

void SceneRenderer::mouse_drag(const glm::vec2 &u) {
    std::lock_guard<std::mutex> lck(mut);
    work_queue.push([this, u] { context_lifetime->mouse_drag(u); });
}

void SceneRenderer::mouse_up(const glm::vec2 &u) {
    std::lock_guard<std::mutex> lck(mut);
    work_queue.push([this, u] { context_lifetime->mouse_up(u); });
}

void SceneRenderer::mouse_wheel_move(float u) {
    std::lock_guard<std::mutex> lck(mut);
    work_queue.push([this, u] { context_lifetime->mouse_wheel_move(u); });
}

void SceneRenderer::broadcast_receiver_position(const glm::vec3 &pos) {
    outgoing_work_queue.push([this, pos] {
        listener_list.call(&Listener::receiver_dragged, this, pos);
    });
    triggerAsyncUpdate();
}

void SceneRenderer::broadcast_source_position(const glm::vec3 &pos) {
    outgoing_work_queue.push([this, pos] {
        listener_list.call(&Listener::source_dragged, this, pos);
    });
    triggerAsyncUpdate();
}

void SceneRenderer::addListener(Listener *l) {
    std::lock_guard<std::mutex> lck(mut);
    listener_list.add(l);
}
void SceneRenderer::removeListener(Listener *l) {
    std::lock_guard<std::mutex> lck(mut);
    listener_list.remove(l);
}

void SceneRenderer::handleAsyncUpdate() {
    while (!outgoing_work_queue.empty()) {
        auto method = outgoing_work_queue.pop_one();
        if (method) {
            (*method)();
        };
    }
}