#pragma once

#include "BasicDrawableObject.hpp"

class AxesObject final : public BasicDrawableObject {
public:
    template <typename T>
    AxesObject(T& shader)
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
                                  GL_LINES) {}
};
