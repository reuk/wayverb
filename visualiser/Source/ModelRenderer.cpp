#include "ModelRenderer.hpp"

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
    shader.set_black(false);

    auto s_vao = vao.get_scoped();
    glDrawElements(GL_LINES, size, GL_UNSIGNED_INT, nullptr);
}

//----------------------------------------------------------------------------//

static constexpr auto model_colour = 0.75;

VoxelisedObject::VoxelisedObject(const GenericShader &shader,
                                 const SceneData &scene_data,
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
    const SceneData &scene_data) const {
    std::vector<glm::vec3> ret(scene_data.get_vertices().size());
    std::transform(scene_data.get_vertices().begin(),
                   scene_data.get_vertices().end(),
                   ret.begin(),
                   [](auto i) { return to_glm_vec3(i); });
    return ret;
}

void fill_indices_vector(const SceneData &scene_data,
                         const VoxelCollection &voxel,
                         std::vector<GLuint> &ret) {
    for (const auto &x : voxel.get_data()) {
        for (const auto &y : x) {
            for (const auto &z : y) {
                for (const auto &i : z.get_triangles()) {
                    const auto &tri = scene_data.get_triangles()[i];
                    ret.push_back(tri.v0);
                    ret.push_back(tri.v1);
                    ret.push_back(tri.v2);
                }
            }
        }
    }
}

std::vector<GLuint> VoxelisedObject::get_indices(
    const SceneData &scene_data, const VoxelCollection &voxel) const {
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

DrawableScene::DrawableScene(const GenericShader &generic_shader,
                             const MeshShader &mesh_shader,
                             const SceneData &scene_data)
        : generic_shader(generic_shader)
        , mesh_shader(mesh_shader)
        , model_object{std::make_unique<VoxelisedObject>(
              generic_shader, scene_data, VoxelCollection(scene_data, 4, 0.1))}
        , source_object{std::make_unique<OctahedronObject>(
              generic_shader, glm::vec3(0, 0, 0), glm::vec4(1, 0, 0, 1))}
        , mic_object{std::make_unique<OctahedronObject>(
              generic_shader, glm::vec3(0, 0, 0), glm::vec4(0, 1, 1, 1))} {
    //  TODO init raytrace object
}

void DrawableScene::update(float dt) {
    std::lock_guard<std::mutex> lck(mut);

    if (!positions.empty()) {
        mesh_object = std::make_unique<MeshObject>(mesh_shader, positions);
        positions.clear();
    }

    if (!pressures.empty() && mesh_object) {
        mesh_object->set_pressures(pressures);
        pressures.clear();
    }
}

void DrawableScene::draw() const {
    std::lock_guard<std::mutex> lck(mut);

    auto draw_thing = [this](const auto &i) {
        if (i) {
            i->draw();
        }
    };

    {
        auto s_shader = generic_shader.get_scoped();
        if (rendering) {
            if (raytracer_enabled) {
                draw_thing(raytrace_object);
            }
        }

        if (model_object) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            model_object->draw();

            draw_thing(source_object);
            draw_thing(mic_object);

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }

    {
        auto s_shader = mesh_shader.get_scoped();

        if (rendering) {
            if (waveguide_enabled) {
                draw_thing(mesh_object);
            }
        }
    }
}

void DrawableScene::set_mic(const Vec3f &u) {
    std::lock_guard<std::mutex> lck(mut);
    mic_object->set_position(to_glm_vec3(u));
}

void DrawableScene::set_source(const Vec3f &u) {
    std::lock_guard<std::mutex> lck(mut);
    source_object->set_position(to_glm_vec3(u));
}

void DrawableScene::set_waveguide_enabled(bool b) {
    std::lock_guard<std::mutex> lck(mut);
    waveguide_enabled = b;
}

void DrawableScene::set_raytracer_enabled(bool b) {
    std::lock_guard<std::mutex> lck(mut);
    raytracer_enabled = b;
}

void DrawableScene::set_rendering(bool b) {
    std::lock_guard<std::mutex> lck(mut);
    rendering = b;

    if (!b) {
        mesh_object = nullptr;
        positions.clear();
        pressures.clear();
    }
}

void DrawableScene::set_positions(const std::vector<glm::vec3> &p) {
    //  this might be called from the message thread
    //  and OpenGL doesn't like me messing with its shit on other threads
    std::lock_guard<std::mutex> lck(mut);
    positions = p;
}

void DrawableScene::set_pressures(const std::vector<float> &p) {
    std::lock_guard<std::mutex> lck(mut);
    pressures = p;
}

//----------------------------------------------------------------------------//

SceneRenderer::SceneRenderer(const SceneData &model)
        : model(model)
        , projection_matrix(get_projection_matrix(1)) {
}

void SceneRenderer::newOpenGLContextCreated() {
    std::lock_guard<std::mutex> lck(mut);
    generic_shader = std::make_unique<GenericShader>();
    mesh_shader = std::make_unique<MeshShader>();
    drawable_scene =
        std::make_unique<DrawableScene>(*generic_shader, *mesh_shader, model);
    axes = std::make_unique<AxesObject>(*generic_shader);

    auto aabb = model.get_aabb();
    auto m = aabb.centre();
    auto max = aabb.dimensions().max();
    scale = max > 0 ? 20 / max : 1;
    translation = glm::translate(-glm::vec3(m.x, m.y, m.z));

    listener_list.call(&Listener::newOpenGLContextCreated, this);
}

void SceneRenderer::renderOpenGL() {
    std::lock_guard<std::mutex> lck(mut);

    update();
    draw();
}

void SceneRenderer::openGLContextClosing() {
    std::lock_guard<std::mutex> lck(mut);
    drawable_scene = nullptr;
    generic_shader = nullptr;
    mesh_shader = nullptr;

    listener_list.call(&Listener::openGLContextClosing, this);
}

glm::mat4 SceneRenderer::get_projection_matrix(float aspect) {
    return glm::perspective(45.0f, aspect, 0.05f, 1000.0f);
}

void SceneRenderer::set_aspect(float aspect) {
    std::lock_guard<std::mutex> lck(mut);
    projection_matrix = get_projection_matrix(aspect);
}

void SceneRenderer::update() {
    if (drawable_scene) {
        drawable_scene->update(0);
    }
}

void SceneRenderer::draw() const {
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

    config_shader(*generic_shader);
    config_shader(*mesh_shader);

    auto draw_thing = [this](const auto &i) {
        if (i) {
            i->draw();
        }
    };

    draw_thing(drawable_scene);
    draw_thing(axes);
}

glm::mat4 SceneRenderer::get_projection_matrix() const {
    return projection_matrix;
}
glm::mat4 SceneRenderer::get_view_matrix() const {
    auto rad = 20;
    glm::vec3 eye(0, 0, rad);
    glm::vec3 target(0, 0, 0);
    glm::vec3 up(0, 1, 0);
    auto mm = rotation * get_scale_matrix() * translation;
    return glm::lookAt(eye, target, up) * mm;
}

void SceneRenderer::set_rotation(float az, float el) {
    std::lock_guard<std::mutex> lck(mut);
    auto i = glm::rotate(az, glm::vec3(0, 1, 0));
    auto j = glm::rotate(el, glm::vec3(1, 0, 0));
    rotation = j * i;
}

void SceneRenderer::update_scale(float delta) {
    std::lock_guard<std::mutex> lck(mut);
    scale = std::max(0.0f, scale + delta);
}

glm::mat4 SceneRenderer::get_scale_matrix() const {
    return glm::scale(glm::vec3(scale, scale, scale));
}

void SceneRenderer::set_rendering(bool b) {
    std::lock_guard<std::mutex> lck(mut);
    if (drawable_scene) {
        drawable_scene->set_rendering(b);
    }
}

void SceneRenderer::set_mic(const Vec3f &u) {
    std::lock_guard<std::mutex> lck(mut);
    if (drawable_scene) {
        drawable_scene->set_mic(u);
    }
}
void SceneRenderer::set_source(const Vec3f &u) {
    std::lock_guard<std::mutex> lck(mut);
    if (drawable_scene) {
        drawable_scene->set_source(u);
    }
}

void SceneRenderer::set_waveguide_enabled(bool u) {
    std::lock_guard<std::mutex> lck(mut);
    if (drawable_scene) {
        drawable_scene->set_waveguide_enabled(u);
    }
}
void SceneRenderer::set_raytracer_enabled(bool u) {
    std::lock_guard<std::mutex> lck(mut);
    if (drawable_scene) {
        drawable_scene->set_raytracer_enabled(u);
    }
}

void SceneRenderer::addListener(Listener *l) {
    std::lock_guard<std::mutex> lck(mut);
    listener_list.add(l);
}

void SceneRenderer::removeListener(Listener *l) {
    std::lock_guard<std::mutex> lck(mut);
    listener_list.remove(l);
}

void SceneRenderer::set_positions(const std::vector<cl_float3> &positions) {
    std::lock_guard<std::mutex> lck(mut);
    if (drawable_scene) {
        std::vector<glm::vec3> ret(positions.size());
        proc::transform(positions, ret.begin(), [](const auto &i) {
            return to_glm_vec3(i);
        });
        drawable_scene->set_positions(ret);
    }
}

void SceneRenderer::set_pressures(const std::vector<float> &pressures) {
    std::lock_guard<std::mutex> lck(mut);
    if (drawable_scene) {
        drawable_scene->set_pressures(pressures);
    }
}