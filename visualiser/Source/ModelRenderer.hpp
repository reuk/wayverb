#pragma once

#include "AxesObject.hpp"
#include "BasicDrawableObject.hpp"
#include "BoxObject.hpp"
#include "LitSceneShader.hpp"
#include "MeshObject.hpp"
#include "ModelObject.hpp"
#include "ModelSectionObject.hpp"
#include "OctahedronObject.hpp"
#include "PointObject.hpp"

#include "FullModel.hpp"

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

#include "../JuceLibraryCode/JuceHeader.h"

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
                    const CopyableSceneData& scene_data,
                    const VoxelCollection& voxel);
    void draw() const override;

private:
    static std::vector<glm::vec3> get_vertices(
        const CopyableSceneData& scene_data);
    static std::vector<GLuint> get_indices(const CopyableSceneData& scene_data,
                                           const VoxelCollection& voxel);

    VoxelCollection voxel;
};

class MultiMaterialObject : public ::Drawable {
public:
    MultiMaterialObject(const GenericShader& generic_shader,
                        const LitSceneShader& lit_scene_shader,
                        const CopyableSceneData& scene_data);

    void draw() const override;

    class SingleMaterialSection : public ::Drawable {
    public:
        SingleMaterialSection(const CopyableSceneData& scene_data,
                              int material_index);

        void draw() const override;

    private:
        static std::vector<GLuint> get_indices(
            const CopyableSceneData& scene_data, int material_index);
        StaticIBO ibo;
        GLuint size;
    };

    void set_highlighted(int material);

private:
    std::vector<glm::vec3> get_vertices(
        const CopyableSceneData& scene_data) const;

    const GenericShader& generic_shader;
    const LitSceneShader& lit_scene_shader;

    VAO wire_vao;
    VAO fill_vao;
    StaticVBO geometry;
    StaticVBO colors;

    int highlighted{-1};

    std::vector<SingleMaterialSection> sections;
};

class DrawableScene final : public ::Drawable, public ::Updatable {
public:
    DrawableScene(const GenericShader& generic_shader,
                  const MeshShader& mesh_shader,
                  const LitSceneShader& lit_scene_shader,
                  const CopyableSceneData& scene_data);

    void update(float dt) override;
    void draw() const override;

    void set_mic(const Vec3f& u);
    void set_source(const Vec3f& u);

    void set_rendering(bool b);

    void set_positions(const std::vector<glm::vec3>& positions);
    void set_pressures(const std::vector<float>& pressures);

    void set_highlighted(int u);

private:
    const GenericShader& generic_shader;
    const MeshShader& mesh_shader;
    const LitSceneShader& lit_scene_shader;

    MultiMaterialObject model_object;
    //    OctahedronObject source_object;
    //    OctahedronObject mic_object;
    PointObject source_object;
    PointObject mic_object;

    struct MeshContext {
        std::unique_ptr<MeshObject> mesh_object;
        std::vector<glm::vec3> positions;
        std::vector<float> pressures;
        void clear();
    };

    MeshContext mesh_context;
    std::unique_ptr<RaytraceObject> raytrace_object;

    bool rendering{false};
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

    SceneRenderer(const CopyableSceneData& model);

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

    void set_positions(const std::vector<cl_float3>& positions);
    void set_pressures(const std::vector<float>& pressures);

    void set_highlighted(int u);

    void addListener(Listener* l);
    void removeListener(Listener* l);

private:
    class ContextLifetime : public ::Drawable, public ::Updatable {
    public:
        ContextLifetime(const CopyableSceneData& scene_data);

        void set_aspect(float aspect);
        void update_scale(float delta);
        void set_rotation(float azimuth, float elevation);

        void set_rendering(bool b);

        void set_mic(const Vec3f& u);
        void set_source(const Vec3f& u);

        void set_positions(const std::vector<cl_float3>& positions);
        void set_pressures(const std::vector<float>& pressures);

        void set_highlighted(int u);

        void draw() const override;
        void update(float dt) override;

    private:
        glm::mat4 get_projection_matrix() const;
        glm::mat4 get_view_matrix() const;
        glm::mat4 get_scale_matrix() const;

        static glm::mat4 get_projection_matrix(float aspect);

        const CopyableSceneData& model;

        GenericShader generic_shader;
        MeshShader mesh_shader;
        LitSceneShader lit_scene_shader;
        DrawableScene drawable_scene;
        AxesObject axes;

        glm::mat4 projection_matrix;

        glm::mat4 rotation;
        float scale;
        glm::mat4 translation;
    };

    CopyableSceneData model;

    std::unique_ptr<ContextLifetime> context_lifetime;

    ListenerList<Listener> listener_list;

    mutable std::mutex mut;
};
