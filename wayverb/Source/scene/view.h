#pragma once

#include "combined/model/source.h"
#include "combined/model/receiver.h"

#include "raytracer/cl/reflection.h"

#include "core/gpu_scene_data.h"

#include "utilities/aligned/vector.h"

#include "modern_gl_utils/drawable.h"
#include "modern_gl_utils/updatable.h"

#include "glm/glm.hpp"

#include <experimental/optional>

namespace wayverb {
namespace combined {
namespace model {
class view_state;
}
}
}

namespace scene {

/// This is the actual opengl view. Use it inside a generic_renderer.
class view final : public mglu::drawable, public mglu::updatable {
public:
    //  Setup/teardown.

    view();
    ~view() noexcept;

    //  Nodes.

    void set_node_positions(util::aligned::vector<glm::vec3> positions);
    void set_node_pressures(util::aligned::vector<float> pressures);

    //  Reflections.

    void set_reflections(util::aligned::vector<util::aligned::vector<
                                 wayverb::raytracer::reflection>> reflections,
                         const glm::vec3& source);
    void set_distance_travelled(float distance);

    //  Reset. Will delete current mesh/raytracer state.
    void clear();

    //  Scene/surfaces.

    void set_scene(const wayverb::core::triangle* triangles,
                   size_t num_triangles,
                   const glm::vec3* vertices,
                   size_t num_vertices);

    void set_highlighted_surface(
            std::experimental::optional<size_t> highlighted);
    void set_emphasis_colour(const glm::vec3& colour);

    //  Sources/receivers.

    void set_sources(std::vector<wayverb::combined::model::source::raw> sources);
    void set_receivers(std::vector<wayverb::combined::model::receiver::raw> receivers);

    //  Drawing functionality.

    void set_view_state(const wayverb::combined::model::view_state& view_state);

    void set_projection_matrix(const glm::mat4& matrix);

    void update(float dt) override;

private:
    void do_draw(const glm::mat4& matrix) const override;
    glm::mat4 get_local_model_matrix() const override;

    class impl;
    std::unique_ptr<impl> pimpl_;
};

}  // namespace scene
