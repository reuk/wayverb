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

RaytraceObject::RaytraceObject(const GenericShader &shader,
                               const RaytracerResults &results)
        : shader(shader) {
    auto impulses = results.impulses;
    std::vector<glm::vec3> v(impulses.size() + 1);
    v.front() = to_glm_vec3(results.source);
    std::transform(impulses.begin(), impulses.end(), v.begin() + 1, [](auto i) {
        return to_glm_vec3(i.position);
    });

    std::vector<glm::vec4> c(v.size());
    c.front() = glm::vec4(1, 1, 1, 1);
    std::transform(impulses.begin(), impulses.end(), c.begin() + 1, [](auto i) {
        auto average = std::accumulate(
                           std::begin(i.volume.s), std::end(i.volume.s), 0.0f) /
                       8;
        auto c = i.time ? fabs(average) * 10 : 0;
        return glm::vec4(c, c, c, c);
    });

    std::vector<std::pair<GLuint, GLuint>> lines;
    for (auto ray = 0u; ray != results.rays; ++ray) {
        auto prev = 0u;
        for (auto pt = 0u; pt != results.rays; ++pt) {
            auto index = ray * results.reflections + pt + 1;
            lines.push_back(std::make_pair(prev, index));
            prev = index;
        }
    }

    std::vector<GLuint> indices;
    for (const auto &i : lines) {
        indices.push_back(i.first);
        indices.push_back(i.second);
    }

    size = indices.size();

    geometry.data(v);
    colors.data(c);
    ibo.data(indices);

    //  init vao
    auto s_vao = vao.get_scoped();

    geometry.bind();
    auto v_pos = shader.get_attrib_location("v_position");
    glEnableVertexAttribArray(v_pos);
    glVertexAttribPointer(v_pos, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    colors.bind();
    auto c_pos = shader.get_attrib_location("v_color");
    glEnableVertexAttribArray(c_pos);
    glVertexAttribPointer(c_pos, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

    ibo.bind();
}

void RaytraceObject::draw() const {
    auto s_shader = shader.get_scoped();
    auto s_vao = vao.get_scoped();
    glDrawElements(GL_LINES, size, GL_UNSIGNED_INT, nullptr);
}

//----------------------------------------------------------------------------//

static constexpr auto model_colour = 0.75;

VoxelisedObject::VoxelisedObject(const GenericShader &shader,
                                 const CopyableSceneData &scene_data,
                                 const VoxelCollection &voxel)
        : BasicDrawableObject(
              shader,
              get_vertices(scene_data),
              std::vector<glm::vec4>(
                  scene_data.get_vertices().size(),
                  glm::vec4(
                      model_colour, model_colour, model_colour, model_colour)),
              get_indices(scene_data, voxel),
              GL_TRIANGLES)
        , voxel(voxel) {
}

std::vector<glm::vec3> VoxelisedObject::get_vertices(
    const CopyableSceneData &scene_data) {
    std::vector<glm::vec3> ret(scene_data.get_vertices().size());
    std::transform(scene_data.get_vertices().begin(),
                   scene_data.get_vertices().end(),
                   ret.begin(),
                   [](auto i) { return to_glm_vec3(i); });
    return ret;
}

static void push_triangle_indices(std::vector<GLuint> &ret,
                                  const Triangle &tri) {
    ret.push_back(tri.v0);
    ret.push_back(tri.v1);
    ret.push_back(tri.v2);
}

void fill_indices_vector(const CopyableSceneData &scene_data,
                         const VoxelCollection &voxel,
                         std::vector<GLuint> &ret) {
    for (const auto &x : voxel.get_data()) {
        for (const auto &y : x) {
            for (const auto &z : y) {
                for (auto i : z.get_triangles()) {
                    push_triangle_indices(ret, scene_data.get_triangles()[i]);
                }
            }
        }
    }
}

std::vector<GLuint> VoxelisedObject::get_indices(
    const CopyableSceneData &scene_data, const VoxelCollection &voxel) {
    std::vector<GLuint> ret;
    fill_indices_vector(scene_data, voxel, ret);
    return ret;
}

void VoxelisedObject::draw() const {
    BasicDrawableObject::draw();
    BoxObject box(get_shader());

    for (const auto &x : voxel.get_data()) {
        for (const auto &y : x) {
            for (const auto &z : y) {
                if (!z.get_triangles().empty()) {
                    const auto &aabb = z.get_aabb();
                    box.set_scale(to_glm_vec3(aabb.dimensions()));
                    box.set_position(to_glm_vec3(aabb.centre()));
                    box.draw();
                }
            }
        }
    }
}

//----------------------------------------------------------------------------//

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

std::vector<glm::vec3> MultiMaterialObject::get_vertices(
    const CopyableSceneData &scene_data) const {
    std::vector<glm::vec3> ret(scene_data.get_vertices().size());
    std::transform(scene_data.get_vertices().begin(),
                   scene_data.get_vertices().end(),
                   ret.begin(),
                   [](auto i) { return to_glm_vec3(i); });
    return ret;
}

MultiMaterialObject::MultiMaterialObject(const GenericShader &generic_shader,
                                         const LitSceneShader &lit_scene_shader,
                                         const CopyableSceneData &scene_data)
        : generic_shader(generic_shader)
        , lit_scene_shader(lit_scene_shader) {
    for (auto i = 0; i != scene_data.get_surfaces().size(); ++i) {
        sections.emplace_back(scene_data, i);
    }

    geometry.data(get_vertices(scene_data));
    colors.data(std::vector<glm::vec4>(
        scene_data.get_vertices().size(),
        glm::vec4(model_colour, model_colour, model_colour, 1.0)));

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
        , mic_object(generic_shader, glm::vec4(0, 1, 1, 1)) {
    //  TODO init raytrace object
}

void DrawableScene::MeshContext::clear() {
    mesh_object = nullptr;
    positions.clear();
    pressures.clear();
}

void DrawableScene::update(float dt) {
    if (!mesh_context.positions.empty()) {
        mesh_context.mesh_object =
            std::make_unique<MeshObject>(mesh_shader, mesh_context.positions);
        mesh_context.positions.clear();
    }

    if (!mesh_context.pressures.empty() && mesh_context.mesh_object) {
        mesh_context.mesh_object->set_pressures(mesh_context.pressures);
        mesh_context.pressures.clear();
    }

    if (!rendering) {
        mesh_context.clear();
    }
}

void DrawableScene::draw() const {
    auto draw_thing = [this](const auto &i) {
        if (i) {
            i->draw();
        }
    };

    {
        auto s_shader = generic_shader.get_scoped();
        if (rendering) {
            draw_thing(raytrace_object);
        }

        model_object.draw();

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        source_object.draw();
        mic_object.draw();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    {
        auto s_shader = mesh_shader.get_scoped();

        if (rendering) {
            draw_thing(mesh_context.mesh_object);
        }
    }
}

void DrawableScene::set_mic(const Vec3f &u) {
    mic_object.set_position(to_glm_vec3(u));
}

void DrawableScene::set_source(const Vec3f &u) {
    source_object.set_position(to_glm_vec3(u));
}

void DrawableScene::set_rendering(bool b) {
    rendering = b;
}

void DrawableScene::set_positions(const std::vector<glm::vec3> &p) {
    //  this might be called from the message thread
    //  and OpenGL doesn't like me messing with its shit on other threads
    mesh_context.positions = p;
}

void DrawableScene::set_pressures(const std::vector<float> &p) {
    mesh_context.pressures = p;
}

void DrawableScene::set_highlighted(int u) {
    model_object.set_highlighted(u);
}

void DrawableScene::set_mic_pointing(const std::vector<glm::vec3> &directions) {
    mic_object.set_pointing(directions);
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
    auto max = aabb.dimensions().max();
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

void SceneRenderer::ContextLifetime::set_mic(const Vec3f &u) {
    drawable_scene.set_mic(u);
}
void SceneRenderer::ContextLifetime::set_source(const Vec3f &u) {
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
void SceneRenderer::ContextLifetime::update(float dt) {
    drawable_scene.update(0);
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

void SceneRenderer::ContextLifetime::set_mic_pointing(
    const std::vector<glm::vec3> &directions) {
    drawable_scene.set_mic_pointing(directions);
}

//----------------------------------------------------------------------------//

SceneRenderer::SceneRenderer(const CopyableSceneData &model)
        : model(model) {
}

void SceneRenderer::newOpenGLContextCreated() {
    std::lock_guard<std::mutex> lck(mut);
    context_lifetime = std::make_unique<ContextLifetime>(model);
    context_lifetime->set_emphasis(
        glm::vec3(VisualiserLookAndFeel::emphasis.getFloatRed(),
                  VisualiserLookAndFeel::emphasis.getFloatGreen(),
                  VisualiserLookAndFeel::emphasis.getFloatBlue()));
    listener_list.call(&Listener::newOpenGLContextCreated, this);
}

void SceneRenderer::renderOpenGL() {
    std::lock_guard<std::mutex> lck(mut);
    if (context_lifetime) {
        context_lifetime->update(0);
        context_lifetime->draw();
    }
}

void SceneRenderer::openGLContextClosing() {
    std::lock_guard<std::mutex> lck(mut);
    context_lifetime = nullptr;
    listener_list.call(&Listener::openGLContextClosing, this);
}

void SceneRenderer::addListener(Listener *l) {
    std::lock_guard<std::mutex> lck(mut);
    listener_list.add(l);
}

void SceneRenderer::removeListener(Listener *l) {
    std::lock_guard<std::mutex> lck(mut);
    listener_list.remove(l);
}

void SceneRenderer::set_aspect(float aspect) {
    std::lock_guard<std::mutex> lck(mut);
    if (context_lifetime) {
        context_lifetime->set_aspect(aspect);
    }
}

void SceneRenderer::update_scale(float delta) {
    std::lock_guard<std::mutex> lck(mut);
    if (context_lifetime) {
        context_lifetime->update_scale(delta);
    }
}

void SceneRenderer::set_rotation(float azimuth, float elevation) {
    std::lock_guard<std::mutex> lck(mut);
    if (context_lifetime) {
        context_lifetime->set_rotation(azimuth, elevation);
    }
}

void SceneRenderer::set_rendering(bool b) {
    std::lock_guard<std::mutex> lck(mut);
    if (context_lifetime) {
        context_lifetime->set_rendering(b);
    }
}

void SceneRenderer::set_mic(const Vec3f &u) {
    std::lock_guard<std::mutex> lck(mut);
    if (context_lifetime) {
        context_lifetime->set_mic(u);
    }
}

void SceneRenderer::set_source(const Vec3f &u) {
    std::lock_guard<std::mutex> lck(mut);
    if (context_lifetime) {
        context_lifetime->set_source(u);
    }
}

void SceneRenderer::set_positions(const std::vector<cl_float3> &positions) {
    std::lock_guard<std::mutex> lck(mut);
    if (context_lifetime) {
        context_lifetime->set_positions(positions);
    }
}

void SceneRenderer::set_pressures(const std::vector<float> &pressures) {
    std::lock_guard<std::mutex> lck(mut);
    if (context_lifetime) {
        context_lifetime->set_pressures(pressures);
    }
}

void SceneRenderer::set_highlighted(int u) {
    std::lock_guard<std::mutex> lck(mut);
    if (context_lifetime) {
        context_lifetime->set_highlighted(u);
    }
}

void SceneRenderer::set_emphasis(const glm::vec3 &u) {
    std::lock_guard<std::mutex> lck(mut);
    if (context_lifetime) {
        context_lifetime->set_emphasis(u);
    }
}

void SceneRenderer::set_mic_pointing(const std::vector<glm::vec3> &directions) {
    std::lock_guard<std::mutex> lck(mut);
    if (context_lifetime) {
        context_lifetime->set_mic_pointing(directions);
    }
}
