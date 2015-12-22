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
#include "app_config.h"

#include "../JuceLibraryCode/JuceHeader.h"

#include <cmath>
#include <mutex>

class ModelObject final : public ::Drawable {
public:
    ModelObject(const GenericShader& shader, const SceneData& scene_data);
    void draw() const override;

private:
    const GenericShader& shader;

    VAO vao;
    StaticVBO geometry;
    StaticVBO colors;
    StaticIBO ibo;
    GLuint size;
};

class SphereObject final : public ::Drawable {
public:
    SphereObject(const GenericShader& shader,
                 const glm::vec3& position,
                 const glm::vec4& color);
    void draw() const override;

    glm::mat4 getMatrix() const;

private:
    const GenericShader& shader;

    glm::vec3 position;
    float scale{0.5};

    VAO vao;
    StaticVBO geometry;
    StaticVBO colors;
    StaticIBO ibo;
    GLuint size;
};

class SceneRenderer final : public OpenGLRenderer {
public:
    SceneRenderer();
    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;

    void setAspect(float aspect);
    static glm::mat4 getProjectionMatrix(float aspect);

    glm::mat4 getProjectionMatrix() const;
    glm::mat4 getViewMatrix() const;

    void setRotation(float azimuth, float elevation);

    void setModelObject(const SceneData& sceneData);
    void setConfig(const Config& config);

private:
    std::unique_ptr<GenericShader> shader;
    std::unique_ptr<ModelObject> modelObject;
    std::unique_ptr<SphereObject> sourceObject;
    std::unique_ptr<SphereObject> receiverObject;

    void draw() const;

    glm::mat4 projectionMatrix;

    glm::mat4 rotation;
    glm::mat4 scale;
    glm::mat4 translation;

    std::mutex mut;
};