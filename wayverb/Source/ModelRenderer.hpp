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
#include <queue>

template <typename T>
class WorkItemOwner {
public:
    WorkItemOwner(T& obj)
            : obj(obj) {
    }

    struct WorkItem {
        virtual ~WorkItem() noexcept = default;
        virtual void operator()(T& obj) const = 0;
    };

    template <typename Method>
    struct GenericWorkItem : public WorkItem {
        GenericWorkItem(Method&& method)
                : method(std::forward<Method>(method)) {
        }
        void operator()(T& obj) const override {
            method(obj);
        }
        Method method;
    };

    void push(std::unique_ptr<WorkItem>&& u) {
        std::lock_guard<std::mutex> lck(mut);
        work_items.push(std::move(u));
    }

    template <typename Method>
    void push(Method&& method) {
        std::lock_guard<std::mutex> lck(mut);
        work_items.push(std::make_unique<GenericWorkItem<Method>>(
            std::forward<Method>(method)));
    }

    void pop_one() {
        std::lock_guard<std::mutex> lck(mut);
        pop_impl();
    }

    void pop_all() {
        std::lock_guard<std::mutex> lck(mut);
        while (!work_items.empty()) {
            pop_impl();
        }
    }

private:
    void pop_impl() {
        (*work_items.front())(obj);
        work_items.pop();
    }

    T& obj;
    std::queue<std::unique_ptr<WorkItem>> work_items;
    mutable std::mutex mut;
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
    void set_colour(const glm::vec3& c);

private:
    const GenericShader& generic_shader;
    const LitSceneShader& lit_scene_shader;

    VAO wire_vao;
    VAO fill_vao;
    StaticVBO geometry;
    StaticVBO colors;

    int highlighted{-1};

    std::vector<SingleMaterialSection> sections;
};

class DrawableScene final : public ::Drawable {
public:
    DrawableScene(const GenericShader& generic_shader,
                  const MeshShader& mesh_shader,
                  const LitSceneShader& lit_scene_shader,
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

private:
    const GenericShader& generic_shader;
    const MeshShader& mesh_shader;
    const LitSceneShader& lit_scene_shader;

    MultiMaterialObject model_object;
    PointObject source_object;
    PointObject receiver_object;

    std::unique_ptr<MeshObject> mesh_object;

    bool rendering{false};
};

class SceneRenderer final : public OpenGLRenderer,
                            public ChangeBroadcaster,
                            public WorkItemOwner<SceneRenderer> {
public:
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

    void set_receiver(const glm::vec3& u);
    void set_source(const glm::vec3& u);

    void set_positions(const std::vector<cl_float3>& positions);
    void set_pressures(const std::vector<float>& pressures);

    void set_highlighted(int u);

    void set_emphasis(const glm::vec3& c);

    void set_receiver_pointing(const std::vector<glm::vec3>& directions);

private:
    //  gl state should be updated inside `update` methods
    //  where we know that stuff is happening on the gl thread
    //  if you (e.g.) delete a buffer on a different thread, your computer will
    //  get mad and maybe freeze
    class ContextLifetime : public ::Drawable {
    public:
        ContextLifetime(const CopyableSceneData& scene_data);

        void set_aspect(float aspect);
        void update_scale(float delta);
        void set_rotation(float azimuth, float elevation);

        void set_rendering(bool b);

        void set_receiver(const glm::vec3& u);
        void set_source(const glm::vec3& u);

        void set_positions(const std::vector<cl_float3>& positions);
        void set_pressures(const std::vector<float>& pressures);

        void set_highlighted(int u);

        void set_emphasis(const glm::vec3& c);

        void set_receiver_pointing(const std::vector<glm::vec3>& directions);

        void draw() const override;

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
};
