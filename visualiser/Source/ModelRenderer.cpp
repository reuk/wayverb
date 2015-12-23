#include "ModelRenderer.hpp"

#include "boundaries.h"
#include "conversions.h"
#include "cl_common.h"
#include "combined_config.h"
#include "tetrahedral_program.h"

ModelObject::ModelObject(const GenericShader &shader,
                         const SceneData &scene_data)
        : shader(shader) {
    //  init buffers
    auto &scene_verts = scene_data.vertices;
    std::vector<glm::vec3> v(scene_verts.size());
    std::vector<glm::vec4> c(scene_verts.size());
    std::transform(scene_verts.begin(),
                   scene_verts.end(),
                   v.begin(),
                   [](auto i) { return glm::vec3(i.x, i.y, i.z); });
    std::transform(scene_verts.begin(),
                   scene_verts.end(),
                   c.begin(),
                   [](auto i) { return glm::vec4(1, 1, 1, 1); });

    std::vector<GLuint> indices(scene_data.triangles.size() * 3);
    auto count = 0u;
    for (const auto &tri : scene_data.triangles) {
        indices[count + 0] = tri.v0;
        indices[count + 1] = tri.v1;
        indices[count + 2] = tri.v2;
        count += 3;
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

void ModelObject::draw() const {
    auto s_shader = shader.get_scoped();
    shader.set_black(false);

    auto s_vao = vao.get_scoped();
    glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, nullptr);
}

//----------------------------------------------------------------------------//

SphereObject::SphereObject(const GenericShader &shader,
                           const glm::vec3 &position,
                           const glm::vec4 &color)
        : shader(shader)
        , position(position) {
    //  init buffers
    std::vector<glm::vec3> v{
        {0, 0, -1}, {0, 0, 1}, {0, -1, 0}, {0, 1, 0}, {-1, 0, 0}, {1, 0, 0},
    };
    std::transform(
        v.begin(), v.end(), v.begin(), [this](auto i) { return i * scale; });
    std::vector<glm::vec4> c(v.size(), color);

    std::vector<GLuint> indices{
        0, 2, 4, 0, 2, 5, 0, 3, 4, 0, 3, 5, 1, 2, 4, 1, 2, 5, 1, 3, 4, 1, 3, 5,
    };
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

void SphereObject::draw() const {
    auto s_shader = shader.get_scoped();
    shader.set_black(false);

    auto s_vao = vao.get_scoped();
    glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, nullptr);
}

glm::mat4 SphereObject::get_matrix() const {
    auto s = glm::scale(glm::vec3(scale, scale, scale));
    auto t = glm::translate(position);
    return t * s;
}

//----------------------------------------------------------------------------//

MeshObject::MeshObject(const GenericShader &shader,
                       const TetrahedralWaveguide &waveguide)
        : shader(shader) {
    auto nodes = waveguide.get_mesh().get_nodes();
    std::vector<glm::vec3> v(nodes.size());
    std::transform(nodes.begin(),
                   nodes.end(),
                   v.begin(),
                   [](auto i) {
                       auto p = i.position;
                       return glm::vec3(p.x, p.y, p.z);
                   });

    //  init buffers
    std::vector<glm::vec4> c(v.size());
    std::transform(nodes.begin(),
                   nodes.end(),
                   c.begin(),
                   [](auto i) {
                       auto c = i.inside ? 1 : 0;
                       return glm::vec4(c, c, c, c);
                   });

    std::vector<GLuint> indices(v.size());
    std::iota(indices.begin(), indices.end(), 0);

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

void MeshObject::draw() const {
    auto s_shader = shader.get_scoped();
    shader.set_black(false);

    auto s_vao = vao.get_scoped();
    glDrawElements(GL_POINTS, size, GL_UNSIGNED_INT, nullptr);
}

void MeshObject::set_pressures(const std::vector<float> &pressures) {
    std::vector<glm::vec4> c(pressures.size());
    std::transform(pressures.begin(),
                   pressures.end(),
                   c.begin(),
                   [](auto i) {
                       auto p = i * 100;
                       return p > 0 ? glm::vec4(0, p, p, 1)
                                    : glm::vec4(-p, 0, 0, 1);
                   });
    colors.data(c);
}

//----------------------------------------------------------------------------//

SceneRenderer::SceneRenderer()
        : projection_matrix(get_projection_matrix(1))
        , context(get_context())
        , device(get_device(context))
        , queue(context, device) {
}

SceneRenderer::~SceneRenderer() {
    if (future_pressure.valid())
        future_pressure.get();
}

void SceneRenderer::newOpenGLContextCreated() {
    shader = std::make_unique<GenericShader>();

    File object(
        "/Users/reuben/dev/waveguide/demo/assets/test_models/vault.obj");
    File material(
        "/Users/reuben/dev/waveguide/demo/assets/materials/vault.json");
    File config("/Users/reuben/dev/waveguide/demo/assets/configs/vault.json");

    SceneData sceneData(object.getFullPathName().toStdString(),
                        material.getFullPathName().toStdString());
    CombinedConfig cc;
    try {
        cc = read_config(config.getFullPathName().toStdString());
    } catch (...) {
    }

    MeshBoundary boundary(sceneData);

    auto waveguide_program = get_program<TetrahedralProgram>(context, device);
    //    waveguide = std::make_unique<TetrahedralWaveguide>(
    //        waveguide_program, queue, boundary, cc.get_divisions(),
    //        cc.get_mic());

    waveguide = std::make_unique<TetrahedralWaveguide>(
        waveguide_program, queue, boundary, 0.1, cc.get_mic());

    waveguide->init(
        cc.get_source(), TetrahedralWaveguide::GaussianFunction(0.5), 0, 0);

    //    auto corrected_source =
    //        waveguide->get_corrected_coordinate(cc.get_source());
    //    waveguide->init(
    //        corrected_source, TetrahedralWaveguide::BasicPowerFunction(), 0,
    //        0);

    trigger_pressure_calculation();

    mesh_object = std::make_unique<MeshObject>(*shader, *waveguide);

    set_model_object(sceneData);
    set_config(cc);
}

void SceneRenderer::renderOpenGL() {
    std::lock_guard<std::mutex> lck(mut);
    if (future_pressure.valid() && waveguide && mesh_object) {
        try {
            if (future_pressure.wait_for(std::chrono::milliseconds(1)) ==
                std::future_status::ready) {
                mesh_object->set_pressures(future_pressure.get());
                trigger_pressure_calculation();
            }
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
        }
    }
    draw();
}

void SceneRenderer::trigger_pressure_calculation() {
    try {
        future_pressure = std::async(std::launch::async,
                                     [this] {
                                         auto ret = waveguide->run_step_slow();
                                         waveguide->swap_buffers();
                                         return ret;
                                     });
    } catch (...) {
        std::cout << "async error?" << std::endl;
    }
}

void SceneRenderer::openGLContextClosing() {
    model_object = nullptr;
    shader = nullptr;
}

glm::mat4 SceneRenderer::get_projection_matrix(float aspect) {
    return glm::perspective(45.0f, aspect, 0.05f, 1000.0f);
}

void SceneRenderer::set_aspect(float aspect) {
    std::lock_guard<std::mutex> lck(mut);
    projection_matrix = get_projection_matrix(aspect);
}

void SceneRenderer::draw() const {
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    auto s_shader = shader->get_scoped();
    shader->set_model_matrix(glm::mat4());
    shader->set_view_matrix(get_view_matrix());
    shader->set_projection_matrix(get_projection_matrix());

    if (mesh_object) {
        mesh_object->draw();
    }

    if (model_object) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        model_object->draw();

        auto drawThing = [this](const auto &i) {
            auto s_shader = shader->get_scoped();
            if (i) {
                shader->set_model_matrix(i->get_matrix());
                i->draw();
            }
        };

        drawThing(source_object);
        drawThing(receiver_object);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

glm::mat4 SceneRenderer::get_projection_matrix() const {
    return projection_matrix;
}
glm::mat4 SceneRenderer::get_view_matrix() const {
    auto rad = 20;
    glm::vec3 eye(0, 0, rad);
    glm::vec3 target(0, 0, 0);
    glm::vec3 up(0, 1, 0);
    auto mm = rotation * scale * translation;
    return glm::lookAt(eye, target, up) * mm;
}

void SceneRenderer::set_model_object(const SceneData &sceneData) {
    std::unique_lock<std::mutex> lck(mut);

    auto aabb = sceneData.get_aabb();
    auto m = aabb.get_centre();
    lck.unlock();
    translation = glm::translate(-glm::vec3(m.x, m.y, m.z));

    auto max = (aabb.c1 - aabb.c0).max();
    auto s = max > 0 ? 20 / max : 1;
    scale = glm::scale(glm::vec3(s, s, s));

    model_object = std::make_unique<ModelObject>(*shader, sceneData);
}

void SceneRenderer::set_config(const Config &config) {
    std::unique_lock<std::mutex> lck(mut);

    auto s = config.get_source();
    glm::vec3 spos(s.x, s.y, s.z);
    auto r = config.get_mic();
    glm::vec3 rpos(r.x, r.y, r.z);

    source_object =
        std::make_unique<SphereObject>(*shader, spos, glm::vec4(1, 0, 0, 1));
    receiver_object =
        std::make_unique<SphereObject>(*shader, rpos, glm::vec4(0, 1, 1, 1));
}

void SceneRenderer::set_rotation(float az, float el) {
    std::lock_guard<std::mutex> lck(mut);
    auto i = glm::rotate(az, glm::vec3(0, 1, 0));
    auto j = glm::rotate(el, glm::vec3(1, 0, 0));
    rotation = j * i;
}