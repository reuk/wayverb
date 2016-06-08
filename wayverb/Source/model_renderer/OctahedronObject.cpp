#include "OctahedronObject.hpp"

OctahedronObject::OctahedronObject(GenericShader &shader,
                                   const glm::vec3 &position,
                                   const glm::vec4 &color)
        : BasicDrawableObject(shader,
                              {
                                  {0, 0, -1},
                                  {0, 0, 1},
                                  {0, -1, 0},
                                  {0, 1, 0},
                                  {-1, 0, 0},
                                  {1, 0, 0},
                              },
                              std::vector<glm::vec4>(6, color),
                              {
                                  0, 2, 4, 0, 2, 5, 0, 3, 4, 0, 3, 5,
                                  1, 2, 4, 1, 2, 5, 1, 3, 4, 1, 3, 5,
                              },
                              GL_TRIANGLES) {
    set_position(position);
    set_scale(0.4);
}