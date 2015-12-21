#pragma once

#include "ConfigPanel.hpp"

//  unfortunately we need to include these first because otherwise GL/glew.h
//  will do terrible things
#include "modern_gl_utils/generic_shader.h"
#include "modern_gl_utils/drawable.h"
#include "modern_gl_utils/updatable.h"
#include "modern_gl_utils/vao.h"
#include "modern_gl_utils/buffer_object.h"
#include "modern_gl_utils/fbo.h"
#include "modern_gl_utils/texture_object.h"
#include "modern_gl_utils/render_buffer.h"
#include "modern_gl_utils/screen_quad.h"

#define GLM_FORCE_RADIANS
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/noise.hpp"

#include "scene_data.h"

#include "../JuceLibraryCode/JuceHeader.h"

#include <cmath>
#include <mutex>

class ModelObject final : public ::Drawable, public ::Updatable {
public:
    ModelObject(const GenericShader& shader, const SceneData& scene_data);

    ModelObject(const ModelObject& rhs) = delete;
    ModelObject& operator=(const ModelObject& rhs) = delete;
    ModelObject(ModelObject&& rhs) = delete;
    ModelObject& operator=(ModelObject&& rhs) = delete;

    void draw() const override;
    void update(float dt) override;

    void setModelMatrix(const glm::mat4& mat);
    void setScale(float s);

private:
    const GenericShader& shader;

    VAO vao;
    StaticVBO geometry;
    StaticVBO colors;
    StaticIBO ibo;
    GLuint size;

    glm::mat4 scale;
    glm::mat4 translation;
    glm::mat4 mat;
};

class ModelRenderer final : public OpenGLRenderer {
public:
    ModelRenderer();
    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;

    void setAspect(float aspect);
    static glm::mat4 getProjectionMatrix(float aspect);

    glm::mat4 getProjectionMatrix() const;
    glm::mat4 getViewMatrix() const;
    glm::mat4 getMatrices() const;

    void setModelMatrix(const glm::mat4& mat);

    void setModelObject(const SceneData& sceneData);
    void setConfig(const Config& config);

private:
    std::unique_ptr<GenericShader> shader;
    std::unique_ptr<ModelObject> modelObject;

    void update();
    void draw() const;

    glm::mat4 projectionMatrix;

    std::mutex mut;
};