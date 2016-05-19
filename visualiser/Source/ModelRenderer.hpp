#pragma once

#include "BasicDrawableObject.hpp"
#include "BoxObject.hpp"
#include "MeshObject.hpp"
#include "ModelObject.hpp"
#include "ModelSectionObject.hpp"
#include "OctahedronObject.hpp"

#include "ModelWrapper.hpp"

#define GLM_FORCE_RADIANS
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/noise.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"

#include "combined/config.h"

#include "common/octree.h"
#include "common/scene_data.h"
#include "common/voxel_collection.h"

#include "waveguide/waveguide.h"

#include "raytracer/raytracer.h"

#include "RenderState.hpp"

#include <cmath>
#include <future>
#include <mutex>

class RaytraceObject final : public ::Drawable {
public:
    RaytraceObject(const GenericShader& shader,
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

class VoxelisedObject final : public BasicDrawableObject {
public:
    VoxelisedObject(const GenericShader& shader,
                    const SceneData& scene_data,
                    const VoxelCollection& voxel);
    void draw() const override;

private:
    std::vector<glm::vec3> get_vertices(const SceneData& scene_data) const;
    std::vector<GLuint> get_indices(const SceneData& scene_data,
                                    const VoxelCollection& voxel) const;

    VoxelCollection voxel;
};

class DrawableScene final : public ::Drawable, public ::Updatable {
public:
    DrawableScene(const GenericShader& shader, const SceneData& scene_data);

    void update(float dt) override;
    void draw() const override;

    void set_mic(const Vec3f& u);
    void set_source(const Vec3f& u);

    void set_waveguide_enabled(bool b);
    void set_raytracer_enabled(bool b);

    void set_rendering(bool b);

private:
    const GenericShader& shader;

    std::unique_ptr<VoxelisedObject> model_object;
    std::unique_ptr<OctahedronObject> source_object;
    std::unique_ptr<OctahedronObject> mic_object;

    bool waveguide_enabled{true};
    std::unique_ptr<MeshObject> mesh_object;

    bool raytracer_enabled{true};
    std::unique_ptr<RaytraceObject> raytrace_object;

    bool rendering{false};

    mutable std::mutex mut;
};

class SceneRenderer final : public OpenGLRenderer {
public:
    class Listener {
    public:
        Listener() = default;
        Listener(const Listener&) = default;
        Listener& operator=(const Listener&) = default;
        Listener(Listener&&) noexcept = default;
        Listener& operator=(Listener&&) noexcept = default;
        virtual ~Listener() noexcept = default;

        virtual void newOpenGLContextCreated(OpenGLRenderer* r) = 0;
        virtual void openGLContextClosing(OpenGLRenderer* r) = 0;
    };

    SceneRenderer(const SceneData& model);
    virtual ~SceneRenderer() noexcept = default;

    //  lock on all public methods
    //  don't call public methods from one another!
    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;

    void set_aspect(float aspect);
    void update_scale(float delta);
    void set_rotation(float azimuth, float elevation);

    void set_rendering(bool b);

    void set_mic(const Vec3f& u);
    void set_source(const Vec3f& u);

    void set_waveguide_enabled(bool u);
    void set_raytracer_enabled(bool u);

    void addListener(Listener* l);
    void removeListener(Listener* l);

private:
    //  don't lock on anything private
    void update();
    void draw() const;
    glm::mat4 get_projection_matrix() const;
    glm::mat4 get_view_matrix() const;
    glm::mat4 get_scale_matrix() const;

    static glm::mat4 get_projection_matrix(float aspect);

    SceneData model;

    std::unique_ptr<GenericShader> shader;
    std::unique_ptr<DrawableScene> drawable_scene;

    glm::mat4 projection_matrix;

    glm::mat4 rotation;
    float scale;
    glm::mat4 translation;

    ListenerList<Listener> listener_list;

    mutable std::mutex mut;
};
