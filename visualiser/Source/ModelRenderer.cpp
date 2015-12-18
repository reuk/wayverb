#include "ModelRenderer.hpp"

ModelObject::ModelObject(const GenericShader & shader,
                         const SceneData & scene_data)
        : shader(shader) {
    //  init buffers
    auto & scene_verts = scene_data.vertices;
    std::vector<glm::vec3> v(scene_verts.size());
    std::vector<glm::vec4> c(scene_verts.size());
    std::transform(scene_verts.begin(),
                   scene_verts.end(),
                   v.begin(),
                   [](auto i) { return glm::vec3(i.x, i.y, i.z); });
    std::transform(scene_verts.begin(),
                   scene_verts.end(),
                   c.begin(),
                   [](auto i) { return glm::vec4(0, 1, 1, 1); });

    std::vector<GLuint> indices(scene_data.triangles.size() * 3);
    auto count = 0u;
    for (const auto & tri : scene_data.triangles) {
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
    shader.set_model_matrix(mat);
    shader.set_black(false);

    auto s_vao = vao.get_scoped();
    glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, nullptr);
}

void ModelObject::update(float dt) {
    //  init model matrix
    auto s = 0.5;
    mat = glm::scale(glm::vec3(s, s, s));
    mat *= glm::rotate(phase, glm::vec3(0, 1, 0));

    phase += 0.01;
    phase = fmod(phase, 2 * M_PI);
}

const auto OBJ_PATH =
    "/Users/reuben/dev/waveguide/demo/assets/test_models/vault.obj";
const auto MAT_PATH =
    "/Users/reuben/dev/waveguide/demo/assets/materials/vault.json";

ModelRenderer::ModelRenderer()
        : scene_data(OBJ_PATH, MAT_PATH)
        , projectionMatrix(getProjectionMatrix(1)) {
}

void ModelRenderer::newOpenGLContextCreated() {
    shader = std::make_unique<GenericShader>();
    modelObject = std::make_unique<ModelObject>(*shader, scene_data);
}

void ModelRenderer::renderOpenGL() {
    std::lock_guard<std::mutex> lck(mut);
    update();
    draw();
}

void ModelRenderer::openGLContextClosing() {
    modelObject = nullptr;
    shader = nullptr;
}

void ModelRenderer::update() {
    modelObject->update(1);
}

glm::mat4 ModelRenderer::getProjectionMatrix(float aspect) {
    return glm::perspective(45.0f, aspect, 0.05f, 1000.0f);
}

void ModelRenderer::setAspect(float aspect) {
    std::lock_guard<std::mutex> lck(mut);
    projectionMatrix = getProjectionMatrix(aspect);
}

void ModelRenderer::draw() const {
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto phase = 0.0f;

    auto rad = 12;
    glm::vec3 eye(sin(M_PI / 2) * rad, 3, cos(M_PI / 2) * rad);
    glm::vec3 offset(simplex(glm::vec2(phase)),
                     simplex(glm::vec2(phase + 4)),
                     simplex(glm::vec2(phase + 8)));
    offset *= 2;
    eye += offset;
    glm::vec3 target(0, 0, 0);
    glm::vec3 up(0, 1, 0);
    {
        auto s_shader = shader->get_scoped();
        shader->set_view_matrix(glm::lookAt(eye, target, up));
        shader->set_projection_matrix(projectionMatrix);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        modelObject->draw();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}