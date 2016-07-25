#include "BoxObject.hpp"

BoxObject::BoxObject(mglu::GenericShader& shader)
        : BasicDrawableObject(
                  shader,
                  {{-0.5, -0.5, -0.5},
                   {-0.5, -0.5, 0.5},
                   {-0.5, 0.5, -0.5},
                   {-0.5, 0.5, 0.5},
                   {0.5, -0.5, -0.5},
                   {0.5, -0.5, 0.5},
                   {0.5, 0.5, -0.5},
                   {0.5, 0.5, 0.5}},
                  aligned::vector<glm::vec4>(8, glm::vec4(0.2, 0.2, 0, 1)),
                  {0, 1, 1, 3, 3, 2, 2, 0,

                   4, 5, 5, 7, 7, 6, 6, 4,

                   0, 4, 1, 5, 2, 6, 3, 7},
                  GL_LINES) {
}
