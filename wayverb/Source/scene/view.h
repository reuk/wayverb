#include "raytracer/cl/reflection.h"

#include "core/gpu_scene_data.h"

#include "utilities/aligned/vector.h"

#include "modern_gl_utils/drawable.h"
#include "modern_gl_utils/updatable.h"

#include "glm/glm.hpp"

namespace scene {

/// This is the actual opengl view. Use it inside a generic_renderer.
class view final : public mglu::drawable, public mglu::updatable {
public:
    //  Nodes.

    void set_node_positions(util::aligned::vector<glm::vec3> positions);
    void set_node_pressures(util::aligned::vector<float> pressures);
    void set_node_colours(util::aligned::vector<glm::vec3> colours);

    void set_nodes_visible(bool visible);

    //  Reflections.

    void set_reflections(util::aligned::vector<util::aligned::vector<
                                 wayverb::raytracer::reflection>> reflections,
                         const glm::vec3& source);
    void set_distance_travelled(double distance);

    void set_reflections_visible(bool visible);

    //  Scene/surfaces.

    void set_view(wayverb::core::gpu_scene_data view);

    void set_highlighted_surface(int surface);
    void set_emphasis_colour(const glm::vec3& colour);

    //  Sources/receivers.

    void set_sources(util::aligned::vector<glm::vec3> sources);
    void set_receivers(util::aligned::vector<glm::vec3> receivers);

    //  Drawing functionality.

    void set_view_projection_matrix(const glm::mat4& matrix);

    void update(float dt) override;

private:
    void do_draw(const glm::mat4& model_matrix) const override;
    glm::mat4 get_local_model_matrix() const override;

    glm::mat4 view_projection_matrix_;
    bool nodes_visible_ = false;
    bool reflections_visible_ = false;
};

}  // namespace scene
