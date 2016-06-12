#pragma once

#include "AxesObject.hpp"
#include "BasicDrawableObject.hpp"
#include "BoxObject.hpp"
#include "LitSceneShader.hpp"
#include "MeshObject.hpp"
#include "ModelObject.hpp"
#include "OctahedronObject.hpp"
#include "PointObject.hpp"
#include "RenderHelpers.hpp"
#include "WorkQueue.hpp"

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
#include <queue>

class MultiMaterialObject : public ::Drawable {
public:
    MultiMaterialObject(GenericShader& generic_shader,
                        LitSceneShader& lit_scene_shader,
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
    void set_colour(const glm::vec3& c);

private:
    GenericShader* generic_shader;
    LitSceneShader* lit_scene_shader;

    VAO wire_vao;
    VAO fill_vao;
    StaticVBO geometry;
    StaticVBO colors;

    int highlighted{-1};

    std::vector<SingleMaterialSection> sections;
};

class DrawableScene final : public ::Drawable {
public:
    DrawableScene(GenericShader& generic_shader,
                  MeshShader& mesh_shader,
                  LitSceneShader& lit_scene_shader,
                  const CopyableSceneData& scene_data);

    void draw() const override;

    void set_receiver(const glm::vec3& u);
    void set_source(const glm::vec3& u);

    void set_rendering(bool b);

    void set_positions(const std::vector<glm::vec3>& positions);
    void set_pressures(const std::vector<float>& pressures);

    void set_highlighted(int u);

    void set_receiver_pointing(const std::vector<glm::vec3>& directions);

    void set_emphasis(const glm::vec3& c);

    /// Should return a list of Nodes which can be selected with the mouse.
    std::vector<Node*> get_selectable_objects();

    Node* get_source();
    Node* get_receiver();

private:
    GenericShader* generic_shader;
    MeshShader* mesh_shader;
    LitSceneShader* lit_scene_shader;

    MultiMaterialObject model_object;
    PointObject source_object;
    PointObject receiver_object;

    std::unique_ptr<MeshObject> mesh_object;

    bool rendering{false};
};

class SceneRenderer final : public BaseRenderer {
public:
    class Listener {
    public:
        Listener() = default;
        Listener(const Listener&) = default;
        Listener& operator=(const Listener&) = default;
        Listener(Listener&&) noexcept = default;
        Listener& operator=(Listener&&) noexcept = default;
        virtual ~Listener() noexcept = default;

        virtual void source_dragged(SceneRenderer*,
                                    const glm::vec3& new_pos) = 0;
        virtual void receiver_dragged(SceneRenderer*,
                                      const glm::vec3& new_pos) = 0;
    };

    SceneRenderer(const CopyableSceneData& model);
    virtual ~SceneRenderer() noexcept;

    void newOpenGLContextCreated() override;
    void openGLContextClosing() override;

    void set_rendering(bool b);

    void set_receiver(const glm::vec3& u);
    void set_source(const glm::vec3& u);

    void set_positions(const std::vector<cl_float3>& positions);
    void set_pressures(const std::vector<float>& pressures);

    void set_highlighted(int u);

    void set_emphasis(const glm::vec3& c);

    void set_receiver_pointing(const std::vector<glm::vec3>& directions);

    void addListener(Listener*);
    void removeListener(Listener*);

private:
    BaseContextLifetime* get_context_lifetime() override;

    void broadcast_receiver_position(const glm::vec3& pos);
    void broadcast_source_position(const glm::vec3& pos);

    mutable std::mutex mut;

    CopyableSceneData model;

    class ContextLifetime;
    std::unique_ptr<ContextLifetime> context_lifetime;

    ListenerList<Listener> listener_list;
};
