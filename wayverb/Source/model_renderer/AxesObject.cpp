#include "AxesObject.hpp"

AxesObject::AxesObject(mglu::GenericShader& shader)
        : BasicDrawableObject(shader,
                              {{0, 0, 0},
                               {1, 0, 0},
                               {0, 0, 0},
                               {0, 1, 0},
                               {0, 0, 0},
                               {0, 0, 1}},
                              {{1, 0, 0, 1},
                               {1, 0, 0, 1},
                               {0, 1, 0, 1},
                               {0, 1, 0, 1},
                               {0, 0, 1, 1},
                               {0, 0, 1, 1}},
                              {0, 1, 2, 3, 4, 5},
                              GL_LINES) {
}
