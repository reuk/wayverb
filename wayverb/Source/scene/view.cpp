#include "view.h"

#include "../JuceLibraryCode/JuceHeader.h"  //  Only for GL stuff

namespace scene {

/*
void view::set_node_positions(util::aligned::vector<glm::vec3> positions) {}

void view::set_node_pressures(util::aligned::vector<float> pressures) {}

void view::set_node_colours(util::aligned::vector<glm::vec3> colours) {}

void view::set_nodes_visible(bool visible) {
    nodes_visible_ = visible;
}

void view::set_reflections(
        util::aligned::vector<util::aligned::vector<
                wayverb::raytracer::reflection>> reflections,
        const glm::vec3& source) {}

void view::set_distance_travelled(double distance) {}

void view::set_reflections_visible(bool visible) {
    reflections_visible_ = visible;
}

void view::set_view(wayverb::core::gpu_view_data view) {}

void view::set_highlighted_surface(int surface) {}

void view::set_emphasis_colour(const glm::vec3& colour) {}

void view::set_sources(util::aligned::vector<glm::vec3> sources) {}

void view::set_receivers(util::aligned::vector<glm::vec3> receivers) {}
*/

void view::set_view_projection_matrix(const glm::mat4& matrix) {
    view_projection_matrix_ = matrix;
}

void view::update(float) {}

void view::do_draw(const glm::mat4&) const {
    const auto c = 0.0;
    glClearColor(c, c, c, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //  TODO Set up shaders.

    //  TODO Draw objects.
}

glm::mat4 view::get_local_model_matrix() const { return glm::mat4{}; }

}  // namespace scene
