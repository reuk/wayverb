#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "FullModel.hpp"

#include "BoxObject.hpp"
#include "LitSceneShader.hpp"
#include "MeshObject.hpp"
#include "ModelObject.hpp"
#include "PointObject.hpp"

#include "OtherComponents/AxesObject.hpp"
#include "OtherComponents/BasicDrawableObject.hpp"
#include "OtherComponents/RenderHelpers.hpp"
#include "OtherComponents/WorkQueue.hpp"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/noise.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"

#include "combined/model.h"

#include "waveguide/waveguide.h"

#include "raytracer/raytracer.h"

#include "common/octree.h"
#include "common/scene_data.h"
#include "common/voxel_collection.h"

#include <cmath>
#include <future>
#include <mutex>
#include <queue>

class MultiMaterialObject : public mglu::drawable {
public:
    MultiMaterialObject(mglu::generic_shader& generic_shader,
                        LitSceneShader& lit_scene_shader,
                        const copyable_scene_data& scene_data);

    class SingleMaterialSection : public mglu::drawable {
    public:
        SingleMaterialSection(const copyable_scene_data& scene_data,
                              int material_index);

    private:
        void do_draw(const glm::mat4& modelview_matrix) const override;
        glm::mat4 get_local_modelview_matrix() const override;

        static aligned::vector<GLuint> get_indices(
                const copyable_scene_data& scene_data, int material_index);
        mglu::static_ibo ibo;
        GLuint size;
    };

    void set_highlighted(int material);
    void set_colour(const glm::vec3& c);

private:
    void do_draw(const glm::mat4& modelview_matrix) const override;
    glm::mat4 get_local_modelview_matrix() const override;

    mglu::generic_shader* generic_shader;
    LitSceneShader* lit_scene_shader;

    mglu::vao wire_vao;
    mglu::vao fill_vao;
    mglu::static_vbo geometry;
    mglu::static_vbo colors;

    int highlighted{-1};

    aligned::vector<SingleMaterialSection> sections;
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

        virtual void source_dragged(
                SceneRenderer*, const aligned::vector<glm::vec3>& new_pos) = 0;
        virtual void receiver_dragged(
                SceneRenderer*, const aligned::vector<glm::vec3>& new_pos) = 0;
    };

    SceneRenderer(const copyable_scene_data& model);
    virtual ~SceneRenderer() noexcept;

    void newOpenGLContextCreated() override;
    void openGLContextClosing() override;

    void set_rendering(bool b);

    void set_sources(const aligned::vector<glm::vec3>& sources);
    void set_receivers(
            const aligned::vector<model::ReceiverSettings>& receivers);

    void set_positions(const aligned::vector<glm::vec3>& positions);
    void set_pressures(const aligned::vector<float>& pressures);

    void set_highlighted(int u);

    void set_emphasis(const glm::vec3& c);

    void addListener(Listener*);
    void removeListener(Listener*);

private:
    BaseContextLifetime* get_context_lifetime() override;

    void broadcast_receiver_positions(const aligned::vector<glm::vec3>& pos);
    void broadcast_source_positions(const aligned::vector<glm::vec3>& pos);

    mutable std::mutex mut;

    copyable_scene_data model;

    class ContextLifetime;
    std::unique_ptr<ContextLifetime> context_lifetime;

    ListenerList<Listener> listener_list;
};
