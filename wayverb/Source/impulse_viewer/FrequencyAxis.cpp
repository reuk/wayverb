#include "FrequencyAxis.hpp"

FrequencyAxisObject::FrequencyAxisObject(ShaderProgram& shader,
                                         TextShader& text_shader)
        : BasicDrawableObject(
                  shader,
                  {{0, 0, 0}, {1, 0, 0}, {0, 0, 0}, {0, 1, 0}},
                  {{1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}},
                  {0, 1, 2, 3},
                  GL_LINES)
        , text_handler(text_shader) {
}

void FrequencyAxisObject::set_label(const std::string& t) {
    label = t;
}

void FrequencyAxisObject::draw() const {
    BasicDrawableObject::draw();
    text_handler.draw(label, 20, glm::vec2{0, 0});
}