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

#include "waveguide.h"
#include "scene_data.h"
#include "app_config.h"

#include "../JuceLibraryCode/JuceHeader.h"

#include <cmath>
#include <mutex>
#include <future>

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

class MeshObject final : public ::Drawable {
public:
    MeshObject(const GenericShader& shader,
               const TetrahedralWaveguide& waveguide);
    void draw() const override;

    void set_pressures(const std::vector<float>& pressures);

private:
    const GenericShader& shader;

    VAO vao;
    StaticVBO geometry;
    DynamicVBO colors;
    StaticIBO ibo;
    GLuint size;
};

class SphereObject final : public ::Drawable {
public:
    SphereObject(const GenericShader& shader,
                 const glm::vec3& position,
                 const glm::vec4& color);
    void draw() const override;

    glm::mat4 get_matrix() const;

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
    ~SceneRenderer();
    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;

    void set_aspect(float aspect);
    static glm::mat4 get_projection_matrix(float aspect);

    glm::mat4 get_projection_matrix() const;
    glm::mat4 get_view_matrix() const;

    void set_rotation(float azimuth, float elevation);

    void set_model_object(const SceneData& sceneData);
    void set_config(const Config& config);

private:
    std::unique_ptr<GenericShader> shader;
    std::unique_ptr<ModelObject> model_object;
    std::unique_ptr<SphereObject> source_object;
    std::unique_ptr<SphereObject> receiver_object;

    std::unique_ptr<TetrahedralWaveguide> waveguide;
    std::unique_ptr<MeshObject> mesh_object;

    std::future<std::vector<cl_float>> future_pressure;

    void draw() const;
    void trigger_pressure_calculation();

    glm::mat4 projection_matrix;

    glm::mat4 rotation;
    glm::mat4 scale;
    glm::mat4 translation;

    std::mutex mut;

    cl::Context context;
    cl::Device device;
    cl::CommandQueue queue;
};