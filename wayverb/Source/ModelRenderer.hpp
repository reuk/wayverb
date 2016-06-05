#pragma once

#include "AxesObject.hpp"
#include "BasicDrawableObject.hpp"
#include "BoxObject.hpp"
#include "LitSceneShader.hpp"
#include "MeshObject.hpp"
#include "ModelObject.hpp"
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

class WorkQueue {
public:
    struct WorkItem {
        virtual ~WorkItem() noexcept = default;
        virtual void operator()() const = 0;
    };

    template <typename Method>
    struct GenericWorkItem : public WorkItem {
        GenericWorkItem(Method&& method)
                : method(std::forward<Method>(method)) {
        }
        void operator()() const override {
            method();
        }
        Method method;
    };

    template <typename Method>
    void push(Method&& method) {
        std::lock_guard<std::mutex> lck(mut);
        work_items.push(std::make_unique<GenericWorkItem<Method>>(
            std::forward<Method>(method)));
    }

    std::unique_ptr<WorkItem> pop_one() {
        std::lock_guard<std::mutex> lck(mut);
        if (work_items.empty()) {
            return nullptr;
        }
        auto ret = std::move(work_items.front());
        work_items.pop();
        return ret;
    }

    auto size() const {
        std::lock_guard<std::mutex> lck(mut);
        return work_items.size();
    }

    auto empty() const {
        std::lock_guard<std::mutex> lck(mut);
        return work_items.empty();
    }

private:
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

    /// Should return a list of Nodes which can be selected with the mouse.
    std::vector<Node*> get_selectable_objects();

    Node* get_source();
    Node* get_receiver();

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
                            public AsyncUpdater {
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
    void renderOpenGL() override;
    void openGLContextClosing() override;

    void set_viewport(const glm::ivec2& v);
    void set_scale(float u);
    void set_rotation(const Orientable::AzEl& u);

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

    void mouse_down(const glm::vec2& pos);
    void mouse_drag(const glm::vec2& pos);
    void mouse_up(const glm::vec2& pos);
    void mouse_wheel_move(float delta_y);

private:
    void handleAsyncUpdate() override;

    void broadcast_receiver_position(const glm::vec3& pos);
    void broadcast_source_position(const glm::vec3& pos);

    //  gl state should be updated inside `update` methods
    //  where we know that stuff is happening on the gl thread
    //  if you (e.g.) delete a buffer on a different thread, your computer will
    //  get mad and maybe freeze
    WorkQueue work_queue;
    WorkQueue outgoing_work_queue;
    CopyableSceneData model;

    class ContextLifetime;
    std::unique_ptr<ContextLifetime> context_lifetime;

    ListenerList<Listener> listener_list;

    mutable std::mutex mut;
};
