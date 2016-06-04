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

//----------------------------------------------------------------------------//

class SceneRenderer::ContextLifetime : public ::Drawable {
public:
    ContextLifetime(const CopyableSceneData &scene_data)
            : model(scene_data)
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

    auto get_aspect() const {
        return viewport.x / static_cast<float>(viewport.y);
    }

    void set_scale(float u) {
        scale = std::max(0.0f, u);
    }

    auto get_scale() const {
        return scale;
    }

    void set_rotation(const Orientable::AzEl &u) {
        azel = Orientable::AzEl{
            std::fmod(u.azimuth, static_cast<float>(M_PI * 2)), u.elevation};
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

    Node *get_currently_hovered(const glm::vec2 &pos) {
        auto ray_clip = glm::vec4{
            (2 * pos.x) / viewport.x - 1, 1 - (2 * pos.y) / viewport.y, -1, 1};
        auto ray_eye = glm::inverse(get_projection_matrix()) * ray_clip;
        ray_eye = glm::vec4{ray_eye.x, ray_eye.y, -1, 0};

        auto camera_world = glm::vec3{glm::inverse(get_view_matrix())[3]};

        auto ray_world = glm::normalize(
            glm::vec3{glm::inverse(get_view_matrix()) * ray_eye});

        struct Intersection {
            Node *ref;
            double distance;
        };

        Intersection intersection{nullptr, 0};
        for (auto i : drawable_scene.get_selectable_objects()) {
            auto diff = camera_world - i->get_position();
            auto b = glm::dot(ray_world, diff);
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

    void mouse_down(const glm::vec2 &pos) {
        mousing = std::make_unique<Mousing>(
            Mousing{get_currently_hovered(pos), pos, azel});
        if (mousing->to_move) {
            mousing->to_move->set_highlight(0.5);
        }
    }

    void mouse_drag(const glm::vec2 &pos) {
        assert(mousing);
        if (mousing->to_move) {
            //  if we're dragging a source/receiver, we're going to move that

        } else {
            //  otherwise, we want to rotate the scene
            auto diff = pos - mousing->begin;

            set_rotation(
                Orientable::AzEl{mousing->begin_orientation.azimuth +
                                     diff.x * Mousing::angle_scale,
                                 clamp(mousing->begin_orientation.elevation +
                                           diff.y * Mousing::angle_scale,
                                       static_cast<float>(-M_PI / 2),
                                       static_cast<float>(M_PI / 2))});
        }
    }

    void mouse_up(const glm::vec2 &pos) {
        //  clear all the mousing state
        if (mousing->to_move) {
            mousing->to_move->set_highlight(0);
        }
        mousing = nullptr;
    }

    void mouse_wheel_move(float delta_y) {
        //  TODO tween this
        set_scale(get_scale() + delta_y);
    }

private:
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
        static const float angle_scale;

        Node *to_move{nullptr};
        glm::vec2 begin;
        Orientable::AzEl begin_orientation;
    };
    std::unique_ptr<Mousing> mousing;
};

const float SceneRenderer::ContextLifetime::Mousing::angle_scale{0.01};

//----------------------------------------------------------------------------//

SceneRenderer::SceneRenderer(const CopyableSceneData &model)
        : work_queue(*this)
        , model(model) {
}

//  defined here so that we can PIMPL the ContextLifetime
SceneRenderer::~SceneRenderer() noexcept = default;

void SceneRenderer::newOpenGLContextCreated() {
    context_lifetime = std::make_unique<ContextLifetime>(model);
    context_lifetime->set_emphasis(
        glm::vec3(VisualiserLookAndFeel::emphasis.getFloatRed(),
                  VisualiserLookAndFeel::emphasis.getFloatGreen(),
                  VisualiserLookAndFeel::emphasis.getFloatBlue()));
    sendChangeMessage();
}

void SceneRenderer::renderOpenGL() {
    work_queue.pop_all();
    context_lifetime->draw();
}

void SceneRenderer::openGLContextClosing() {
    sendChangeMessage();
    context_lifetime = nullptr;
}

void SceneRenderer::set_viewport(const glm::ivec2 &v) {
    work_queue.push([v](auto &i) { i.context_lifetime->set_viewport(v); });
}

void SceneRenderer::set_scale(float u) {
    work_queue.push([u](auto &i) { i.context_lifetime->set_scale(u); });
}

void SceneRenderer::set_rotation(const Orientable::AzEl &u) {
    work_queue.push([u](auto &i) { i.context_lifetime->set_rotation(u); });
}

void SceneRenderer::set_rendering(bool b) {
    work_queue.push([b](auto &i) { i.context_lifetime->set_rendering(b); });
}

void SceneRenderer::set_receiver(const glm::vec3 &u) {
    work_queue.push([u](auto &i) { i.context_lifetime->set_receiver(u); });
}

void SceneRenderer::set_source(const glm::vec3 &u) {
    work_queue.push([u](auto &i) { i.context_lifetime->set_source(u); });
}

void SceneRenderer::set_positions(const std::vector<cl_float3> &positions) {
    work_queue.push(
        [positions](auto &i) { i.context_lifetime->set_positions(positions); });
}

void SceneRenderer::set_pressures(const std::vector<float> &pressures) {
    work_queue.push(
        [pressures](auto &i) { i.context_lifetime->set_pressures(pressures); });
}

void SceneRenderer::set_highlighted(int u) {
    work_queue.push([u](auto &i) { i.context_lifetime->set_highlighted(u); });
}

void SceneRenderer::set_emphasis(const glm::vec3 &u) {
    work_queue.push([u](auto &i) { i.context_lifetime->set_emphasis(u); });
}

void SceneRenderer::set_receiver_pointing(
    const std::vector<glm::vec3> &directions) {
    work_queue.push([directions](auto &i) {
        i.context_lifetime->set_receiver_pointing(directions);
    });
}

void SceneRenderer::mouse_down(const glm::vec2 &pos) {
    work_queue.push([pos](auto &i) { i.context_lifetime->mouse_down(pos); });
}

void SceneRenderer::mouse_drag(const glm::vec2 &pos) {
    work_queue.push([pos](auto &i) { i.context_lifetime->mouse_drag(pos); });
}

void SceneRenderer::mouse_up(const glm::vec2 &pos) {
    work_queue.push([pos](auto &i) { i.context_lifetime->mouse_up(pos); });
}

void SceneRenderer::mouse_wheel_move(float delta_y) {
    work_queue.push(
        [delta_y](auto &i) { i.context_lifetime->mouse_wheel_move(delta_y); });
}