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
        , source_object(generic_shader, glm::vec4(1, 0, 0, 1))
        , receiver_object(generic_shader, glm::vec4(0, 1, 1, 1)) {
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

//----------------------------------------------------------------------------//

SceneRenderer::ContextLifetime::ContextLifetime(
    const CopyableSceneData &scene_data)
        : model(scene_data)
        , drawable_scene(generic_shader, mesh_shader, lit_scene_shader, model)
        , axes(generic_shader)
        , projection_matrix(get_projection_matrix(1)) {
    auto aabb = model.get_aabb();
    auto m = aabb.centre();
    auto max = glm::length(aabb.dimensions());
    scale = max > 0 ? 20 / max : 1;
    translation = glm::translate(-glm::vec3(m.x, m.y, m.z));
}

void SceneRenderer::ContextLifetime::set_aspect(float aspect) {
    projection_matrix = get_projection_matrix(aspect);
}
void SceneRenderer::ContextLifetime::update_scale(float delta) {
    scale = std::max(0.0f, scale + delta);
}
void SceneRenderer::ContextLifetime::set_rotation(float azimuth,
                                                  float elevation) {
    auto i = glm::rotate(azimuth, glm::vec3(0, 1, 0));
    auto j = glm::rotate(elevation, glm::vec3(1, 0, 0));
    rotation = j * i;
}

void SceneRenderer::ContextLifetime::set_rendering(bool b) {
    drawable_scene.set_rendering(b);
}

void SceneRenderer::ContextLifetime::set_receiver(const glm::vec3 &u) {
    drawable_scene.set_receiver(u);
}
void SceneRenderer::ContextLifetime::set_source(const glm::vec3 &u) {
    drawable_scene.set_source(u);
}

void SceneRenderer::ContextLifetime::set_positions(
    const std::vector<cl_float3> &positions) {
    std::vector<glm::vec3> ret(positions.size());
    proc::transform(
        positions, ret.begin(), [](const auto &i) { return to_glm_vec3(i); });
    drawable_scene.set_positions(ret);
}
void SceneRenderer::ContextLifetime::set_pressures(
    const std::vector<float> &pressures) {
    drawable_scene.set_pressures(pressures);
}

void SceneRenderer::ContextLifetime::set_highlighted(int u) {
    drawable_scene.set_highlighted(u);
}

void SceneRenderer::ContextLifetime::set_emphasis(const glm::vec3 &c) {
    drawable_scene.set_emphasis(c);
}

void SceneRenderer::ContextLifetime::draw() const {
    auto c = 0.0;
    glClearColor(c, c, c, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

glm::mat4 SceneRenderer::ContextLifetime::get_projection_matrix() const {
    return projection_matrix;
}
glm::mat4 SceneRenderer::ContextLifetime::get_view_matrix() const {
    auto rad = 20;
    glm::vec3 eye(0, 0, rad);
    glm::vec3 target(0, 0, 0);
    glm::vec3 up(0, 1, 0);
    auto mm = rotation * get_scale_matrix() * translation;
    return glm::lookAt(eye, target, up) * mm;
}
glm::mat4 SceneRenderer::ContextLifetime::get_scale_matrix() const {
    return glm::scale(glm::vec3(scale, scale, scale));
}

glm::mat4 SceneRenderer::ContextLifetime::get_projection_matrix(float aspect) {
    return glm::perspective(45.0f, aspect, 0.05f, 1000.0f);
}

void SceneRenderer::ContextLifetime::set_receiver_pointing(
    const std::vector<glm::vec3> &directions) {
    drawable_scene.set_receiver_pointing(directions);
}

//----------------------------------------------------------------------------//

SceneRenderer::SceneRenderer(const CopyableSceneData &model)
        : WorkItemOwner(*this)
        , model(model) {
}

void SceneRenderer::newOpenGLContextCreated() {
    context_lifetime = std::make_unique<ContextLifetime>(model);
    context_lifetime->set_emphasis(
        glm::vec3(VisualiserLookAndFeel::emphasis.getFloatRed(),
                  VisualiserLookAndFeel::emphasis.getFloatGreen(),
                  VisualiserLookAndFeel::emphasis.getFloatBlue()));
    sendChangeMessage();
}

void SceneRenderer::renderOpenGL() {
    pop_all();
    context_lifetime->draw();
}

void SceneRenderer::openGLContextClosing() {
    sendChangeMessage();
    context_lifetime = nullptr;
}

void SceneRenderer::set_aspect(float aspect) {
    push([aspect](auto &i) { i.context_lifetime->set_aspect(aspect); });
}

void SceneRenderer::update_scale(float delta) {
    push([delta](auto &i) { i.context_lifetime->update_scale(delta); });
}

void SceneRenderer::set_rotation(float azimuth, float elevation) {
    push([azimuth, elevation](auto &i) {
        i.context_lifetime->set_rotation(azimuth, elevation);
    });
}

void SceneRenderer::set_rendering(bool b) {
    push([b](auto &i) { i.context_lifetime->set_rendering(b); });
}

void SceneRenderer::set_receiver(const glm::vec3 &u) {
    push([u](auto &i) { i.context_lifetime->set_receiver(u); });
}

void SceneRenderer::set_source(const glm::vec3 &u) {
    push([u](auto &i) { i.context_lifetime->set_source(u); });
}

void SceneRenderer::set_positions(const std::vector<cl_float3> &positions) {
    push(
        [positions](auto &i) { i.context_lifetime->set_positions(positions); });
}

void SceneRenderer::set_pressures(const std::vector<float> &pressures) {
    push(
        [pressures](auto &i) { i.context_lifetime->set_pressures(pressures); });
}

void SceneRenderer::set_highlighted(int u) {
    push([u](auto &i) { i.context_lifetime->set_highlighted(u); });
}

void SceneRenderer::set_emphasis(const glm::vec3 &u) {
    push([u](auto &i) { i.context_lifetime->set_emphasis(u); });
}

void SceneRenderer::set_receiver_pointing(
    const std::vector<glm::vec3> &directions) {
    push([directions](auto &i) {
        i.context_lifetime->set_receiver_pointing(directions);
    });
}