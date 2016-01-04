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
#include "combined_config.h"
#include "octree.h"
#include "rayverb.h"

#include "../JuceLibraryCode/JuceHeader.h"

#include <cmath>
#include <mutex>
#include <future>

template <int DRAW_MODE>
class BasicDrawableObject : public ::Drawable {
public:
    BasicDrawableObject(const GenericShader& shader,
                        const std::vector<glm::vec3>& g,
                        const std::vector<glm::vec4>& c,
                        const std::vector<GLuint>& i)
            : shader(shader)
            , size(i.size()) {
        geometry.data(g);
        colors.data(c);
        ibo.data(i);

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
    virtual ~BasicDrawableObject() noexcept = default;

    void draw() const override {
        auto s_shader = shader.get_scoped();
        shader.set_model_matrix(get_matrix());
        shader.set_black(false);

        auto s_vao = vao.get_scoped();
        glDrawElements(DRAW_MODE, size, GL_UNSIGNED_INT, nullptr);
    }

    glm::vec3 get_position() const {
        return position;
    }
    void set_position(const glm::vec3& p) {
        position = p;
    }

    glm::vec3 get_scale() const {
        return scale;
    }
    void set_scale(const glm::vec3& s) {
        scale = s;
    }
    void set_scale(float s) {
        scale = glm::vec3(s, s, s);
    }

    const GenericShader& get_shader() const {
        return shader;
    }

private:
    glm::mat4 get_matrix() const {
        return glm::translate(position) * glm::scale(scale);
    }

    const GenericShader& shader;

    glm::vec3 position{0, 0, 0};
    glm::vec3 scale{1, 1, 1};

    VAO vao;
    StaticVBO geometry;
    StaticVBO colors;
    StaticIBO ibo;
    GLuint size;
};

class SphereObject final : public BasicDrawableObject<GL_TRIANGLES> {
public:
    SphereObject(const GenericShader& shader,
                 const glm::vec3& position,
                 const glm::vec4& color);
};

class BoxObject final : public BasicDrawableObject<GL_LINES> {
public:
    BoxObject(const GenericShader& shader);
};

class ModelSectionObject final : public BasicDrawableObject<GL_TRIANGLES> {
public:
    ModelSectionObject(const GenericShader& shader,
                       const SceneData& scene_data,
                       const Octree& octree);
    void draw() const override;

private:
    std::vector<glm::vec3> get_vertices(const SceneData& scene_data) const;
    std::vector<GLuint> get_indices(const SceneData& scene_data,
                                    const Octree& octree) const;

    void draw_octree(const Octree& octree, BoxObject& box) const;

    Octree octree;
};

class ModelObject final : public BasicDrawableObject<GL_TRIANGLES> {
public:
    ModelObject(const GenericShader& shader, const SceneData& scene_data);

private:
    std::vector<glm::vec3> get_vertices(const SceneData& scene_data) const;
    std::vector<GLuint> get_indices(const SceneData& scene_data) const;
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

    float amp{100};
};

class RaytraceObject final : public ::Drawable {
public:
    RaytraceObject(const GenericShader& shader,
                   const RayverbConfig& cc,
                   const RaytracerResults& results);
    void draw() const override;

private:
    const GenericShader& shader;

    VAO vao;
    StaticVBO geometry;
    StaticVBO colors;
    StaticIBO ibo;
    GLuint size;
};

class SceneRenderer final : public OpenGLRenderer {
public:
    SceneRenderer();
    virtual ~SceneRenderer();
    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;

    void set_aspect(float aspect);
    static glm::mat4 get_projection_matrix(float aspect);

    glm::mat4 get_projection_matrix() const;
    glm::mat4 get_view_matrix() const;

    void set_rotation(float azimuth, float elevation);

    void set_model_object(const SceneData& sceneData);
    void set_config(const CombinedConfig& config);

private:
    void draw() const;
    void trigger_pressure_calculation();
    void init_waveguide(const SceneData& scene_data, const WaveguideConfig& cc);

    CombinedConfig config;

    std::unique_ptr<GenericShader> shader;
    std::unique_ptr<ModelSectionObject> model_object;
    std::unique_ptr<SphereObject> source_object;
    std::unique_ptr<SphereObject> receiver_object;

    std::unique_ptr<TetrahedralWaveguide> waveguide;
    std::unique_ptr<MeshObject> mesh_object;

    std::future<RaytracerResults> raytracer_results;
    std::unique_ptr<RaytraceObject> raytrace_object;

    std::future<std::vector<cl_float>> future_pressure;
    std::thread waveguide_load_thread;

    glm::mat4 projection_matrix;

    glm::mat4 rotation;
    glm::mat4 scale;
    glm::mat4 translation;

    std::mutex mut;

    cl::Context context;
    cl::Device device;
    cl::CommandQueue queue;
};