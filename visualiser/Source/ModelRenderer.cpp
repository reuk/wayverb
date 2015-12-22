#include "ModelRenderer.hpp"

#include "boundaries.h"
#include "conversions.h"

#include "combined_config.h"

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

glm::mat4 SphereObject::getMatrix() const {
    auto s = glm::scale(glm::vec3(scale, scale, scale));
    auto t = glm::translate(position);
    return s * t;
}

//----------------------------------------------------------------------------//

SceneRenderer::SceneRenderer()
        : projectionMatrix(getProjectionMatrix(1)) {
}

void SceneRenderer::newOpenGLContextCreated() {
    shader = std::make_unique<GenericShader>();

#ifdef DEBUG
    File object(
        "/Users/reuben/dev/waveguide/demo/assets/test_models/vault.obj");
    File material(
        "/Users/reuben/dev/waveguide/demo/assets/materials/vault.json");
    File config("/Users/reuben/dev/waveguide/demo/assets/configs/vault.json");

    setModelObject(SceneData(object.getFullPathName().toStdString(),
                             material.getFullPathName().toStdString()));
    try {
        setConfig(read_config(config.getFullPathName().toStdString()));
    } catch (...) {
    }
#endif
}

void SceneRenderer::renderOpenGL() {
    std::lock_guard<std::mutex> lck(mut);
    draw();
}

void SceneRenderer::openGLContextClosing() {
    modelObject = nullptr;
    shader = nullptr;
}

glm::mat4 SceneRenderer::getProjectionMatrix(float aspect) {
    return glm::perspective(45.0f, aspect, 0.05f, 1000.0f);
}

void SceneRenderer::setAspect(float aspect) {
    std::lock_guard<std::mutex> lck(mut);
    projectionMatrix = getProjectionMatrix(aspect);
}

void SceneRenderer::draw() const {
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    {
        auto s_shader = shader->get_scoped();
        auto mm = rotation * scale * translation;
        shader->set_model_matrix(mm);
        shader->set_view_matrix(getViewMatrix());
        shader->set_projection_matrix(getProjectionMatrix());

        if (modelObject) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            modelObject->draw();
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            auto drawHedron = [this, &mm](const auto &i) {
                auto s_shader = shader->get_scoped();
                if (i) {
                    shader->set_model_matrix(mm * i->getMatrix());
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    i->draw();
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                }
            };

            drawHedron(sourceObject);
            drawHedron(receiverObject);
        }
    }
}

glm::mat4 SceneRenderer::getProjectionMatrix() const {
    return projectionMatrix;
}
glm::mat4 SceneRenderer::getViewMatrix() const {
    auto rad = 20;
    glm::vec3 eye(0, 0, rad);
    glm::vec3 target(0, 0, 0);
    glm::vec3 up(0, 1, 0);
    return glm::lookAt(eye, target, up);
}

void SceneRenderer::setModelObject(const SceneData &sceneData) {
    std::unique_lock<std::mutex> lck(mut);

    auto aabb = sceneData.get_aabb();
    auto m = aabb.get_centre();
    lck.unlock();
    translation = glm::translate(-glm::vec3(m.x, m.y, m.z));

    auto max = (aabb.c1 - aabb.c0).max();
    auto s = max > 0 ? 20 / max : 1;
    scale = glm::scale(glm::vec3(s, s, s));

    modelObject = std::make_unique<ModelObject>(*shader, sceneData);
}

void SceneRenderer::setConfig(const Config &config) {
    std::unique_lock<std::mutex> lck(mut);

    auto s = config.get_source();
    glm::vec3 spos(s.x, s.y, s.z);
    auto r = config.get_mic();
    glm::vec3 rpos(r.x, r.y, r.z);

    sourceObject =
        std::make_unique<SphereObject>(*shader, spos, glm::vec4(1, 0, 0, 1));
    receiverObject =
        std::make_unique<SphereObject>(*shader, rpos, glm::vec4(0, 1, 1, 1));
}

void SceneRenderer::setRotation(float az, float el) {
    std::lock_guard<std::mutex> lck(mut);
    auto i = glm::rotate(az, glm::vec3(0, 1, 0));
    auto j = glm::rotate(el, glm::vec3(1, 0, 0));
    rotation = j * i;
}