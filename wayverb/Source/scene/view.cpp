#include "view.h"

#include "../3d_objects/AxesObject.h"
#include "../3d_objects/LitSceneShader.h"
#include "../3d_objects/mesh_object.h"
#include "../3d_objects/multi_material_object.h"
#include "../3d_objects/reflections_object.h"

#include "modern_gl_utils/generic_shader.h"

#include "../JuceLibraryCode/JuceHeader.h"  //  Only for GL stuff

namespace scene {

class view::impl final {
public:
    //  Nodes.

    void set_node_positions(util::aligned::vector<glm::vec3> positions) {
        mesh_object_ = std::make_unique<mesh_object>(
                mesh_shader_, positions.data(), positions.size());
    }

    void set_node_colours(util::aligned::vector<glm::vec4> colours) {
        if (mesh_object_) {
            mesh_object_->set_colours(colours.data(), colours.size());
        }
    }

    void set_nodes_visible(bool visible) { nodes_visible_ = visible; }

    //  Reflections.

    void set_reflections(util::aligned::vector<util::aligned::vector<
                                 wayverb::raytracer::reflection>> reflections,
                         const glm::vec3& source) {
        reflections_object_ = std::make_unique<reflections_object>(
                reflections_shader_, std::move(reflections), source);
        reflections_object_->set_distance(distance_);
    }

    void set_distance_travelled(float distance) {
        distance_ = distance;
        if (reflections_object_) {
            reflections_object_->set_distance(distance);
        }
    }

    void set_reflections_visible(bool visible) {
        reflections_visible_ = visible;
    }

    //  Scene/surfaces.

    void set_scene(wayverb::core::gpu_scene_data scene) {
        model_object_ = std::make_unique<multi_material_object>(
                generic_shader_, lit_scene_shader_, scene);
        model_object_->set_highlighted(highlighted_);
    }

    void set_highlighted_surface(
            std::experimental::optional<size_t> highlighted) {
        highlighted_ = highlighted;
        if (model_object_) {
            model_object_->set_highlighted(highlighted);
        }
    }

    void set_emphasis_colour(const glm::vec3& colour) {
        const auto s_shader = lit_scene_shader_->get_scoped();
        lit_scene_shader_->set_colour(colour);
    }

    //  TODO Sources/receivers.

    // void set_sources(util::aligned::vector<glm::vec3> sources);
    // void set_receivers(util::aligned::vector<glm::vec3> receivers);

    //  Drawing functionality.

    void set_view_matrix(const glm::mat4& matrix) { view_matrix_ = matrix; }

    void set_projection_matrix(const glm::mat4& matrix) {
        projection_matrix_ = matrix;
    }

    void update(float) {}

    void do_draw(const glm::mat4& model_matrix) const {
        const auto c = 0.0;
        glClearColor(c, c, c, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_DEPTH_TEST);

        glEnable(GL_MULTISAMPLE);
        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_POLYGON_SMOOTH);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        const auto config_shader = [this](const auto& shader) {
            const auto s_shader = shader->get_scoped();
            shader->set_model_matrix(glm::mat4{});
            shader->set_view_matrix(view_matrix_);
            shader->set_projection_matrix(projection_matrix_);
        };

        config_shader(generic_shader_);
        config_shader(mesh_shader_);
        config_shader(lit_scene_shader_);
        config_shader(reflections_shader_);

        if (model_object_) {
            model_object_->draw(model_matrix);
        }
        //  TODO
        //  point_objects.draw(model_matrix);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        if (nodes_visible_ && mesh_object_) {
            mesh_object_->draw(model_matrix);
        }

        if (reflections_visible_ && reflections_object_) {
            reflections_object_->draw(model_matrix);
        }

        axes_.draw(model_matrix);
    }

    glm::mat4 get_local_model_matrix() const { return glm::mat4{}; }

private:
    //  State
    glm::mat4 view_matrix_;
    glm::mat4 projection_matrix_;

    //  Shaders
    std::shared_ptr<mglu::generic_shader> generic_shader_ =
            std::make_shared<mglu::generic_shader>();
    std::shared_ptr<mesh_shader> mesh_shader_ = std::make_shared<mesh_shader>();
    std::shared_ptr<LitSceneShader> lit_scene_shader_ =
            std::make_shared<LitSceneShader>();
    std::shared_ptr<reflections_shader> reflections_shader_ =
            std::make_shared<reflections_shader>();

    //  Objects
    std::unique_ptr<multi_material_object> model_object_;
    std::experimental::optional<size_t> highlighted_ =
            std::experimental::nullopt;

    std::unique_ptr<mesh_object> mesh_object_;
    bool nodes_visible_ = false;

    std::unique_ptr<reflections_object> reflections_object_;
    bool reflections_visible_ = false;
    double distance_ = 0.0;

    //  PointObjects
    AxesObject axes_{generic_shader_};
};

////////////////////////////////////////////////////////////////////////////////

view::view()
        : pimpl_{std::make_unique<impl>()} {}

view::~view() noexcept = default;

void view::set_node_positions(util::aligned::vector<glm::vec3> positions) {
    pimpl_->set_node_positions(std::move(positions));
}

void view::set_node_colours(util::aligned::vector<glm::vec4> colours) {
    pimpl_->set_node_colours(std::move(colours));
}

void view::set_nodes_visible(bool visible) {
    pimpl_->set_nodes_visible(visible);
}

void view::set_reflections(util::aligned::vector<util::aligned::vector<
                                   wayverb::raytracer::reflection>> reflections,
                           const glm::vec3& source) {
    pimpl_->set_reflections(std::move(reflections), source);
}

void view::set_distance_travelled(float distance) {
    pimpl_->set_distance_travelled(distance);
}

void view::set_reflections_visible(bool visible) {
    pimpl_->set_reflections_visible(visible);
}

void view::set_scene(wayverb::core::gpu_scene_data scene) {
    pimpl_->set_scene(std::move(scene));
}

void view::set_highlighted_surface(
        std::experimental::optional<size_t> highlighted) {
    pimpl_->set_highlighted_surface(highlighted);
}

void view::set_emphasis_colour(const glm::vec3& colour) {
    pimpl_->set_emphasis_colour(colour);
}

void view::set_view_matrix(const glm::mat4& matrix) {
    pimpl_->set_view_matrix(matrix);
}

void view::set_projection_matrix(const glm::mat4& matrix) {
    pimpl_->set_projection_matrix(matrix);
}

void view::update(float dt) { pimpl_->update(dt); }

void view::do_draw(const glm::mat4& matrix) const { pimpl_->do_draw(matrix); }

glm::mat4 view::get_local_model_matrix() const {
    return pimpl_->get_local_model_matrix();
}

}  // namespace scene
